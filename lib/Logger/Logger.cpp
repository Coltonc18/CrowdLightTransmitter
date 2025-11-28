#include "Logger.h"

// Static member initialization
LogLevel Logger::_runtimeLevel = LOG_LEVEL_INFO;
Logger::LogStats Logger::_stats = {0, 0, 0, 0, 0, 0, 0};
SemaphoreHandle_t Logger::_mutex = nullptr;
Logger::ErrorLogEntry Logger::_errorBuffer[LOG_ERROR_BUFFER_SIZE] = {};
uint8_t Logger::_errorBufferIndex = 0;

void Logger::begin() {
    // Create mutex for thread safety
    _mutex = xSemaphoreCreateMutex();
    
    // Initialize error buffer
    for (int i = 0; i < LOG_ERROR_BUFFER_SIZE; i++) {
        _errorBuffer[i].valid = false;
    }
    
    // Set initial runtime level to match compile-time level
    _runtimeLevel = (LogLevel)LOG_LEVEL;
    
    LOG_INFO_TAG("LOGGER", "Logger initialized - Level: %d", _runtimeLevel);
}

void Logger::log(LogLevel level, const char* tag, const char* format, ...) {
    // Runtime level check
    if (level > _runtimeLevel) {
        return;
    }
    
    // Thread safety
    if (_mutex != nullptr) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
    }
    
    // Build the message
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    
    // Get color and level string
    const char* color = getLevelColor(level);
    const char* levelStr = getLevelString(level);
    
    // Format and print the log message
#if LOG_ENABLE_TIMESTAMPS
    unsigned long timestamp = millis();
    Serial.printf("%s[%8lu] [%-5s] [%-*s] %s%s\r\n", 
                  color, timestamp, levelStr, LOG_MAX_TAG_LENGTH, tag, buffer, ANSI_COLOR_RESET);
#else
    Serial.printf("%s[%-5s] [%-*s] %s%s\r\n", 
                  color, levelStr, LOG_MAX_TAG_LENGTH, tag, buffer, ANSI_COLOR_RESET);
#endif
    
    // Update statistics
    updateStats(level);
    
    // Add to error buffer if ERROR or WARN
    if (level <= LOG_LEVEL_WARN) {
        addToErrorBuffer(tag, buffer);
    }
    
    // Release mutex
    if (_mutex != nullptr) {
        xSemaphoreGive(_mutex);
    }
}

void Logger::logWithLocation(LogLevel level, const char* tag, const char* file, 
                            int line, const char* format, ...) {
    // Runtime level check
    if (level > _runtimeLevel) {
        return;
    }
    
    // Thread safety
    if (_mutex != nullptr) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
    }
    
    // Build the message
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    
    // Extract just filename from full path
    const char* filename = strrchr(file, '/');
    if (filename) {
        filename++;
    } else {
        filename = strrchr(file, '\\');
        if (filename) {
            filename++;
        } else {
            filename = file;
        }
    }
    
    // Get color and level string
    const char* color = getLevelColor(level);
    const char* levelStr = getLevelString(level);
    
    // Format and print with location
#if LOG_ENABLE_TIMESTAMPS
    unsigned long timestamp = millis();
    Serial.printf("%s[%8lu] [%-5s] [%-*s] %s (%s:%d)%s\r\n", 
                  color, timestamp, levelStr, LOG_MAX_TAG_LENGTH, tag, 
                  buffer, filename, line, ANSI_COLOR_RESET);
#else
    Serial.printf("%s[%-5s] [%-*s] %s (%s:%d)%s\r\n", 
                  color, levelStr, LOG_MAX_TAG_LENGTH, tag, 
                  buffer, filename, line, ANSI_COLOR_RESET);
#endif
    
    // Update statistics
    updateStats(level);
    
    // Add to error buffer if ERROR or WARN
    if (level <= LOG_LEVEL_WARN) {
        addToErrorBuffer(tag, buffer);
    }
    
    // Release mutex
    if (_mutex != nullptr) {
        xSemaphoreGive(_mutex);
    }
}

void Logger::setLevel(LogLevel level) {
    _runtimeLevel = level;
    LOG_INFO_TAG("LOGGER", "Runtime log level changed to %d", level);
}

LogLevel Logger::getLevel() {
    return _runtimeLevel;
}

void Logger::setModuleLevel(const char* module, LogLevel level) {
    // Note: Full per-module level control would require a hash map
    // For now, we just log the request. Could be extended with a static array
    LOG_INFO_TAG("LOGGER", "Module '%s' level set to %d (not yet implemented)", module, level);
}

LogLevel Logger::getModuleLevel(const char* module) {
    // Return global level for now
    return _runtimeLevel;
}

void Logger::printStats() {
    Serial.println(F("\r\n=== LOGGER STATISTICS ==="));
    Serial.printf("ERROR   : %lu\r\n", _stats.errorCount);
    Serial.printf("WARN    : %lu\r\n", _stats.warnCount);
    Serial.printf("INFO    : %lu\r\n", _stats.infoCount);
    Serial.printf("DEBUG   : %lu\r\n", _stats.debugCount);
    Serial.printf("VERBOSE : %lu\r\n", _stats.verboseCount);
    Serial.printf("Last ERROR: %lu ms\r\n", _stats.lastErrorTime);
    Serial.printf("Last WARN : %lu ms\r\n", _stats.lastWarnTime);
    Serial.printf("Uptime    : %lu ms\r\n", millis());
    Serial.println(F("========================\r\n"));
}

