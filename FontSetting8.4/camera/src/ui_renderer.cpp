#include "ui_renderer.h"
#include "config.h"
#include "logger.h"
#include <cmath>

UIRenderer::UIRenderer(int width, int height)
    : target_width(width),
      target_height(height),
      color_green(Config::COLOR_GREEN_B, Config::COLOR_GREEN_G, Config::COLOR_GREEN_R),
      color_yellow(Config::COLOR_YELLOW_B, Config::COLOR_YELLOW_G, Config::COLOR_YELLOW_R),
      color_white(Config::COLOR_WHITE_B, Config::COLOR_WHITE_G, Config::COLOR_WHITE_R),
      box_thickness(Config::BOUNDING_BOX_THICKNESS),
      text_font_scale(Config::TEXT_FONT_SCALE),
      confidence_text_font_scale(Config::CONFIDENCE_TEXT_FONT_SCALE) {}

void UIRenderer::set_dimensions(int width, int height) {
    target_width = width;
    target_height = height;
}

GdkPixbuf* UIRenderer::mat_to_pixbuf(const cv::Mat& mat) {
    if (mat.empty()) {
        LOG_WARN("Empty input matrix");
        return nullptr;
    }

    try {
        cv::Mat display_mat = mat.clone();

        // Resize to target dimensions if needed
        if (display_mat.cols != target_width || display_mat.rows != target_height) {
            cv::resize(display_mat, display_mat, cv::Size(target_width, target_height));
        }

        // Convert BGR to RGB (OpenCV uses BGR by default)
        cv::Mat rgb_mat;
        cv::cvtColor(display_mat, rgb_mat, cv::COLOR_BGR2RGB);

        // Ensure continuous memory layout
        if (!rgb_mat.isContinuous()) {
            rgb_mat = rgb_mat.clone();
        }

        // Create GdkPixbuf from RGB data
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(
            rgb_mat.data,
            GDK_COLORSPACE_RGB,
            FALSE,                              // no alpha channel
            8,                                  // bits per sample
            rgb_mat.cols,                       // width
            rgb_mat.rows,                       // height
            rgb_mat.step,                       // rowstride
            nullptr,                            // destroy_fn (none needed)
            nullptr                             // destroy_fn_data
        );

        if (!pixbuf) {
            LOG_ERROR("Failed to create GdkPixbuf");
            return nullptr;
        }

        // Copy data to ensure persistence (GdkPixbuf doesn't own the data)
        GdkPixbuf* owned_pixbuf = gdk_pixbuf_copy(pixbuf);
        g_object_unref(pixbuf);

        return owned_pixbuf;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in mat_to_pixbuf: " << e.what());
        return nullptr;
    }
}

UIRenderer::Color UIRenderer::get_face_color(double confidence_percent) {
    if (confidence_percent >= 70.0) {
        return color_green;  // High confidence
    } else {
        return color_yellow;  // Low confidence
    }
}

void UIRenderer::draw_bounding_box(cv::Mat& frame, const Face& face, const Color& color) {
    if (face.bbox.empty()) {
        return;
    }

    // Scale the bounding box
    int x1 = std::max(0, face.bbox.x);
    int y1 = std::max(0, face.bbox.y);
    int x2 = std::min(frame.cols - 1, face.bbox.x + face.bbox.width);
    int y2 = std::min(frame.rows - 1, face.bbox.y + face.bbox.height);

    cv::Scalar cv_color(color.B, color.G, color.R);
    cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), cv_color, box_thickness);
}

void UIRenderer::draw_label(cv::Mat& frame, const Face& face, const Color& color) {
    if (face.bbox.empty()) {
        return;
    }

    // Only draw label if the face is recognized (id > 0)
    if (face.id <= 0) {
        // Unknown or unrecognized face - no label
        return;
    }

    // Prepare label text with ID and confidence
    char label_text[100];
    snprintf(label_text, sizeof(label_text), "%s (%.0f%%)",
            face.name.c_str(), face.confidence);
    std::string label = label_text;

    // Get text size
    int baseline = 0;
    cv::Size text_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX,
                                         text_font_scale / 10.0, 1, &baseline);

    // Position label above the bounding box
    int text_x = face.bbox.x;
    int text_y = std::max(20, face.bbox.y - 5);

    // Draw background rectangle for text
    cv::Scalar bg_color(color.B, color.G, color.R);
    cv::rectangle(frame,
                 cv::Point(text_x, text_y - text_size.height - 5),
                 cv::Point(text_x + text_size.width + 5, text_y + 5),
                 bg_color, -1);  // Filled rectangle

    // Draw text
    cv::Scalar text_color(color_white.B, color_white.G, color_white.R);
    cv::putText(frame, label,
               cv::Point(text_x + 3, text_y - 3),
               cv::FONT_HERSHEY_SIMPLEX,
               text_font_scale / 10.0,
               text_color, 1, cv::LINE_AA);
}

void UIRenderer::draw_single_face(cv::Mat& frame, const Face& face) {
    if (face.bbox.empty()) {
        return;
    }

    // Always use green color for bounding boxes (no red for unknown faces)
    Color face_color = color_green;

    // Draw bounding box (always shown)
    draw_bounding_box(frame, face, face_color);

    // Draw label only if face is recognized (id > 0)
    if (face.id > 0) {
        draw_label(frame, face, face_color);
    }
}

void UIRenderer::draw_faces(cv::Mat& frame, const std::vector<Face>& faces) {
    if (frame.empty()) {
        LOG_WARN("Empty frame for drawing");
        return;
    }

    try {
        for (const auto& face : faces) {
            draw_single_face(frame, face);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in draw_faces: " << e.what());
    }
}
