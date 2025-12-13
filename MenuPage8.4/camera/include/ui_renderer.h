#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include <opencv2/opencv.hpp>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <vector>
#include "face_detector.h"

/**
 * @file ui_renderer.h
 * @brief Rendering engine for converting frames to UI-displayable format
 *
 * Handles conversion of OpenCV Mat frames to GdkPixbuf for GTK display,
 * and drawing face detection/recognition results on frames.
 */

/**
 * @brief UI rendering engine
 *
 * Provides methods for:
 * - Converting OpenCV Mat to GdkPixbuf for GTK display
 * - Drawing face detections and recognition results
 * - Color management and text rendering
 *
 * @thread_safety NOT thread-safe. Call from GTK main thread only.
 */
class UIRenderer {
private:
    // Display dimensions
    int target_width;
    int target_height;

    // Colors (BGR format for OpenCV)
    struct Color {
        int B, G, R;
        Color(int b = 0, int g = 0, int r = 0) : B(b), G(g), R(r) {}
    };

    Color color_green;
    Color color_yellow;
    Color color_white;

    // Rendering parameters
    int box_thickness;
    int text_font_scale;
    int confidence_text_font_scale;

public:
    /**
     * @brief Construct renderer
     *
     * @param width Display width in pixels
     * @param height Display height in pixels
     */
    UIRenderer(int width = 640, int height = 480);
    ~UIRenderer() = default;

    /**
     * @brief Convert OpenCV Mat to GdkPixbuf
     *
     * Handles color space conversion (BGR to RGB) and resizing to target dimensions.
     *
     * @param mat Input OpenCV matrix (BGR format)
     * @return GdkPixbuf pointer (must be unreferenced by caller), nullptr on error
     */
    GdkPixbuf* mat_to_pixbuf(const cv::Mat& mat);

    /**
     * @brief Draw detected faces on frame
     *
     * Draws bounding boxes and labels for detected faces.
     * - Green box with name: High-confidence recognition (≥70%)
     * - Yellow box with "Unknown": Low-confidence detection (<70%)
     *
     * @param frame Frame to draw on (modified in-place)
     * @param faces Vector of detected faces
     */
    void draw_faces(cv::Mat& frame, const std::vector<Face>& faces);

    /**
     * @brief Set target display dimensions
     *
     * @param width Width in pixels
     * @param height Height in pixels
     */
    void set_dimensions(int width, int height);

    /**
     * @brief Set box rendering thickness
     *
     * @param thickness Line thickness in pixels
     */
    void set_box_thickness(int thickness) { box_thickness = thickness; }

    /**
     * @brief Set text font size
     *
     * @param scale Font scale factor
     */
    void set_text_font_scale(int scale) { text_font_scale = scale; }

private:
    /// Draw single face with all visual elements
    void draw_single_face(cv::Mat& frame, const Face& face);

    /// Get display color based on confidence
    Color get_face_color(double confidence_percent);

    /// Draw bounding box
    void draw_bounding_box(cv::Mat& frame, const Face& face, const Color& color);

    /// Draw text label
    void draw_label(cv::Mat& frame, const Face& face, const Color& color);

};  // class UIRenderer

#endif // UI_RENDERER_H
