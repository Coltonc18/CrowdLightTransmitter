# CrowdLight Transmitter - Logging System Documentation

## Overview

A comprehensive, production-ready logging system for ESP32-S3 embedded systems with compile-time and runtime filtering, module tagging, timestamps, and thread-safe operation for FreeRTOS.

## Features

### Core Features

- **5 Log Levels**: NONE, ERROR, WARN, INFO, DEBUG, VERBOSE
- **Compile-time Filtering**: Reduces flash/RAM usage in production builds
- **Runtime Level Control**: Adjust verbosity without recompiling
- **Module Tagging**: Automatic module/component identification
- **Timestamps**: Millisecond-precision timestamps using `millis()`
- **Color-Coded Output**: ANSI terminal colors for easy visual parsing
- **Printf-style Formatting**: Full support for formatted strings
- **Thread-Safe**: Works correctly with FreeRTOS multi-core tasks
- **Log Statistics**: Track message counts and last error times
- **Error Buffer**: Circular buffer keeps last 50 ERROR/WARN messages

### Performance Optimizations

- Compile-time disabled logs compile to nothing (zero overhead)
- Runtime filtering is fast (single integer comparison)
- Stack-based buffers minimize heap allocations
- Minimal mutex locking overhead
- F() macro support for string constants (saves RAM)

## Configuration

### In `include/Config.h`

```cpp
// Set compile-time log level (affects binary size)
#define LOG_LEVEL LOG_LEVEL_DEBUG

// Enable/disable features
#define LOG_ENABLE_COLORS true       // ANSI color codes
#define LOG_ENABLE_TIMESTAMPS true   // Millisecond timestamps
#define LOG_BUFFER_SIZE 256          // Formatting buffer size
#define LOG_MAX_TAG_LENGTH 8         // Max tag name length
#define LOG_ERROR_BUFFER_SIZE 50     // Error history buffer

// Uncomment to run self-tests on boot
// #define DEBUG_TESTS
```

### Log Levels

| Level | Value | Description | Use Case |
|-------|-------|-------------|----------|
| `LOG_LEVEL_NONE` | 0 | No logging | Production builds |
| `LOG_LEVEL_ERROR` | 1 | Critical errors only | Production with minimal logging |
| `LOG_LEVEL_WARN` | 2 | Errors + warnings | Production with diagnostics |
| `LOG_LEVEL_INFO` | 3 | Important events | Default for most builds |
| `LOG_LEVEL_DEBUG` | 4 | Detailed debugging | Development builds |
| `LOG_LEVEL_VERBOSE` | 5 | Everything | Deep debugging |

## Usage

### Initialization

In `setup()`:

```cpp
Serial.begin(115200);
delay(100);
Logger::begin();
```

### Basic Logging

```cpp
LOG_ERROR("Critical failure occurred");
LOG_WARN("Configuration value out of range: %d", value);
LOG_INFO("System initialized successfully");
LOG_DEBUG("Buffer size: %d bytes", bufferSize);
LOG_VERBOSE("Packet data: %02X %02X %02X", data[0], data[1], data[2]);
```

### Tagged Logging (Recommended)

```cpp
LOG_ERROR_TAG("E131", "Failed to initialize Ethernet hardware");
LOG_WARN_TAG("RADIO", "HC-12 module not responding");
LOG_INFO_TAG("CONFIG", "Settings saved - Universe: %d", universe);
LOG_DEBUG_TAG("DISPLAY", "Button pressed: %d", buttonPin);
LOG_VERBOSE_TAG("NETWORK", "Packet received: %d channels", numChannels);
```

### Location-Aware Logging

For critical errors, include file/line information:

```cpp
LOG_ERROR_LOC("Assertion failed in packet parser");
LOG_WARN_LOC("Buffer overflow detected");
```

### Runtime Control

Change log level without recompiling:

```cpp
Logger::setLevel(LOG_LEVEL_VERBOSE);  // More verbose
Logger::setLevel(LOG_LEVEL_ERROR);    // Less verbose
LogLevel current = Logger::getLevel(); // Query current level
```

### Statistics

View logging statistics:

```cpp
Logger::printStats();  // Print to serial

// Query specific values
uint32_t errors = Logger::getErrorCount();
uint32_t warnings = Logger::getWarnCount();
unsigned long lastError = Logger::getLastErrorTime();
```

### Error History

Dump recent errors/warnings:

```cpp
Logger::dumpRecentErrors();  // Shows last 50 ERROR/WARN messages
```

### Testing

Run self-tests (when `DEBUG_TESTS` is defined):

```cpp
Logger::runTests();  // Tests all features
```

## Output Format

