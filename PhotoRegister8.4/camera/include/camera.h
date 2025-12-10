#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

class Camera {
private:
    cv::VideoCapture cap;
    std::thread capture_thread;
    std::mutex frame_mutex;
    std::queue<cv::Mat> frame_queue;
    std::atomic<bool> is_running{false};
    std::atomic<bool> is_active{false};
    size_t max_queue_size = 5;

    void capture_frames();

public:
    Camera();
    ~Camera();

    bool open(int camera_id = 0);
    void close();

    void start();
    void stop();

    bool get_frame(cv::Mat& frame);
    bool has_frame() const;

    bool is_camera_active() const;
    int get_frame_width() const;
    int get_frame_height() const;
    int get_fps() const;
};

#endif // CAMERA_H
