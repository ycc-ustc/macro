#ifndef __MACRO_LOG_H__
#define __MACRO_LOG_H__

#include <cstdarg>
#include <cstdint>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "singleton.h"

#define MACRO_LOG_LEVEL(logger, level)                                     \
    if (logger->getLevel() <= level)                                       \
    macro::LogEventWrapper(                                                \
        macro::LogEvent::ptr(new macro::LogEvent(                          \
            logger, level, __FILE__, __LINE__, 0, macro::GetThreadId(),    \
            macro::GetFiberId(), time(nullptr))))                          \
        .getSS()

#define MACRO_LOG_DEBUG(logger)                                            \
    MACRO_LOG_LEVEL(logger, macro::LogLevel::DEBUG)
#define MACRO_LOG_INFO(logger)                                             \
    MACRO_LOG_LEVEL(logger, macro::LogLevel::INFO)
#define MACRO_LOG_WARN(logger)                                             \
    MACRO_LOG_LEVEL(logger, macro::LogLevel::WARN)
#define MACRO_LOG_ERROR(logger)                                            \
    MACRO_LOG_LEVEL(logger, macro::LogLevel::ERROR)
#define MACRO_LOG_FATAL(logger)                                            \
    MACRO_LOG_LEVEL(logger, macro::LogLevel::FATAL)

#define MACRO_LOG_FMT_LEVEL(logger, level, fmt, ...)                       \
    if (logger->getLevel() <= level)                                       \
    macro::LogEventWrapper(                                                \
        macro::LogEvent::ptr(new macro::LogEvent(                          \
            logger, level, __FILE__, __LINE__, 0, macro::GetThreadId(),    \
            macro::GetFiberId(), time(nullptr))))                          \
        .getEvent()                                                        \
        ->format(fmt, __VA_ARGS__)

#define MACRO_LOG_FMT_DEBUG(logger, fmt, ...)                              \
    MACRO_LOG_FMT_LEVEL(logger, macro::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define MACRO_LOG_FMT_INFO(logger, fmt, ...)                               \
    MACRO_LOG_FMT_LEVEL(logger, macro::LogLevel::INFO, fmt, __VA_ARGS__)
#define MACRO_LOG_FMT_WARN(logger, fmt, ...)                               \
    MACRO_LOG_FMT_LEVEL(logger, macro::LogLevel::WARN, fmt, __VA_ARGS__)
#define MACRO_LOG_FMT_ERROR(logger, fmt, ...)                              \
    MACRO_LOG_FMT_LEVEL(logger, macro::LogLevel::ERROR, fmt, __VA_ARGS__)
#define MACRO_LOG_FMT_FATAL(logger, fmt, ...)                              \
    MACRO_LOG_FMT_LEVEL(logger, macro::LogLevel::FATAL, fmt, __VA_ARGS__)
namespace macro {
class Logger;

// 日志级别
class LogLevel {
public:
    enum Level { DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5 };

    static const char* ToString(LogLevel::Level level);
};

// 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
             const char* file, int32_t line, uint32_t elapse,
             uint32_t thread_id, uint32_t fiber_id, uint64_t time);

    const char* getFile() const {
        return m_file;
    }
    int32_t getLine() const {
        return m_line;
    }
    uint32_t getElapse() const {
        return m_elapse;
    }
    uint32_t getThreadId() const {
        return m_threadId;
    }
    uint32_t getFiberId() const {
        return m_fiberId;
    }
    uint64_t getTime() const {
        return m_time;
    }
    const std::string getContent() {
        return m_ss.str();
    }
    const std::string& getThreadName() const {
        return m_threadName;
    }
    // 返回日志内容字符串流
    std::stringstream& getSS() {
        return m_ss;
    }
    std::shared_ptr<Logger> getLogger() {
        return m_logger;
    }

    LogLevel::Level getLevel() {
        return m_level;
    }
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

private:
    const char* m_file = nullptr;  // 文件名
    int32_t     m_line = 0;        // 行号
    uint32_t    m_elapse = 0;    // 程序启动开始到现在的毫秒数
    uint32_t    m_threadId = 0;  // 线程id
    uint32_t    m_fiberId = 0;   // 协程id
    uint64_t    m_time;          // 时间戳
    std::string m_threadName;    // 线程名称
    std::stringstream       m_ss;  // 日志内容流
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level         m_level;
};

class LogEventWrapper {
public:
    LogEventWrapper(LogEvent::ptr event);
    ~LogEventWrapper();
    std::stringstream& getSS();
    LogEvent::ptr      getEvent() {
        return m_event;
    }

private:
    LogEvent::ptr m_event;
};

// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);

    std::string format(std::shared_ptr<Logger> logger,
                       LogLevel::Level level, LogEvent::ptr event);

    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        virtual ~FormatItem() {}
        virtual void format(std::ostream&           os,
                            std::shared_ptr<Logger> logger,
                            LogLevel::Level level, LogEvent::ptr event) = 0;
    };

private:
    void                         init();
    std::string                  m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool                         m_error = false;
};

// 日志输出地
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender() {}
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr val) {
        m_formatter = val;
    }
    LogFormatter::ptr getFormatter() const {
        return m_formatter;
    }

    void setLevel(LogLevel::Level level) {
        m_level = level;
    }
    LogLevel::Level getLevel() {
        return m_level;
    }

protected:
    LogLevel::Level   m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
             LogEvent::ptr event) override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
             LogEvent::ptr event) override;
    // 文件打开成功 返回true
    bool reopen();

private:
    std::string   m_filename;
    std::ofstream m_filestream;
};

// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std::string& name = "root");

    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    std::string getName() const {
        return m_name;
    }
    LogLevel::Level getLevel() const {
        return m_level;
    };
    void setLevel(LogLevel::Level val) {
        m_level = val;
    }

private:
    std::string                 m_name;       // 日志名称
    LogLevel::Level             m_level;      // 日志级别
    std::list<LogAppender::ptr> m_appenders;  // appender集合
    LogFormatter::ptr           m_formatter;
};
class LogManager {
public:
    LogManager();
    Logger::ptr getLogger(const std::string& name);
    void        init();

private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr                        m_root;
};

typedef macro::Singleton<LogManager> LoggerMgr;
}  // namespace macro
#endif