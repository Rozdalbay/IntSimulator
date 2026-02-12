#include "core/Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace civ {

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::~Logger() {
    shutdown();
}

void Logger::init(const std::string& filename, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized) {
        return;
    }
    m_file.open(filename, std::ios::out | std::ios::trunc);
    if (!m_file.is_open()) {
        std::cerr << "[Logger] Failed to open log file: " << filename << std::endl;
        return;
    }
    m_minLevel = minLevel;
    m_initialized = true;
    m_file << "=== Civilization Simulator Log ===" << std::endl;
    m_file << "Session started" << std::endl;
    m_file << "=================================" << std::endl;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) {
        m_file << "=== Session ended ===" << std::endl;
        m_file.close();
    }
    m_initialized = false;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_minLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // Build timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%H:%M:%S");
    std::string timestamp = oss.str();

    std::string logLine = "[" + timestamp + "] [" + levelToString(level) + "] " + message;

    // Write to file
    if (m_file.is_open()) {
        m_file << logLine << std::endl;
        m_file.flush();
    }

    // Store in recent logs
    m_recentLogs.push_back(logLine);
    if (m_recentLogs.size() > MAX_RECENT_LOGS) {
        m_recentLogs.erase(m_recentLogs.begin());
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::Critical, message);
}

const std::vector<std::string>& Logger::getRecentLogs() const {
    return m_recentLogs;
}

void Logger::clearRecentLogs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_recentLogs.clear();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO ";
        case LogLevel::Warning:  return "WARN ";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT ";
        default:                 return "?????";
    }
}

} // namespace civ
