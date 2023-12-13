#include "log_target_file.hpp"
#include <log/log_entry.hpp>

jt::LogTargetFile::LogTargetFile()
    : m_file { std::ofstream { "log.txt" } }
{
}

jt::LogTargetFile::~LogTargetFile()
{
    try {
        m_file.flush();
    } catch (...) {
        // ignore exceptions in destructor
    }
}

void jt::LogTargetFile::log(jt::LogEntry const& entry)
{
    if (entry.level > m_logLevel) {
        return;
    }
    m_file << entry.time << ": ";
    for (auto& t : entry.tags) {
        m_file << "<" + t + ">";
    }
    if (!entry.tags.empty()) {
        m_file << ": ";
    }
    m_file << entry.message << "\n";
}

void jt::LogTargetFile::setLogLevel(LogLevel level) { m_logLevel = level; }
