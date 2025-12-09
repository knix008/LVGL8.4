#ifndef FACE_RECOGNIZER_BASE_H
#define FACE_RECOGNIZER_BASE_H

#include <string>
#include <vector>
#include <opencv2/core.hpp>

/**
 * @file face_recognizer_base.h
 * @brief Abstract base class for face recognition algorithms
 *
 * Provides a common interface for different face recognition implementations
 * (LBPH, ArcFace, etc.), allowing for algorithm switching and abstraction.
 */

/**
 * @brief Abstract base class for face recognizer implementations
 *
 * Defines common interface for face recognition algorithms. Implementations
 * should provide training and recognition capabilities.
 *
 * @thread_safety Implementations may vary. Check concrete class documentation.
 */
class FaceRecognizerBase {
public:
    virtual ~FaceRecognizerBase() = default;

    /**
     * @brief Train recognizer from images in directory structure
     *
     * Loads images from dataset directory structure: dataset/PersonID/image.jpg
     *
     * @param dataset_path Path to dataset directory
     * @return true if training successful, false otherwise
     */
    virtual bool train_from_images(const std::string& dataset_path) = 0;

    /**
     * @brief Train recognizer from embeddings stored in database
     *
     * For incremental learning and persistence across sessions.
     *
     * @return true if training successful, false otherwise
     */
    virtual bool train_from_database() = 0;

    /**
     * @brief Recognize face in image
     *
     * Returns person ID and confidence score for the recognized face.
     *
     * @param face_image Face image to recognize (preprocessed)
     * @param[out] confidence Recognition confidence score (0.0-1.0)
     * @return Person ID (>0) if recognized, -1 for unknown face
     */
    virtual int recognize(const cv::Mat& face_image, double& confidence) = 0;

    /**
     * @brief Recognize face and return person name
     *
     * @param face_image Face image to recognize
     * @param[out] confidence Recognition confidence score (0.0-1.0)
     * @return Person name if recognized, "Unknown" otherwise
     */
    virtual std::string recognize_with_name(const cv::Mat& face_image, double& confidence) = 0;

    /**
     * @brief Add training data for a person
     *
     * Used for incremental learning of new faces.
     *
     * @param face_image Face image to train from
     * @param person_id Person ID to associate with image
     * @return true if added successfully, false otherwise
     */
    virtual bool add_training_data(const cv::Mat& face_image, int person_id) = 0;

    /**
     * @brief Register a person in the system
     *
     * Creates database entry for a new person.
     *
     * @param person_name Name of the person
     * @return Person ID if successful, -1 otherwise
     */
    virtual int register_person(const std::string& person_name) = 0;

    /**
     * @brief Get person name by ID
     *
     * @param person_id Person ID
     * @return Person name if found, "Unknown" otherwise
     */
    virtual std::string get_label_name(int person_id) const = 0;

    /**
     * @brief Check if recognizer is trained
     *
     * @return true if model has been trained with data, false otherwise
     */
    virtual bool is_trained() const = 0;

    /**
     * @brief Get number of people in trained model
     *
     * @return Number of unique people, 0 if not trained
     */
    virtual int get_person_count() const = 0;

    /**
     * @brief Retrain recognizer with new data
     *
     * Rebuilds the model from current training data.
     *
     * @return true if retraining successful, false otherwise
     */
    virtual bool retrain_model() = 0;

    /**
     * @brief Clear all training data
     *
     * Resets recognizer to untrained state.
     */
    virtual void clear_model() = 0;

};  // class FaceRecognizerBase

#endif // FACE_RECOGNIZER_BASE_H
