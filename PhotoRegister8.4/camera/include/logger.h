#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <sstream>
#include <memory>

/**
 * @file logger.h
 * @brief Structured logging framework with log levels
 *
 * Provides a centralized logging interface with support for different
 * log levels (DEBUG, INFO, WARN, ERROR).
 */

/**
 * @brief Log level enumeration
 */
enum class LogLevel {
    DEBUG = 0,   ///< Debug messages (most verbose)
    INFO  = 1,   ///< Informational messages
    WARN  = 2,   ///< Warning messages
    ERROR = 3    ///< Error messages (most severe)
};

/**
 * @brief Logger singleton class
 *
 * Provides centralized logging with configurable levels.
 * All methods are thread-safe.
 *
 * Usage:
 * ```cpp
 * Logger::get().info("Starting application");
 * Logger::get().set_level(LogLevel::DEBUG);
 * Logger::get().debug("Debug info: " << value);
 * Logger::get().error("Error occurred: " << error_msg);
 * ```
 */
class Logger {
private:
    static std::unique_ptr<Logger> instance;
    LogLevel current_level;
    bool include_timestamp;
    bool include_level_name;

    Logger() : current_level(LogLevel::INFO),
               include_timestamp(true),
               include_level_name(true) {}

    /**
     * @brief Format log message with metadata
     */
    std::string format_message(LogLevel level, const std::string& message);

    /**
     * @brief Get log level name
     */
    static const char* get_level_name(LogLevel level);

public:
    /**
     * @brief Get logger instance (singleton)
     *
     * @return Reference to logger instance
     */
    static Logger& get();

    /**
     * @brief Set minimum log level
     *
     * Messages below this level will not be logged.
     *
     * @param level Minimum log level
     */
    void set_level(LogLevel level);

    /**
     * @brief Get current log level
     *
     * @return Current minimum log level
     */
    LogLevel get_level() const { return current_level; }

    /**
     * @brief Enable/disable timestamp in log messages
     *
     * @param enable true to include timestamps
     */
    void set_timestamp_enabled(bool enable) { include_timestamp = enable; }

    /**
     * @brief Enable/disable level name in log messages
     *
     * @param enable true to include level names
     */
    void set_level_name_enabled(bool enable) { include_level_name = enable; }

    /**
     * @brief Log debug message
     *
     * Only logged if level is DEBUG or more verbose.
     *
     * @param message Debug message
     */
    void debug(const std::string& message);

    /**
     * @brief Log info message
     *
     * Only logged if level is INFO or more verbose.
     *
     * @param message Info message
     */
    void info(const std::string& message);

    /**
     * @brief Log warning message
     *
     * Only logged if level is WARN or more verbose.
     *
     * @param message Warning message
     */
    void warn(const std::string& message);

    /**
     * @brief Log error message
     *
     * Only logged if level is ERROR or more verbose (always logged by default).
     *
     * @param message Error message
     */
    void error(const std::string& message);

private:
    /**
     * @brief Internal logging function
     */
    void log(LogLevel level, const std::string& message);
};

/**
 * @brief Convenience macros for logging
 *
 * Usage:
 * ```cpp
 * LOG_DEBUG("Debug message");
 * LOG_INFO("Info message");
 * LOG_WARN("Warning message");
 * LOG_ERROR("Error message");
 * ```
 */
#define LOG_DEBUG(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::get().debug(oss.str()); \
    } while (0)

#define LOG_INFO(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::get().info(oss.str()); \
    } while (0)

#define LOG_WARN(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::get().warn(oss.str()); \
    } while (0)

#define LOG_ERROR(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::get().error(oss.str()); \
    } while (0)

#endif // LOGGER_H
