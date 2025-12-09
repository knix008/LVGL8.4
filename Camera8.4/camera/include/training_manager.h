#ifndef TRAINING_MANAGER_H
#define TRAINING_MANAGER_H

#include <string>
#include <memory>
#include "face_recognizer_base.h"
#include "face_database.h"

/**
 * @file training_manager.h
 * @brief Training coordination and management for face recognizer
 *
 * Handles training workflows including loading data from disk and database,
 * coordinating with the recognizer, and reporting training progress.
 */

/// Training status and statistics
struct TrainingStats {
    bool success;                   ///< Training completed successfully
    int total_people;              ///< Total unique people in training set
    int total_images;              ///< Total images processed
    int images_per_person_avg;     ///< Average images per person
    std::string error_message;     ///< Error message if training failed
    long duration_ms;              ///< Training duration in milliseconds
};

/**
 * @brief Training manager
 *
 * Orchestrates the training process including:
 * - Loading training data from filesystem and database
 * - Validating training data
 * - Coordinating with recognizer
 * - Reporting progress and statistics
 *
 * @thread_safety NOT thread-safe. Synchronize all calls from single thread.
 */
class TrainingManager {
private:
    FaceRecognizerBase* recognizer;  // Borrowed reference
    FaceDatabase* database;           // Borrowed reference

    // Training configuration
    int minimum_images_per_person;

    // Training progress tracking
    bool training_in_progress;
    std::string current_status_message;

public:
    /**
     * @brief Construct training manager
     */
    TrainingManager();
    ~TrainingManager() = default;

    /**
     * @brief Initialize manager with dependencies
     *
     * @param face_recognizer Pointer to recognizer (borrowed reference)
     * @param face_database Pointer to database (borrowed reference)
     * @return true if initialization successful
     */
    bool initialize(FaceRecognizerBase* face_recognizer, FaceDatabase* face_database);

    /**
     * @brief Train from filesystem images
     *
     * Loads training images from dataset directory structure:
     * dataset/PersonID/image1.jpg, dataset/PersonID/image2.jpg, etc.
     *
     * @param dataset_path Path to dataset directory
     * @return TrainingStats with training results
     */
    TrainingStats train_from_filesystem(const std::string& dataset_path);

    /**
     * @brief Train from database embeddings
     *
     * Retrains recognizer from embeddings stored in database.
     *
     * @return TrainingStats with training results
     */
    TrainingStats train_from_database();

    /**
     * @brief Retrain model with all available data
     *
     * Retrains recognizer using all images from both filesystem and database.
     *
     * @return TrainingStats with training results
     */
    TrainingStats retrain_all();

    /**
     * @brief Set minimum images required per person
     *
     * @param min_images Minimum number of images
     */
    void set_minimum_images_per_person(int min_images) {
        minimum_images_per_person = std::max(1, min_images);
    }

    /**
     * @brief Get current training status message
     *
     * @return Status message describing current training state
     */
    std::string get_status_message() const { return current_status_message; }

    /**
     * @brief Check if training is in progress
     *
     * @return true if currently training
     */
    bool is_training() const { return training_in_progress; }

    /**
     * @brief Validate training data before training
     *
     * @param dataset_path Path to dataset directory
     * @return TrainingStats with validation results
     */
    TrainingStats validate_dataset(const std::string& dataset_path);

private:
    /// Update status message
    void update_status(const std::string& message);

    /// Load and validate images from directory
    bool load_training_images(const std::string& dataset_path,
                             std::vector<cv::Mat>& images,
                             std::vector<int>& labels);

    /// Check if person has enough training images
    bool has_sufficient_training_data(int /*person_id*/, int image_count) const {
        return image_count >= minimum_images_per_person;
    }

};  // class TrainingManager

#endif // TRAINING_MANAGER_H
