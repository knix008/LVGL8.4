#ifndef DEEP_FACE_RECOGNIZER_H
#define DEEP_FACE_RECOGNIZER_H

#include "model_loader.h"
#include "faiss_index.h"
#include "face_database.h"
#include "face_detector.h"
#include "face_recognizer_base.h"
#include <opencv2/opencv.hpp>
#include <map>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief Deep learning-based face recognizer using ArcFace + FAISS
 *
 * Provides face recognition using ONNX Runtime for ArcFace embeddings
 * and FAISS for fast similarity search. Suitable for large-scale applications
 * supporting 20,000+ people.
 *
 * @thread_safety NOT thread-safe for recognize() calls. Database and index
 *                operations should be synchronized by caller.
 */
class DeepFaceRecognizer : public FaceRecognizerBase {
private:
    std::unique_ptr<ModelLoader> model_loader;
    std::unique_ptr<FAISSIndex> faiss_index;
    std::unique_ptr<FaceDetector> face_detector;  // For detecting faces in training images

    std::map<int, std::string> person_id_to_name;
    std::map<std::string, int> name_to_person_id;

    double confidence_threshold = 0.70;  // 70% threshold for reliable face recognition
    int min_face_size_for_recognition = 80;  // Minimum face size (width/height) for reliable recognition (>70% confidence)
    FaceDatabase* db = nullptr;
    bool model_trained = false;
    std::string model_path;

public:
    DeepFaceRecognizer();
    ~DeepFaceRecognizer() = default;

    // Model and database setup
    bool load_model(const std::string& onnx_model_path);
    void set_database(FaceDatabase* database);

    // Training methods - override base class
    bool train_from_images(const std::string& dataset_path) override;
    bool train_from_database() override;
    bool train_from_embeddings(const std::vector<int>& person_ids,
                               const std::vector<std::vector<float>>& embeddings);
    bool retrain_model() override;
    bool add_training_data(const cv::Mat& face_image, int person_id) override;

    // Recognition methods - override base class
    int recognize(const cv::Mat& face_image, double& confidence) override;
    std::string recognize_with_name(const cv::Mat& face_image, double& confidence) override;

    // Label management - override base class
    int register_person(const std::string& name) override;
    bool set_label_name(int person_id, const std::string& name);
    std::string get_label_name(int person_id) const override;
    int get_label_from_name(const std::string& name) const;
    void load_labels_from_database();

    // Configuration
    void set_confidence_threshold(double threshold);
    double get_confidence_threshold() const { return confidence_threshold; }
    void set_min_face_size_for_recognition(int size) { min_face_size_for_recognition = size; }
    int get_min_face_size_for_recognition() const { return min_face_size_for_recognition; }
    bool is_face_size_sufficient(int width, int height) const;

    // Base class implementations
    bool is_trained() const override { return model_trained; }
    int get_person_count() const override;
    void clear_model() override;

    // Backward compatibility
    bool is_model_trained() const { return model_trained; }
    bool is_model_loaded() const;
    int get_num_people() const { return get_person_count(); }

    // Embedding extraction and analysis
    std::vector<float> extract_embedding(const cv::Mat& face_image);
    double compare_embeddings(const std::vector<float>& emb1, const std::vector<float>& emb2);
    
    // Advanced recognition with top-k results
    std::vector<std::pair<std::string, double>> recognize_top_k(const cv::Mat& face_image, int k = 3);

    // Index management
    bool save_index(const std::string& filepath);
    bool load_index(const std::string& filepath);
    void clear();

private:
    // Helper methods
    cv::Mat preprocess_face(const cv::Mat& face_image);
    bool validate_face_image(const cv::Mat& image);
    std::vector<std::pair<int, std::vector<float>>>
        extract_embeddings_from_directory(const std::string& dataset_path);
};

#endif // DEEP_FACE_RECOGNIZER_H
