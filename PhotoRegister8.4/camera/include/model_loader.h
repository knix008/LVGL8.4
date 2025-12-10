#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>

class ModelLoader {
private:
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    std::vector<std::string> input_names;
    std::vector<std::string> output_names;
    std::vector<const char*> input_names_cstr;
    std::vector<const char*> output_names_cstr;
    std::vector<int64_t> input_shape;
    std::vector<int64_t> output_shape;
    bool is_loaded = false;

    // Helper methods
    std::vector<float> preprocess_image(const cv::Mat& image);
    cv::Mat normalize_image(const cv::Mat& image);

public:
    ModelLoader();
    ~ModelLoader() = default;

    // Load ONNX model
    bool load_model(const std::string& model_path);

    // Check if model is loaded
    bool is_model_loaded() const { return is_loaded; }

    // Run inference on a face image
    // Input: BGR image of detected face
    // Output: 128-dimensional embedding vector
    std::vector<float> inference(const cv::Mat& face_image);

    // Get model input/output information
    int get_embedding_dimension() const;
    int get_flattened_output_size() const;  // Total size of flattened output
    int get_input_width() const;
    int get_input_height() const;
    int get_input_channels() const;
};

#endif // MODEL_LOADER_H
