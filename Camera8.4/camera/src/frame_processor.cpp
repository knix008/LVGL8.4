#include "frame_processor.h"
#include "config.h"
#include "logger.h"
#include <chrono>

FrameProcessor::FrameProcessor()
    : recognizer(nullptr),
      last_recognition_time_us(0),
      recognition_update_interval_us(Config::RECOGNITION_UPDATE_INTERVAL_US),
      use_recognition_cache(true),
      frame_counter(0),
      recognition_frame_skip(Config::RECOGNITION_FRAME_SKIP),
      frame_scale(1.0),
      flip_horizontal(true),
      total_frames_processed(0),
      total_faces_detected(0),
      average_processing_time_ms(0.0) {}

bool FrameProcessor::initialize(std::unique_ptr<FaceDetector> face_detector,
                                FaceRecognizerBase* face_recognizer) {
    if (!face_detector || !face_recognizer) {
        LOG_ERROR("Invalid detector or recognizer");
        return false;
    }

    detector = std::move(face_detector);
    recognizer = face_recognizer;
    return true;
}

cv::Mat FrameProcessor::preprocess_frame(const cv::Mat& frame) {
    if (frame.empty()) {
        return frame;
    }

    cv::Mat processed = frame.clone();

    // Flip horizontally for mirrored effect
    if (flip_horizontal) {
        cv::flip(processed, processed, 1);
    }

    // Scale if needed
    if (frame_scale != 1.0) {
        int new_width = static_cast<int>(processed.cols * frame_scale);
        int new_height = static_cast<int>(processed.rows * frame_scale);
        cv::resize(processed, processed, cv::Size(new_width, new_height));
    }

    return processed;
}

bool FrameProcessor::should_recognize(long current_time_us) {
    if (!use_recognition_cache || recognition_update_interval_us == 0) {
        return true;
    }

    if (last_recognition_time_us == 0) {
        last_recognition_time_us = current_time_us;
        return true;
    }

    long elapsed_us = current_time_us - last_recognition_time_us;
    if (elapsed_us >= recognition_update_interval_us) {
        last_recognition_time_us = current_time_us;
        return true;
    }

    return false;
}

ProcessedFrame FrameProcessor::process_frame(const cv::Mat& frame, bool enable_recognition) {
    auto start_time = std::chrono::high_resolution_clock::now();

    ProcessedFrame result;
    result.is_valid = false;
    result.detection_count = 0;
    result.processing_time_ms = 0.0;
    result.recognition_ran = false;

    if (frame.empty()) {
        return result;
    }

    // Increment frame counter for ALL frames (not just when recognition is enabled)
    frame_counter++;

    // Preprocess frame
    result.frame = preprocess_frame(frame);

    // Detect faces
    if (!detector) {
        LOG_ERROR("Detector not initialized");
        return result;
    }

    try {
        result.faces = detector->detect_faces(result.frame);
        result.detection_count = result.faces.size();
        total_faces_detected += result.detection_count;

        // Initialize all detected faces as unknown by default
        // Only set to Unknown if not already recognized (face.id > 0)
        for (auto& face : result.faces) {
            if (face.id == 0 || face.id < 0) {  // If not recognized yet or explicitly unknown
                face.id = -1;
                face.name = "Unknown";
                // Don't reset confidence here - let it be set by recognition
            }
            // If face.id > 0, it's already recognized - preserve that status
        }

        // Recognize faces if enabled and recognizer is available
        if (enable_recognition && recognizer) {
            // Only perform recognition every Nth frame (frame skip optimization)
            bool should_run_recognition = (frame_counter % recognition_frame_skip == 0);
            
            auto current_time_us = std::chrono::high_resolution_clock::now()
                                      .time_since_epoch()
                                      .count() / 1000;  // Convert to microseconds

            if (should_run_recognition && should_recognize(current_time_us)) {
                // Check if recognizer is trained
                if (is_recognizer_ready()) {
                    result.recognition_ran = true;  // Mark that recognition ran this frame
                    for (auto& face : result.faces) {
                        double confidence = 0.0;

                        // Extract face ROI from bounding box for recognition
                        cv::Rect bbox = face.bbox;
                        if (!bbox.empty() && bbox.x >= 0 && bbox.y >= 0 &&
                            bbox.x + bbox.width <= result.frame.cols &&
                            bbox.y + bbox.height <= result.frame.rows) {
                            try {
                                cv::Mat face_roi = result.frame(bbox);
                                face.id = recognizer->recognize(face_roi, confidence);
                                face.confidence = confidence * 100.0;  // Convert to percentage

                                if (face.id > 0) {
                                    face.name = recognizer->get_label_name(face.id);
                                } else {
                                    face.name = "Unknown";
                                    face.id = -1;
                                }
                            } catch (const std::exception& e) {
                                face.id = -1;
                                face.name = "Unknown";
                            }
                        } else {
                            face.id = -1;
                            face.name = "Unknown";
                        }
                    }
                    // Cache the recognition results
                    cached_faces = result.faces;
                } else {
                    // Mark all faces as unknown if recognizer not ready
                    for (auto& face : result.faces) {
                        face.id = -1;
                        face.name = "Unknown";
                        face.confidence = 0.0;
                    }
                    cached_faces.clear();
                }
            } else if (use_recognition_cache && !cached_faces.empty()) {
                // Use cached recognition results between recognition intervals
                // Apply cached recognition data to detected faces
                for (size_t i = 0; i < result.faces.size() && i < cached_faces.size(); i++) {
                    result.faces[i].id = cached_faces[i].id;
                    result.faces[i].name = cached_faces[i].name;
                    result.faces[i].confidence = cached_faces[i].confidence;
                }
            }
        }

        result.is_valid = true;

        // Calculate processing time
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
        result.processing_time_ms = duration.count();

        // Update average processing time
        total_frames_processed++;
        average_processing_time_ms = (average_processing_time_ms * (total_frames_processed - 1) +
                                     result.processing_time_ms) / total_frames_processed;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in process_frame: " << e.what());
        result.is_valid = false;
    }

    return result;
}

bool FrameProcessor::is_recognizer_ready() const {
    if (!recognizer) {
        return false;
    }
    return recognizer->is_trained();
}

void FrameProcessor::reset_statistics() {
    total_frames_processed = 0;
    total_faces_detected = 0;
    average_processing_time_ms = 0.0;
}
