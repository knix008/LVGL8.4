#include "camera.h"
#include "config.h"
#include <iostream>
#include <chrono>
#include <thread>

Camera::Camera() : cap(), is_running(false), is_active(false) {}

Camera::~Camera() {
    close();
}

bool Camera::open(int camera_id) {
    try {
        if (cap.isOpened()) {
            cap.release();
        }

        cap.open(camera_id, cv::CAP_V4L2);
        if (!cap.isOpened()) {
            std::cerr << "Error: Failed to open camera " << camera_id << std::endl;
            std::cerr << "Make sure:" << std::endl;
            std::cerr << "  1. Camera device /dev/video" << camera_id << " exists" << std::endl;
            std::cerr << "  2. You have permission to access it (try: sudo usermod -a -G video $USER)" << std::endl;
            std::cerr << "  3. No other application is using the camera" << std::endl;
            return false;
        }

        // Force camera to use lowest resolution
        // Many webcams report only their native resolution but can scale down
        // We'll force it to use a low resolution directly
        
        std::vector<std::pair<int, int>> preferred_resolutions = {
            {320, 240},   // QVGA - good balance
            {640, 480},   // VGA - fallback
            {160, 120},   // QQVGA - absolute minimum
            {176, 144},   // QCIF
        };
        
        std::cout << "\nSetting camera to lowest possible resolution..." << std::endl;
        
        // Try preferred resolutions in order
        int selected_width = 320;
        int selected_height = 240;
        bool resolution_set = false;
        
        for (const auto& res : preferred_resolutions) {
            cap.set(cv::CAP_PROP_FRAME_WIDTH, res.first);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, res.second);
            cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
            
            // Verify by capturing a frame
            cv::Mat test_frame;
            cap.grab();
            if (cap.read(test_frame) && !test_frame.empty()) {
                selected_width = test_frame.cols;
                selected_height = test_frame.rows;
                std::cout << "Set resolution to: " << selected_width << "x" << selected_height;
                if (selected_width != res.first || selected_height != res.second) {
                    std::cout << " (requested " << res.first << "x" << res.second 
                              << " - camera scaled to nearest supported)";
                }
                std::cout << std::endl;
                resolution_set = true;
                break;
            }
        }
        
        if (!resolution_set) {
            // Fallback to camera default
            selected_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
            selected_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
            std::cout << "Using camera default resolution: " << selected_width << "x" << selected_height << std::endl;
        }

        cap.set(cv::CAP_PROP_FPS, Config::CAMERA_FPS);
        cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

        std::cout << "Camera opened successfully: " << camera_id << std::endl;
        std::cout << "Final Resolution: " << get_frame_width() << "x" << get_frame_height() << std::endl;
        std::cout << "FPS: " << get_fps() << std::endl;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while opening camera: " << e.what() << std::endl;
        return false;
    }
}

void Camera::close() {
    stop();
    if (cap.isOpened()) {
        cap.release();
    }
}

void Camera::start() {
    if (is_running) return;

    is_running = true;
    is_active = true;
    capture_thread = std::thread(&Camera::capture_frames, this);
}

void Camera::stop() {
    is_running = false;
    is_active = false;

    if (capture_thread.joinable()) {
        capture_thread.join();
    }

    // Clear the queue
    std::lock_guard<std::mutex> lock(frame_mutex);
    while (!frame_queue.empty()) {
        frame_queue.pop();
    }
}

void Camera::capture_frames() {
    cv::Mat frame;
    int error_count = 0;
    const int max_errors = 10;

    while (is_running) {
        try {
            if (cap.read(frame)) {
                error_count = 0; // Reset error counter on success

                std::lock_guard<std::mutex> lock(frame_mutex);

                // Maintain max queue size
                if (frame_queue.size() >= max_queue_size) {
                    frame_queue.pop();
                }

                frame_queue.push(frame.clone());
            } else {
                error_count++;
                if (error_count == 1) {
                    std::cerr << "Warning: Failed to read frame from camera" << std::endl;
                }

                // Stop after too many consecutive errors
                if (error_count >= max_errors) {
                    std::cerr << "Error: Camera disconnected or no longer available (reached " << max_errors << " consecutive errors)" << std::endl;
                    is_running = false;
                    is_active = false;
                }

                // Small delay to avoid busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in capture thread: " << e.what() << std::endl;
            is_running = false;
            is_active = false;
        }
    }
}

bool Camera::get_frame(cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(frame_mutex);

    if (frame_queue.empty()) {
        return false;
    }

    frame = frame_queue.front();
    frame_queue.pop();
    return true;
}

bool Camera::has_frame() const {
    // This is a non-blocking check, so we don't lock
    return !frame_queue.empty();
}

bool Camera::is_camera_active() const {
    return is_active;
}

int Camera::get_frame_width() const {
    return static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
}

int Camera::get_frame_height() const {
    return static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
}

int Camera::get_fps() const {
    return static_cast<int>(cap.get(cv::CAP_PROP_FPS));
}