### With Timestamps (default)

```txt
[   1234] [INFO ] [SYSTEM ] System boot...
[   1256] [INFO ] [CONFIG ] Config loaded successfully
[   1267] [ERROR] [DISPLAY] OLED initialization failed
[   2341] [INFO ] [E131  ] Link UP - connected
[   2389] [INFO ] [NETWORK] Receiving on Universe 129
[   2405] [DEBUG] [E131  ] Packet: 512 channels, seq=42
[   3001] [WARN ] [RADIO ] Send timeout, retrying...
```

### Format Structure

```txt
[timestamp_ms] [LEVEL] [MODULE] message
```

## Module Tags

Standard tags used throughout the system:

| Tag | Module | Purpose |
|-----|--------|---------|
| `SYSTEM` | main.cpp | System-level events |
| `CONFIG` | ConfigManager | NVS configuration operations |
| `E131` | E131Handler | Ethernet/E1.31 protocol |
| `RADIO` | RadioLink | HC-12 radio communication |
| `DISPLAY` | DisplayMgr | OLED display and UI |
| `NETWORK` | main.cpp | Network task events |
| `INPUT` | main.cpp | Button input events |

## Compile-Time vs Runtime Filtering

### Compile-Time Filtering

Set `LOG_LEVEL` in `Config.h` to exclude log calls from binary:

```cpp
#define LOG_LEVEL LOG_LEVEL_ERROR  // Only ERROR messages compiled in
```

**Benefits:**

- Reduces flash usage (smaller binary)
- Reduces RAM usage (no string constants)
- Zero runtime overhead for disabled logs
- Messages completely removed from code

**Example:**
With `LOG_LEVEL` set to `ERROR`, this code:

```cpp
LOG_DEBUG("This message");
```

Compiles to:

```cpp
((void)0)  // Nothing - optimized away
```

### Runtime Filtering

Use `Logger::setLevel()` to filter at runtime:

```cpp
Logger::setLevel(LOG_LEVEL_VERBOSE);  // Show all compiled logs
```

**Benefits:**

- No recompilation needed
- Change verbosity on the fly
- Useful for debugging specific issues
- Can be controlled via serial commands or button

**Limitation:**

- Can only filter logs that were compiled in
- If compile-time level is ERROR, can't show DEBUG messages at runtime

## Best Practices

### 1. Choose Appropriate Levels

```cpp
// ✓ Good - Use ERROR for critical failures
LOG_ERROR_TAG("E131", "Hardware not detected - system halted");

// ✗ Bad - Don't use ERROR for expected conditions
LOG_ERROR_TAG("E131", "No packet received yet");  // Use DEBUG instead
```

### 2. Use Module Tags

```cpp
// ✓ Good - Clear module identification
LOG_INFO_TAG("RADIO", "HC-12 initialized");

// ✗ Bad - Generic tag
LOG_INFO("HC-12 initialized");  // Uses "SYSTEM" tag
```

### 3. Include Context

```cpp
// ✓ Good - Includes relevant values
LOG_ERROR_TAG("CONFIG", "Universe %d out of range [%d-%d]", 
              value, MIN_UNIVERSE, MAX_UNIVERSE);

// ✗ Bad - Vague error
LOG_ERROR_TAG("CONFIG", "Bad universe");
```

### 4. Avoid Flooding in Loops

```cpp
// ✗ Bad - Logs every iteration
for (int i = 0; i < 1000; i++) {
    LOG_DEBUG("Processing: %d", i);  // Too much output!
}

// ✓ Good - Log summary or use VERBOSE
LOG_DEBUG("Processing 1000 items...");
// or use LOG_VERBOSE for very detailed tracing
```

### 5. Performance-Critical Code

In time-critical sections (like network packet processing):

```cpp
// Use VERBOSE level for detailed tracing
LOG_VERBOSE_TAG("E131", "Packet: %d channels", len);

// Or use compile-time disabling in production
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
    LOG_DEBUG_TAG("E131", "Processing packet...");
#endif
```

## Migration from Serial.print()

### Before (Old Code)

```cpp
Serial.println("Starting system...");
Serial.printf("Error: %s\n", errorMsg);
Serial.print("Value: ");
Serial.println(value);
```

### After (New Code)

```cpp
LOG_INFO_TAG("SYSTEM", "Starting system...");
LOG_ERROR_TAG("SYSTEM", "Error: %s", errorMsg);
LOG_INFO_TAG("SYSTEM", "Value: %d", value);
```

### Search and Replace Guide

