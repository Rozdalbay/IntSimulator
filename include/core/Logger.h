#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <mutex>

namespace civ {

enum class LogLevel : uint8_t {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

/**
 * @brief Thread-safe singleton logger with file and in-memory log support.
 */
class Logger {
public:
    static Logger& instance();

    // Non-copyable, non-movable
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void init(const std::string& filename, LogLevel minLevel = LogLevel::Info);
    void shutdown();

    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

    [[nodiscard]] const std::vector<std::string>& getRecentLogs() const;
    void clearRecentLogs();

    [[nodiscard]] std::string levelToString(LogLevel level) const;

private:
    Logger() = default;
    ~Logger();

    std::ofstream m_file;
    std::vector<std::string> m_recentLogs;
    LogLevel m_minLevel = LogLevel::Info;
    mutable std::mutex m_mutex;
    bool m_initialized = false;

    static constexpr size_t MAX_RECENT_LOGS = 50;
};

} // namespace civ
