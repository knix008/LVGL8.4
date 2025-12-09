#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 * @brief Centralized configuration for the GTK Face Recognition Application
 *
 * This header contains all magic numbers, thresholds, and constants used throughout
 * the application. Update values here to tune application behavior without modifying
 * source code.
 */

namespace Config {

    // ========================
    // Face Detection Parameters
    // ========================

    /// Haar Cascade scale factor for multi-scale detection
    /// Range: 1.01-2.0 (lower = more thorough but slower, higher = faster but may miss faces)
    constexpr float FACE_DETECTION_SCALE = 1.1f;

    /// Minimum overlapping detections required to confirm face
    /// Range: 1-10 (higher = fewer false positives, lower = more detections)
    constexpr int FACE_DETECTION_MIN_NEIGHBORS = 8;

    /// Minimum face size in pixels (width and height)
    constexpr int FACE_MIN_SIZE = 30;

    /// Maximum face size in pixels (0 = unlimited)
    constexpr int FACE_MAX_SIZE = 0;

    // ========================
    // Face Recognition Parameters
    // ========================

    /// Confidence threshold for showing recognized name (50-100%)
    /// Below this threshold shows "Unknown" label with yellow box
    constexpr double RECOGNITION_CONFIDENCE_THRESHOLD = 0.70;

    /// Minimum face size required for reliable recognition
    constexpr int MINIMUM_FACE_SIZE_FOR_RECOGNITION = 80;

    /// Time interval between recognition updates (microseconds)
    /// Reduces CPU load by caching recognition results
    constexpr long RECOGNITION_UPDATE_INTERVAL_US = 1500000;  // 1.5 seconds

    /// Maximum recognition cache validity time (microseconds)
    constexpr long RECOGNITION_CACHE_MAX_AGE_US = 2000000;  // 2 seconds

    // ========================
    // UI Display Parameters
    // ========================

    /// Camera display width in pixels
    constexpr int DISPLAY_WIDTH = 640;

    /// Camera display height in pixels
    constexpr int DISPLAY_HEIGHT = 480;

    /// Bounding box scale factor (slightly expand box beyond detected face)
    constexpr double BOUNDING_BOX_SCALE = 1.2;

    /// Bounding box line thickness in pixels
    constexpr int BOUNDING_BOX_THICKNESS = 2;

    /// Text label font size (in pixels, for OpenCV putText)
    constexpr int TEXT_FONT_SCALE = 4;

    /// Confidence text font size
    constexpr int CONFIDENCE_TEXT_FONT_SCALE = 3;

    /// UI refresh rate in milliseconds (30ms ≈ 33 FPS)
    constexpr int UI_REFRESH_RATE_MS = 30;

    /// Window width in pixels
    constexpr int WINDOW_WIDTH = 800;

    /// Window height in pixels
    constexpr int WINDOW_HEIGHT = 600;

    // ========================
    // Color Definitions (BGR format for OpenCV)
    // ========================

    /// Green color for high-confidence face recognition
    constexpr int COLOR_GREEN_B = 0;
    constexpr int COLOR_GREEN_G = 255;
    constexpr int COLOR_GREEN_R = 0;

    /// Yellow color for low-confidence/unknown faces
    constexpr int COLOR_YELLOW_B = 0;
    constexpr int COLOR_YELLOW_G = 255;
    constexpr int COLOR_YELLOW_R = 255;

    /// White color for text
    constexpr int COLOR_WHITE_B = 255;
    constexpr int COLOR_WHITE_G = 255;
    constexpr int COLOR_WHITE_R = 255;

    // ========================
    // ArcFace Model Parameters
    // ========================

    /// ArcFace model input size (112x112 pixels)
    constexpr int ARCFACE_INPUT_SIZE = 112;

    /// ArcFace embedding dimension (512D after processing)
    constexpr int ARCFACE_EMBEDDING_DIMENSION = 512;

    /// Normalization mean for ArcFace preprocessing
    constexpr float ARCFACE_NORM_MEAN = 127.5f;

    /// Normalization scale for ArcFace preprocessing
    constexpr float ARCFACE_NORM_SCALE = 128.0f;

    /// Model file path relative to application directory
    extern const char* ARCFACE_MODEL_PATH;

    // ========================
    // Threading and Queue Parameters
    // ========================

    /// Maximum number of frames in camera queue
    /// Limits memory usage during slow processing
    constexpr size_t CAMERA_QUEUE_MAX_SIZE = 5;

    /// Maximum consecutive camera errors before stopping capture
    constexpr int CAMERA_ERROR_THRESHOLD = 10;

    // ========================
    // Training Parameters
    // ========================

    /// Image resize dimension for training (200x200)
    constexpr int TRAINING_IMAGE_SIZE = 200;

    /// Minimum images per person for reliable training
    constexpr int MINIMUM_IMAGES_PER_PERSON = 2;

    /// Recommended images per person for best results
    constexpr int RECOMMENDED_IMAGES_PER_PERSON = 5;

    // ========================
    // Database Parameters
    // ========================

    /// Database file name
    extern const char* DATABASE_FILE_NAME;

    /// Dataset directory for training images
    extern const char* DATASET_DIRECTORY;

    // ========================
    // Performance Thresholds
    // ========================

    /// Target FPS for display
    constexpr int TARGET_FPS = 30;

    /// Minimum FPS warning threshold
    constexpr int MIN_FPS_WARNING = 10;

    /// Detection rate tracking window (in frames)
    constexpr int DETECTION_TRACKING_WINDOW = 100;

} // namespace Config

#endif // CONFIG_H
