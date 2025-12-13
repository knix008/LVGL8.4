#include "face_detector.h"
#include "logger.h"
#include <fstream>

FaceDetector::FaceDetector() {}

bool FaceDetector::initialize() {
    // Try to load the default Haar Cascade classifier
    std::string cascade_path = cv::samples::findFile(
        "haarcascades/haarcascade_frontalface_default.xml"
    );

    // If not found, try common system paths
    if (cascade_path.empty()) {
        const char* common_paths[] = {
            "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml"
        };

        for (const auto& path : common_paths) {
            if (std::ifstream(path).good()) {
                cascade_path = path;
                break;
            }
        }
    }

    if (cascade_path.empty()) {
        LOG_ERROR("Could not find haarcascade_frontalface_default.xml");
        return false;
    }

    return load_cascade(cascade_path);
}

bool FaceDetector::load_cascade(const std::string& cascade_path) {
    if (!face_cascade.load(cascade_path)) {
        LOG_ERROR("Failed to load cascade classifier from: " << cascade_path);
        return false;
    }

    LOG_INFO("Face cascade loaded successfully from: " << cascade_path);
    return true;
}

std::vector<Face> FaceDetector::detect_faces(const cv::Mat& frame) {
    std::vector<Face> faces;

    if (frame.empty()) {
        LOG_WARN("Input frame is empty");
        return faces;
    }

    if (!is_loaded()) {
        LOG_ERROR("Face cascade not loaded");
        return faces;
    }

    try {
        // Increment total frames processed for metrics
        total_frames_processed++;

        // Convert to grayscale for detection
        cv::Mat gray;
        if (frame.channels() == 3) {
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = frame.clone();
        }

        // Enhance contrast
        cv::Mat enhanced;
        cv::equalizeHist(gray, enhanced);

        // Detect faces with confidence levels
        std::vector<cv::Rect> face_rects;
        std::vector<int> num_detections;
        face_cascade.detectMultiScale(
            enhanced,
            face_rects,
            num_detections,
            scale_factor,
            min_neighbors,
            0,
            min_face_size,
            max_face_size
        );

        // Track frames with detections
        if (!face_rects.empty()) {
            frames_with_detections++;
        }

        // Convert to Face objects
        for (size_t i = 0; i < face_rects.size(); ++i) {
            Face face;
            face.bbox = face_rects[i];
            face.id = -1;  // Unknown
            face.name = "Unknown";
            // Initialize confidence to 0.0 - will be set by recognition process
            // (Detection confidence is not meaningful for display - only recognition confidence matters)
            face.confidence = 0.0;

            faces.push_back(face);
        }

        return faces;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in detect_faces: " << e.what());
        return faces;
    }
}

std::vector<Face> FaceDetector::detect_faces_with_id(
    const cv::Mat& frame,
    const std::vector<int>& face_ids
) {
    std::vector<Face> faces = detect_faces(frame);

    // Assign IDs if provided
    for (size_t i = 0; i < faces.size() && i < face_ids.size(); ++i) {
        faces[i].id = face_ids[i];
    }

    return faces;
}

void FaceDetector::set_scale_factor(double scale) {
    if (scale > 1.0) {
        scale_factor = scale;
    }
}

void FaceDetector::set_min_neighbors(int neighbors) {
    if (neighbors > 0) {
        min_neighbors = neighbors;
    }
}

void FaceDetector::set_min_face_size(int width, int height) {
    if (width > 0 && height > 0) {
        min_face_size = cv::Size(width, height);
    }
}

void FaceDetector::set_max_face_size(int width, int height) {
    if (width > 0 && height > 0) {
        max_face_size = cv::Size(width, height);
    }
}

bool FaceDetector::is_loaded() const {
    return !face_cascade.empty();
}

void FaceDetector::reset_metrics() {
    total_frames_processed = 0;
    frames_with_detections = 0;
    total_false_positives = 0;
}

double FaceDetector::get_detection_rate() const {
    if (total_frames_processed == 0) {
        return 0.0;
    }
    return (static_cast<double>(frames_with_detections) / total_frames_processed) * 100.0;
}

double FaceDetector::get_false_positive_rate() const {
    if (total_frames_processed == 0) {
        return 0.0;
    }
    return (static_cast<double>(total_false_positives) / total_frames_processed) * 100.0;
}