void Logger::resetStats() {
    if (_mutex != nullptr) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
    }
    
    _stats.errorCount = 0;
    _stats.warnCount = 0;
    _stats.infoCount = 0;
    _stats.debugCount = 0;
    _stats.verboseCount = 0;
    _stats.lastErrorTime = 0;
    _stats.lastWarnTime = 0;
    
    if (_mutex != nullptr) {
        xSemaphoreGive(_mutex);
    }
    
    LOG_INFO_TAG("LOGGER", "Statistics reset");
}

uint32_t Logger::getErrorCount() {
    return _stats.errorCount;
}

uint32_t Logger::getWarnCount() {
    return _stats.warnCount;
}

unsigned long Logger::getLastErrorTime() {
    return _stats.lastErrorTime;
}

void Logger::dumpRecentErrors() {
    Serial.println(F("\r\n=== RECENT ERRORS/WARNINGS ==="));
    
    bool foundAny = false;
    for (int i = 0; i < LOG_ERROR_BUFFER_SIZE; i++) {
        // Start from current index and wrap around
        int idx = (_errorBufferIndex + i) % LOG_ERROR_BUFFER_SIZE;
        if (_errorBuffer[idx].valid) {
            foundAny = true;
            Serial.printf("[%8lu] [%s] %s\r\n", 
                         _errorBuffer[idx].timestamp,
                         _errorBuffer[idx].tag,
                         _errorBuffer[idx].message);
        }
    }
    
    if (!foundAny) {
        Serial.println(F("No errors/warnings logged"));
    }
    
    Serial.println(F("==============================\r\n"));
}

void Logger::runTests() {
    Serial.println(F("\r\n=== LOGGER TESTS ==="));
    
    // Test all log levels
    LOG_ERROR("Test ERROR message");
    delay(10);
    LOG_WARN("Test WARN message");
    delay(10);
    LOG_INFO("Test INFO message");
    delay(10);
    LOG_DEBUG("Test DEBUG message");
    delay(10);
    LOG_VERBOSE("Test VERBOSE message");
    delay(10);
    
    // Test with tags
    LOG_ERROR_TAG("TEST", "Error with tag");
    delay(10);
    LOG_INFO_TAG("TEST", "Info with tag");
    delay(10);
    
    // Test formatting
    LOG_INFO_TAG("TEST", "Formatted: %d %s %f", 42, "hello", 3.14);
    delay(10);
    
    // Test location
    LOG_ERROR_LOC("Error with location info");
    delay(10);
    
    // Test statistics
    printStats();
    
    // Test error buffer
    dumpRecentErrors();
    
    // Test runtime level change
    LogLevel originalLevel = getLevel();
    Serial.println(F("\r\nTesting runtime level change to VERBOSE..."));
    setLevel(LOG_LEVEL_VERBOSE);
    LOG_VERBOSE_TAG("TEST", "This should appear at VERBOSE level");
    
    Serial.println(F("Testing runtime level change to ERROR..."));
    setLevel(LOG_LEVEL_ERROR);
    LOG_INFO_TAG("TEST", "This should NOT appear (INFO when level is ERROR)");
    LOG_ERROR_TAG("TEST", "This should appear (ERROR when level is ERROR)");
    
    // Restore original level
    setLevel(originalLevel);
    
    Serial.println(F("\r\n=== TESTS COMPLETE ===\r\n"));
}

// Private helper methods

const char* Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_ERROR:   return "ERROR";
        case LOG_LEVEL_WARN:    return "WARN";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_VERBOSE: return "VERB";
        default:                return "NONE";
    }
}

const char* Logger::getLevelColor(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_ERROR:   return ANSI_COLOR_RED;
        case LOG_LEVEL_WARN:    return ANSI_COLOR_YELLOW;
        case LOG_LEVEL_INFO:    return ANSI_COLOR_GREEN;
        case LOG_LEVEL_DEBUG:   return ANSI_COLOR_CYAN;
        case LOG_LEVEL_VERBOSE: return ANSI_COLOR_MAGENTA;
        default:                return ANSI_COLOR_RESET;
    }
}

void Logger::addToErrorBuffer(const char* tag, const char* message) {
    ErrorLogEntry& entry = _errorBuffer[_errorBufferIndex];
    entry.timestamp = millis();
    strncpy(entry.tag, tag, LOG_MAX_TAG_LENGTH);
    entry.tag[LOG_MAX_TAG_LENGTH] = '\0';
    strncpy(entry.message, message, sizeof(entry.message) - 1);
    entry.message[sizeof(entry.message) - 1] = '\0';
    entry.valid = true;
    
    _errorBufferIndex = (_errorBufferIndex + 1) % LOG_ERROR_BUFFER_SIZE;
}

void Logger::updateStats(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_ERROR:
            _stats.errorCount++;
            _stats.lastErrorTime = millis();
            break;
        case LOG_LEVEL_WARN:
            _stats.warnCount++;
            _stats.lastWarnTime = millis();
            break;
        case LOG_LEVEL_INFO:
            _stats.infoCount++;
            break;
        case LOG_LEVEL_DEBUG:
            _stats.debugCount++;
            break;
        case LOG_LEVEL_VERBOSE:
            _stats.verboseCount++;
            break;
        default:
            break;
    }
}
