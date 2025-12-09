#include "model_loader.h"
#include <iostream>
#include <cmath>

ModelLoader::ModelLoader() {
    // Create ONNX Runtime environment
    try {
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ArcFace");
    } catch (const std::exception& e) {
        std::cerr << "Error creating ONNX Runtime environment: " << e.what() << std::endl;
    }
}

bool ModelLoader::load_model(const std::string& model_path) {
    if (!env) {
        std::cerr << "Error: ONNX Runtime environment not initialized" << std::endl;
        return false;
    }

    try {
        // Set session options
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(4);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // Create session from model file
        std::string model_path_str = model_path;
        session = std::make_unique<Ort::Session>(*env, model_path_str.c_str(), session_options);

        // Get input names and shapes
        Ort::AllocatorWithDefaultOptions allocator;
        size_t num_input_nodes = session->GetInputCount();

        if (num_input_nodes == 0) {
            std::cerr << "Error: Model has no input nodes" << std::endl;
            return false;
        }

        // Get first input name and shape
        auto input_name_allocated = session->GetInputNameAllocated(0, allocator);
        input_names.push_back(std::string(input_name_allocated.get()));
        input_names_cstr.push_back(input_names[0].c_str());

        std::vector<int64_t> input_dims = session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
        input_shape = input_dims;

        std::cout << "Model input name: " << input_names[0] << std::endl;
        std::cout << "Input shape: [";
        for (size_t i = 0; i < input_dims.size(); ++i) {
            std::cout << input_dims[i];
            if (i < input_dims.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;

        // Get output names and shapes
        size_t num_output_nodes = session->GetOutputCount();

        if (num_output_nodes == 0) {
            std::cerr << "Error: Model has no output nodes" << std::endl;
            return false;
        }

        auto output_name_allocated = session->GetOutputNameAllocated(0, allocator);
        output_names.push_back(std::string(output_name_allocated.get()));
        output_names_cstr.push_back(output_names[0].c_str());

        std::vector<int64_t> output_dims = session->GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
        output_shape = output_dims;

        std::cout << "Model output name: " << output_names[0] << std::endl;
        std::cout << "Output shape: [";
        for (size_t i = 0; i < output_dims.size(); ++i) {
            std::cout << output_dims[i];
            if (i < output_dims.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;

        is_loaded = true;
        std::cout << "Model loaded successfully from: " << model_path << std::endl;
        return true;

    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        return false;
    }
}

cv::Mat ModelLoader::normalize_image(const cv::Mat& image) {
    cv::Mat normalized;
    // ArcFace (InsightFace) normalization: (pixel - 127.5) / 128.0
    // This maps [0, 255] to approximately [-1, 1]
    image.convertTo(normalized, CV_32F);
    normalized = (normalized - 127.5) / 128.0;

    return normalized;
}

std::vector<float> ModelLoader::preprocess_image(const cv::Mat& image) {
    if (image.empty()) {
        std::cerr << "Error: Input image is empty" << std::endl;
        return std::vector<float>();
    }

    // ArcFace input: [1, 3, 112, 112] (batch_size=1, channels=3, height=112, width=112)
    // Model will dynamically read the actual dimensions
    int expected_width = get_input_width();
    int expected_height = get_input_height();
    int expected_channels = get_input_channels();

    cv::Mat img = image.clone();

    // Convert to RGB (ArcFace expects RGB, OpenCV uses BGR)
    if (img.channels() == 1) {
        cv::cvtColor(img, img, cv::COLOR_GRAY2RGB);
    } else if (img.channels() == 4) {
        cv::cvtColor(img, img, cv::COLOR_BGRA2RGB);
    } else if (img.channels() == 3) {
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
    }

    // Resize to model input size (112x112 for ArcFace)
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(expected_width, expected_height), 0, 0, cv::INTER_LINEAR);

    // Normalize using ArcFace normalization: (pixel - 127.5) / 128.0
    cv::Mat normalized = normalize_image(resized);

    // Convert to float array in CHW format (Channels, Height, Width)
    std::vector<float> input_data;
    input_data.reserve(expected_channels * expected_height * expected_width);

    // OpenCV Mat is HWC format, convert to CHW for ONNX
    for (int c = 0; c < expected_channels; ++c) {
        for (int h = 0; h < expected_height; ++h) {
            for (int w = 0; w < expected_width; ++w) {
                float value = normalized.at<cv::Vec3f>(h, w)[c];
                input_data.push_back(value);
            }
        }
    }

    return input_data;
}

std::vector<float> ModelLoader::inference(const cv::Mat& face_image) {
    std::vector<float> output;

    if (!is_loaded) {
        std::cerr << "Error: Model not loaded" << std::endl;
        return output;
    }

    try {
        // Preprocess image
        std::vector<float> input_data = preprocess_image(face_image);

        if (input_data.empty()) {
            std::cerr << "Error: Failed to preprocess image" << std::endl;
            return output;
        }

        // Create a fixed shape for inference (handle dynamic batch dimension)
        // Replace -1 (dynamic) with 1 for single image inference
        std::vector<int64_t> inference_shape = input_shape;
        for (auto& dim : inference_shape) {
            if (dim < 0) dim = 1;  // Replace dynamic dimensions with 1
        }

        // Prepare input tensor
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            input_data.data(),
            input_data.size(),
            inference_shape.data(),
            inference_shape.size()
        );

        // Run inference
        auto output_tensors = session->Run(
            Ort::RunOptions{nullptr},
            input_names_cstr.data(),
            &input_tensor,
            input_names_cstr.size(),
            output_names_cstr.data(),
            output_names_cstr.size()
        );

        // Extract output
        if (output_tensors.size() > 0 && output_tensors[0].IsTensor()) {
            float* output_data = output_tensors[0].GetTensorMutableData<float>();
            int64_t output_size = output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();

            // Model outputs [batch, channels, height, width] - flatten and use as embedding
            // For FaceNet-like models, this gives us a high-dimensional feature vector
            output.assign(output_data, output_data + output_size);

            // Normalize embedding to unit length (L2 normalization)
            float norm = 0.0f;
            for (float val : output) {
                norm += val * val;
            }
            norm = std::sqrt(norm);

            if (norm > 1e-6) {
                for (float& val : output) {
                    val /= norm;
                }
            }
        }

    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime inference error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error during inference: " << e.what() << std::endl;
    }

    return output;
}

int ModelLoader::get_embedding_dimension() const {
    if (output_shape.empty()) return 0;
    return static_cast<int>(output_shape.back());
}

int ModelLoader::get_flattened_output_size() const {
    if (output_shape.empty()) return 0;
    int64_t total_size = 1;
    for (int64_t dim : output_shape) {
        if (dim > 0) total_size *= dim;
    }
    return static_cast<int>(total_size);
}

int ModelLoader::get_input_width() const {
    if (input_shape.size() >= 4) return static_cast<int>(input_shape[3]);
    return 112;  // Default ArcFace input width
}

int ModelLoader::get_input_height() const {
    if (input_shape.size() >= 4) return static_cast<int>(input_shape[2]);
    return 112;  // Default ArcFace input height
}

int ModelLoader::get_input_channels() const {
    if (input_shape.size() >= 4) return static_cast<int>(input_shape[1]);
    return 3;  // Default RGB channels
}
