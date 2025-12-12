#ifndef FACE_DETECTOR_H
#define FACE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <string>
#include <vector>

struct Face {
    cv::Rect bbox;              // Bounding box of the face
    int id;                     // Face ID (-1 if unknown)
    std::string name;           // Name of the person
    double confidence;          // Confidence level
};

class FaceDetector {
private:
    cv::CascadeClassifier face_cascade;
    double scale_factor = 1.1;
    int min_neighbors = 8;  // Increased from 4 to reduce false positives
    cv::Size min_face_size{30, 30};
    cv::Size max_face_size{};

    // Metrics tracking
    int total_frames_processed = 0;
    int frames_with_detections = 0;
    int total_false_positives = 0;  // Need manual annotation to track accurately

public:
    FaceDetector();
    ~FaceDetector() = default;

    bool initialize();
    bool load_cascade(const std::string& cascade_path);

    std::vector<Face> detect_faces(const cv::Mat& frame);
    std::vector<Face> detect_faces_with_id(const cv::Mat& frame, const std::vector<int>& face_ids);

    void set_scale_factor(double scale);
    void set_min_neighbors(int neighbors);
    void set_min_face_size(int width, int height);
    void set_max_face_size(int width, int height);

    bool is_loaded() const;

    // Metrics methods
    void reset_metrics();
    int get_total_frames() const { return total_frames_processed; }
    int get_frames_with_detections() const { return frames_with_detections; }
    double get_detection_rate() const;
    int get_total_false_positives() const { return total_false_positives; }
    void set_total_false_positives(int count) { total_false_positives = count; }
    double get_false_positive_rate() const;
};

#endif // FACE_DETECTOR_H
