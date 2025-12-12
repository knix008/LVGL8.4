#include "training_manager.h"
#include "config.h"
#include "logger.h"
#include <filesystem>
#include <chrono>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

TrainingManager::TrainingManager()
    : recognizer(nullptr),
      database(nullptr),
      minimum_images_per_person(Config::MINIMUM_IMAGES_PER_PERSON),
      training_in_progress(false),
      current_status_message("Idle") {}

bool TrainingManager::initialize(FaceRecognizerBase* face_recognizer, FaceDatabase* face_database) {
    if (!face_recognizer || !face_database) {
        LOG_ERROR("Invalid recognizer or database pointer");
        return false;
    }

    recognizer = face_recognizer;
    database = face_database;
    return true;
}

void TrainingManager::update_status(const std::string& message) {
    current_status_message = message;
    LOG_INFO(message);
}

bool TrainingManager::load_training_images(const std::string& dataset_path,
                                          std::vector<cv::Mat>& images,
                                          std::vector<int>& labels) {
    if (!fs::exists(dataset_path)) {
        update_status("Dataset directory not found: " + dataset_path);
        return false;
    }

    images.clear();
    labels.clear();

    try {
        for (const auto& person_dir : fs::directory_iterator(dataset_path)) {
            if (!person_dir.is_directory()) {
                continue;
            }

            std::string dir_name = person_dir.path().filename().string();
            int person_id = std::stoi(dir_name);

            int image_count = 0;
            for (const auto& img_file : fs::directory_iterator(person_dir.path())) {
                if (!img_file.is_regular_file()) {
                    continue;
                }

                std::string ext = img_file.path().extension().string();
                if (ext != ".jpg" && ext != ".jpeg" && ext != ".png" && ext != ".bmp") {
                    continue;
                }

                cv::Mat image = cv::imread(img_file.path().string(), cv::IMREAD_GRAYSCALE);
                if (image.empty()) {
                    LOG_WARN("Failed to load image: " << img_file.path().string());
                    continue;
                }

                // Preprocess image
                cv::resize(image, image, cv::Size(Config::TRAINING_IMAGE_SIZE, Config::TRAINING_IMAGE_SIZE));
                cv::equalizeHist(image, image);

                images.push_back(image);
                labels.push_back(person_id);
                image_count++;
            }

            if (image_count > 0) {
                update_status("Loaded " + std::to_string(image_count) + " images for person " + dir_name);
            }
        }

        if (images.empty()) {
            update_status("No valid training images found in dataset");
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        update_status("Error loading training images: " + std::string(e.what()));
        return false;
    }
}

TrainingStats TrainingManager::validate_dataset(const std::string& dataset_path) {
    TrainingStats stats = {false, 0, 0, 0, "", 0};

    if (!fs::exists(dataset_path)) {
        stats.error_message = "Dataset directory not found";
        return stats;
    }

    std::map<int, int> person_image_count;

    try {
        for (const auto& person_dir : fs::directory_iterator(dataset_path)) {
            if (!person_dir.is_directory()) {
                continue;
            }

            std::string dir_name = person_dir.path().filename().string();
            int person_id = std::stoi(dir_name);

            int image_count = 0;
            for (const auto& img_file : fs::directory_iterator(person_dir.path())) {
                if (img_file.is_regular_file()) {
                    std::string ext = img_file.path().extension().string();
                    if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp") {
                        image_count++;
                    }
                }
            }

            if (image_count > 0) {
                person_image_count[person_id] = image_count;
                stats.total_images += image_count;
            }
        }

        stats.total_people = person_image_count.size();
        if (stats.total_people > 0) {
            stats.images_per_person_avg = stats.total_images / stats.total_people;
            stats.success = true;
        }

    } catch (const std::exception& e) {
        stats.error_message = std::string(e.what());
    }

    return stats;
}

TrainingStats TrainingManager::train_from_filesystem(const std::string& dataset_path) {
    auto start_time = std::chrono::high_resolution_clock::now();
    TrainingStats stats = {false, 0, 0, 0, "", 0};

    if (!recognizer || !database) {
        stats.error_message = "Recognizer or database not initialized";
        return stats;
    }

    training_in_progress = true;
    update_status("Loading training images from filesystem...");

    std::vector<cv::Mat> images;
    std::vector<int> labels;

    if (!load_training_images(dataset_path, images, labels)) {
        stats.error_message = current_status_message;
        training_in_progress = false;
        return stats;
    }

    update_status("Training recognizer with " + std::to_string(images.size()) + " images...");

    try {
        if (!recognizer->train_from_images(dataset_path)) {
            stats.error_message = "Training failed";
            training_in_progress = false;
            return stats;
        }

        stats.success = true;
        stats.total_people = recognizer->get_person_count();
        stats.total_images = images.size();
        if (stats.total_people > 0) {
            stats.images_per_person_avg = stats.total_images / stats.total_people;
        }

        update_status("Training complete! " + std::to_string(stats.total_people) +
                     " people trained with " + std::to_string(stats.total_images) + " images");

    } catch (const std::exception& e) {
        stats.error_message = std::string(e.what());
        update_status("Training error: " + stats.error_message);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    stats.duration_ms = duration.count();

    training_in_progress = false;
    return stats;
}

TrainingStats TrainingManager::train_from_database() {
    auto start_time = std::chrono::high_resolution_clock::now();
    TrainingStats stats = {false, 0, 0, 0, "", 0};

    if (!recognizer || !database) {
        stats.error_message = "Recognizer or database not initialized";
        return stats;
    }

    training_in_progress = true;
    update_status("Loading training data from database...");

    try {
        if (!recognizer->train_from_database()) {
            stats.error_message = "Failed to train from database";
            training_in_progress = false;
            return stats;
        }

        stats.success = true;
        stats.total_people = recognizer->get_person_count();
        update_status("Training complete! " + std::to_string(stats.total_people) + " people loaded from database");

    } catch (const std::exception& e) {
        stats.error_message = std::string(e.what());
        update_status("Training error: " + stats.error_message);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    stats.duration_ms = duration.count();

    training_in_progress = false;
    return stats;
}

TrainingStats TrainingManager::retrain_all() {
    auto start_time = std::chrono::high_resolution_clock::now();
    TrainingStats stats = {false, 0, 0, 0, "", 0};

    if (!recognizer || !database) {
        stats.error_message = "Recognizer or database not initialized";
        return stats;
    }

    training_in_progress = true;
    update_status("Retraining model with all data...");

    try {
        if (!recognizer->retrain_model()) {
            stats.error_message = "Retraining failed";
            training_in_progress = false;
            return stats;
        }

        stats.success = true;
        stats.total_people = recognizer->get_person_count();
        update_status("Retraining complete! " + std::to_string(stats.total_people) + " people in model");

    } catch (const std::exception& e) {
        stats.error_message = std::string(e.what());
        update_status("Retraining error: " + stats.error_message);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    stats.duration_ms = duration.count();

    training_in_progress = false;
    return stats;
}
