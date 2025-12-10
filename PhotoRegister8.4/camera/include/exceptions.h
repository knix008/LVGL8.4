#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

/**
 * @file exceptions.h
 * @brief Custom exception hierarchy for the application
 *
 * Defines application-specific exceptions for better error handling
 * and distinguishing different types of failures.
 */

/**
 * @brief Base exception class for the application
 *
 * All custom exceptions inherit from this class.
 */
class ApplicationException : public std::exception {
protected:
    std::string message;

public:
    /**
     * @brief Construct exception with message
     *
     * @param msg Error message
     */
    explicit ApplicationException(const std::string& msg) : message(msg) {}

    /**
     * @brief Get exception message
     *
     * @return C-string error message
     */
    const char* what() const noexcept override {
        return message.c_str();
    }
};

/**
 * @brief Exception for database-related errors
     *
 * Thrown when database operations fail.
 */
class DatabaseException : public ApplicationException {
public:
    explicit DatabaseException(const std::string& msg)
        : ApplicationException("DatabaseException: " + msg) {}
};

/**
 * @brief Exception for model loading errors
 *
 * Thrown when ONNX model fails to load or inference fails.
 */
class ModelException : public ApplicationException {
public:
    explicit ModelException(const std::string& msg)
        : ApplicationException("ModelException: " + msg) {}
};

/**
 * @brief Exception for face detection errors
 *
 * Thrown when face detection fails or produces invalid results.
 */
class DetectionException : public ApplicationException {
public:
    explicit DetectionException(const std::string& msg)
        : ApplicationException("DetectionException: " + msg) {}
};

/**
 * @brief Exception for face recognition errors
 *
 * Thrown when face recognition fails or model is not trained.
 */
class RecognitionException : public ApplicationException {
public:
    explicit RecognitionException(const std::string& msg)
        : ApplicationException("RecognitionException: " + msg) {}
};

/**
 * @brief Exception for camera-related errors
 *
 * Thrown when camera fails to open or capture frames.
 */
class CameraException : public ApplicationException {
public:
    explicit CameraException(const std::string& msg)
        : ApplicationException("CameraException: " + msg) {}
};

/**
 * @brief Exception for invalid input data
 *
 * Thrown when input validation fails.
 */
class InvalidDataException : public ApplicationException {
public:
    explicit InvalidDataException(const std::string& msg)
        : ApplicationException("InvalidDataException: " + msg) {}
};

/**
 * @brief Exception for file I/O errors
 *
 * Thrown when file operations fail.
 */
class FileException : public ApplicationException {
public:
    explicit FileException(const std::string& msg)
        : ApplicationException("FileException: " + msg) {}
};

/**
 * @brief Exception for configuration errors
 *
 * Thrown when configuration is invalid or missing.
 */
class ConfigurationException : public ApplicationException {
public:
    explicit ConfigurationException(const std::string& msg)
        : ApplicationException("ConfigurationException: " + msg) {}
};

#endif // EXCEPTIONS_H
