#ifndef FRAME_PROCESSOR_H
#define FRAME_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include "face_detector.h"
#include "face_recognizer_base.h"

/**
 * @file frame_processor.h
 * @brief Frame processing pipeline for face detection and recognition
 *
 * Handles the processing of video frames including face detection,
 * recognition, and metrics calculation.
 */

/// Result structure for processed frame
struct ProcessedFrame {
    cv::Mat frame;                      ///< Processed frame with detections
    std::vector<Face> faces;            ///< Detected faces in frame
    bool is_valid;                      ///< Frame is valid and processed
    int detection_count;                ///< Number of faces detected in this frame
    double processing_time_ms;          ///< Time taken to process this frame
    bool recognition_ran;               ///< True if recognition was actually performed this frame
};

/**
 * @brief Pipeline for frame processing
 *
 * Orchestrates face detection and recognition on video frames.
 * Provides caching and performance optimization.
 *
 * @thread_safety NOT thread-safe. Synchronize all method calls from single thread.
 */
class FrameProcessor {
private:
    std::unique_ptr<FaceDetector> detector;
    FaceRecognizerBase* recognizer;  // Borrowed reference

    // Caching and performance
    long last_recognition_time_us;
    long recognition_update_interval_us;
    bool use_recognition_cache;
    int frame_counter;              // Counter for frame skipping
    int recognition_frame_skip;     // Process recognition every Nth frame

    // Cache for recognition results between recognition intervals
    std::vector<Face> cached_faces;  // Store last recognized faces

    // Preprocessing parameters
    double frame_scale;
    bool flip_horizontal;

    // Statistics
    int total_frames_processed;
    int total_faces_detected;
    double average_processing_time_ms;

public:
    /**
     * @brief Construct frame processor
     */
    FrameProcessor();
    ~FrameProcessor() = default;

    /**
     * @brief Initialize processor with detector and recognizer
     *
     * @param face_detector Pointer to face detector (takes ownership)
     * @param face_recognizer Pointer to recognizer (borrowed reference)
     * @return true if initialization successful
     */
    bool initialize(std::unique_ptr<FaceDetector> face_detector,
                   FaceRecognizerBase* face_recognizer);

    /**
     * @brief Process a video frame
     *
     * Performs face detection and optional recognition on input frame.
     *
     * @param frame Input video frame
     * @param enable_recognition Enable face recognition (slower)
     * @return ProcessedFrame with detection results
     */
    ProcessedFrame process_frame(const cv::Mat& frame, bool enable_recognition = true);

    /**
     * @brief Preprocess frame (resize, flip, etc.)
     *
     * @param frame Input frame
     * @return Preprocessed frame
     */
    cv::Mat preprocess_frame(const cv::Mat& frame);

    /**
     * @brief Set frame scale factor
     *
     * @param scale Scale factor (0.5 = half resolution, 1.0 = full, 2.0 = double)
     */
    void set_frame_scale(double scale) { frame_scale = scale; }

    /**
     * @brief Enable/disable horizontal flip
     *
     * @param enable true to flip frames horizontally
     */
    void set_horizontal_flip(bool enable) { flip_horizontal = enable; }

    /**
     * @brief Set recognition update interval
     *
     * Reduces computation by caching recognition results.
     *
     * @param interval_us Interval in microseconds (0 = every frame)
     */
    void set_recognition_interval(long interval_us) { recognition_update_interval_us = interval_us; }

    /**
     * @brief Set recognition frame skip interval
     *
     * Process recognition only every Nth frame to reduce CPU load.
     *
     * @param skip_frames Number of frames to skip (1 = process every frame, 5-10 recommended)
     */
    void set_recognition_frame_skip(int skip_frames) { 
        recognition_frame_skip = (skip_frames > 0) ? skip_frames : 1; 
    }

    /**
     * @brief Get processing statistics
     *
     * @return Average processing time per frame in milliseconds
     */
    double get_average_processing_time() const { return average_processing_time_ms; }

    /**
     * @brief Get total frames processed
     *
     * @return Number of frames processed
     */
    int get_total_frames() const { return total_frames_processed; }

    /**
     * @brief Get total faces detected
     *
     * @return Number of faces detected across all frames
     */
    int get_total_faces_detected() const { return total_faces_detected; }

    /**
     * @brief Reset statistics
     */
    void reset_statistics();

    /**
     * @brief Check if recognizer is ready
     *
     * @return true if recognizer is initialized and trained
     */
    bool is_recognizer_ready() const;

private:
    /// Check if recognition should run (based on timing)
    bool should_recognize(long current_time_us);

};  // class FrameProcessor

#endif // FRAME_PROCESSOR_H