1. `Serial.println("text")` → `LOG_INFO_TAG("MODULE", "text")`
2. `Serial.printf("fmt", args)` → `LOG_INFO_TAG("MODULE", "fmt", args)`
3. Error messages → Use `LOG_ERROR_TAG`
4. Debug prints → Use `LOG_DEBUG_TAG` or `LOG_VERBOSE_TAG`

## Troubleshooting

### No Log Output

**Check:**

1. `Logger::begin()` called in `setup()`
2. `Serial.begin(115200)` called before Logger
3. Compile-time `LOG_LEVEL` includes desired messages
4. Runtime level allows messages: `Logger::setLevel(LOG_LEVEL_DEBUG)`

### Compilation Errors

**"Logger not found":**

- Ensure `#include "Logger.h"` in source files
- Check that `lib/Logger/Logger.cpp` exists

**"LOG_LEVEL not defined":**

- Add `#include "Config.h"` before `#include "Logger.h"`

### Performance Issues

**Logs slowing down system:**

- Set compile-time level to INFO or ERROR
- Reduce runtime level: `Logger::setLevel(LOG_LEVEL_INFO)`
- Remove VERBOSE logs from tight loops

**Out of memory:**

- Reduce `LOG_BUFFER_SIZE` (default 256)
- Reduce `LOG_ERROR_BUFFER_SIZE` (default 50)
- Lower compile-time LOG_LEVEL

## Advanced Features

### Custom Module Levels (Future Enhancement)

Placeholder for per-module level control:

```cpp
Logger::setModuleLevel("E131", LOG_LEVEL_VERBOSE);
Logger::setModuleLevel("RADIO", LOG_LEVEL_ERROR);
```

*Note: Currently logs request but not implemented. Would require hash map.*

### Thread Safety

The logger uses FreeRTOS mutexes for thread safety:

- Safe to call from multiple tasks
- Minimal blocking time
- No deadlocks with proper usage

### ISR Safety

**Important:** Do NOT call logging functions from interrupts (ISR).

- Mutexes cannot be used in ISR context
- Serial output is blocking
- Will cause system crashes

## Performance Metrics

### Flash Usage

| Configuration | Approximate Size Impact |
|---------------|-------------------------|
| LOG_LEVEL_NONE | ~0 KB (removed) |
| LOG_LEVEL_ERROR | ~2-4 KB |
| LOG_LEVEL_INFO | ~6-10 KB |
| LOG_LEVEL_DEBUG | ~12-20 KB |
| LOG_LEVEL_VERBOSE | ~20-30 KB |

#### Actual size depends on number of log statements

### RAM Usage

- Logger class: ~350 bytes (static data)
- Error buffer: ~3.2 KB (50 * 64 bytes)
- Stack per call: ~256 bytes (formatting buffer)
- Mutex overhead: ~100 bytes

#### Total: ~3.9 KB static, ~256 bytes per concurrent log

### CPU Overhead

- Runtime level check: <1 µs
- Message formatting: ~50-200 µs (depends on complexity)
- Serial output: ~1-5 ms (blocking, depends on baud rate)

**For VERBOSE level in tight loops**: Consider performance impact of serial output.

## Examples

### Complete System Initialization

```cpp
void setup() {
    Serial.begin(115200);
    delay(100);
    
    Logger::begin();
    LOG_INFO_TAG("SYSTEM", "=== CrowdLight Transmitter ===");
    LOG_INFO_TAG("SYSTEM", "Firmware Version: 1.0.0");
    
#ifdef DEBUG_TESTS
    Logger::runTests();
#endif
    
    LOG_INFO_TAG("SYSTEM", "Initializing subsystems...");
    
    // Initialize modules...
}
```

### Error Handling with Logging

```cpp
bool initializeHardware() {
    if (!eth.begin()) {
        LOG_ERROR_TAG("E131", "Ethernet initialization failed");
        LOG_ERROR_TAG("SYSTEM", "Cannot continue - hardware required");
        return false;
    }
    
    LOG_INFO_TAG("E131", "Ethernet initialized successfully");
    return true;
}
```

### Debugging Network Issues

```cpp
// Change to VERBOSE level for detailed network debugging
Logger::setLevel(LOG_LEVEL_VERBOSE);

// All VERBOSE logs now appear
while (debugging) {
    int len = eth.parsePacket(buffer);
    // LOG_VERBOSE in parsePacket() now shows up
}

// Return to normal level
Logger::setLevel(LOG_LEVEL_INFO);
```

## Support

For issues or questions:

1. Check compilation errors in PlatformIO
2. Verify configuration in `Config.h`
3. Review this documentation
4. Check serial output for log messages

---

**Version:** 1.0  
**Last Updated:** November 2025  
**Author:** CrowdLight Development Team
