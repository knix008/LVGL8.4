#include "logger.h"
#include <ctime>
#include <iomanip>

std::unique_ptr<Logger> Logger::instance = nullptr;

Logger& Logger::get() {
    if (!instance) {
        instance = std::unique_ptr<Logger>(new Logger());
    }
    return *instance;
}

void Logger::set_level(LogLevel level) {
    current_level = level;
}

const char* Logger::get_level_name(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string Logger::format_message(LogLevel level, const std::string& message) {
    std::ostringstream oss;

    // Add timestamp if enabled
    if (include_timestamp) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " ";
    }

    // Add level name if enabled
    if (include_level_name) {
        oss << "[" << get_level_name(level) << "] ";
    }

    // Add message
    oss << message;

    return oss.str();
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level) {
        return;  // Don't log messages below current level
    }

    std::string formatted = format_message(level, message);

    // Output to appropriate stream
    if (level >= LogLevel::WARN) {
        std::cerr << formatted << std::endl;
    } else {
        std::cout << formatted << std::endl;
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}
