#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/*
 * ============================================================================
 * CROWDLIGHT TRANSMITTER - LOGGING SYSTEM
 * ============================================================================
 * 
 * A comprehensive, production-ready logging system for ESP32-S3 embedded systems
 * with compile-time and runtime filtering, module tagging, timestamps, and 
 * thread-safe operation for use with FreeRTOS.
 * 
 * FEATURES:
 * - 5 log levels: NONE, ERROR, WARN, INFO, DEBUG, VERBOSE
 * - Compile-time filtering (reduces flash/RAM in production)
 * - Runtime level control (adjust verbosity without recompiling)
 * - Automatic module/file tagging
 * - Timestamp support using millis()
 * - Color-coded output (ANSI terminal colors)
 * - Printf-style formatting
 * - Thread-safe FreeRTOS operation
 * - Log statistics tracking
 * - Circular buffer for critical error logs
 * 
 * USAGE:
 * 
 * 1. Set compile-time log level in Config.h:
 *    #define LOG_LEVEL LOG_LEVEL_DEBUG
 * 
 * 2. In your code:
 *    LOG_ERROR("Critical failure: %s", errorMsg);
 *    LOG_INFO_TAG("MYMODULE", "Initialized successfully");
 * 
 * 3. Change runtime level:
 *    Logger::setLevel(LOG_LEVEL_VERBOSE);
 * 
 * 4. View statistics:
 *    Logger::printStats();
 * 
 * PERFORMANCE:
 * - Compile-time disabled logs compile to nothing (zero overhead)
 * - Runtime filtering is fast (single integer comparison)
 * - Uses stack buffers to minimize heap allocations
 * - Thread-safe with minimal locking overhead
 * - F() macro used for all constant strings to save RAM
 * 
 * ============================================================================
 */

// ============================================================================
// LOG LEVEL DEFINITIONS
// ============================================================================

enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_VERBOSE = 5
};

// ============================================================================
// ANSI COLOR CODES (for terminal output)
// ============================================================================

#ifndef LOG_ENABLE_COLORS
#define LOG_ENABLE_COLORS true
#endif

#if LOG_ENABLE_COLORS
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#else
#define ANSI_COLOR_RED     ""
#define ANSI_COLOR_YELLOW  ""
#define ANSI_COLOR_GREEN   ""
#define ANSI_COLOR_CYAN    ""
#define ANSI_COLOR_MAGENTA ""
#define ANSI_COLOR_RESET   ""
#endif

// ============================================================================
// CONFIGURATION DEFAULTS (can be overridden in Config.h)
// ============================================================================

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 256
#endif

#ifndef LOG_ENABLE_TIMESTAMPS
#define LOG_ENABLE_TIMESTAMPS true
#endif

#ifndef LOG_MAX_TAG_LENGTH
#define LOG_MAX_TAG_LENGTH 8
#endif

#ifndef LOG_ERROR_BUFFER_SIZE
#define LOG_ERROR_BUFFER_SIZE 50
#endif

// ============================================================================
// LOGGER CLASS
// ============================================================================

class Logger {
public:
    // Initialize the logger
    static void begin();
    
    // Core logging function (usually called via macros, not directly)
    static void log(LogLevel level, const char* tag, const char* format, ...);
    static void logWithLocation(LogLevel level, const char* tag, const char* file, 
                               int line, const char* format, ...);
    
    // Runtime level control
    static void setLevel(LogLevel level);
    static LogLevel getLevel();
    
    // Module-specific level control
    static void setModuleLevel(const char* module, LogLevel level);
    static LogLevel getModuleLevel(const char* module);
    
    // Statistics
    static void printStats();
    static void resetStats();
    static uint32_t getErrorCount();
    static uint32_t getWarnCount();
    static unsigned long getLastErrorTime();
    
    // Critical error buffer
    static void dumpRecentErrors();
    
    // Testing
    static void runTests();

private:
    struct LogStats {
        uint32_t errorCount;
        uint32_t warnCount;
        uint32_t infoCount;
        uint32_t debugCount;
        uint32_t verboseCount;
        unsigned long lastErrorTime;
        unsigned long lastWarnTime;
    };
    
    struct ErrorLogEntry {
        unsigned long timestamp;
        char tag[LOG_MAX_TAG_LENGTH + 1];
        char message[64];
        bool valid;
    };
    
    static LogLevel _runtimeLevel;
    static LogStats _stats;
    static SemaphoreHandle_t _mutex;
    static ErrorLogEntry _errorBuffer[LOG_ERROR_BUFFER_SIZE];
    static uint8_t _errorBufferIndex;
    
    static const char* getLevelString(LogLevel level);
    static const char* getLevelColor(LogLevel level);
    static void addToErrorBuffer(const char* tag, const char* message);
    static void updateStats(LogLevel level);
};

// ============================================================================
// LOGGING MACROS
// ============================================================================

// These macros provide the main API for logging. They expand to nothing
// when the compile-time log level is set below the message level.

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO  // Default to INFO if not defined
#endif

// Basic logging macros (without explicit tag)
#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(fmt, ...) Logger::log(LOG_LEVEL_ERROR, "SYSTEM", fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN(fmt, ...) Logger::log(LOG_LEVEL_WARN, "SYSTEM", fmt, ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...) Logger::log(LOG_LEVEL_INFO, "SYSTEM", fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) Logger::log(LOG_LEVEL_DEBUG, "SYSTEM", fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE(fmt, ...) Logger::log(LOG_LEVEL_VERBOSE, "SYSTEM", fmt, ##__VA_ARGS__)
#else
#define LOG_VERBOSE(fmt, ...) ((void)0)
#endif

// Tagged logging macros (with module tag)
#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR_TAG(tag, fmt, ...) Logger::log(LOG_LEVEL_ERROR, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR_TAG(tag, fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN_TAG(tag, fmt, ...) Logger::log(LOG_LEVEL_WARN, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_WARN_TAG(tag, fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO_TAG(tag, fmt, ...) Logger::log(LOG_LEVEL_INFO, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_INFO_TAG(tag, fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG_TAG(tag, fmt, ...) Logger::log(LOG_LEVEL_DEBUG, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG_TAG(tag, fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE_TAG(tag, fmt, ...) Logger::log(LOG_LEVEL_VERBOSE, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_VERBOSE_TAG(tag, fmt, ...) ((void)0)
#endif

// Location-aware logging macros (includes file and line number)
#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR_LOC(fmt, ...) Logger::logWithLocation(LOG_LEVEL_ERROR, "SYSTEM", __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR_LOC(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN_LOC(fmt, ...) Logger::logWithLocation(LOG_LEVEL_WARN, "SYSTEM", __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_WARN_LOC(fmt, ...) ((void)0)
#endif

#endif // LOGGER_H
