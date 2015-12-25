/*
 * ReLogger.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RELOGGER_HPP
#define RELOGGER_HPP

/**
 *
 */
class ReLogger;
class ReConfig;

/**
 * @brief Logging level: for controlling of the logging.
 *
 * Each logging location defines one of the following level.
 * If the level of an appender is lower or equals to this level
 * the logging is done.
 */
enum ReLoggerLevel {
   LOG_ERROR = 10,         ///< marks an error.
   LOG_WARNING = 15,       ///< marks a warning
   LOG_INFO = 20,          ///< marks an information
   LOG_DEBUG = 25          ///< for debug purpose only
};

/**
 * Very poor logger normally used for GUI programs with status line.
 *
 * This is a pure abstract class: implementation of the method say() is needed.
 */
class ReAnnouncer {
public:
   /**
    * Issues a message.
    *
    * @param level		type of the message, e.g. LOG_INFO or LOG_ERROR
    * @param message	the message to issue. Use QString::arg() to format,
    *					e.g. <code>say(QString("name: %1 no: %2).arg(name).arg(no)</code>
    * @return			<code>false</code>: level == LOG_ERROR or LOG_WARNING<br>
    *					<code>true</code>: level >= LOG_INFO
    */
   virtual bool say(ReLoggerLevel level, const QString& message) = 0;
};

class ReAppender {
public:
   ReAppender(const QByteArray& name);
   virtual ~ReAppender();
private:
   // No copy constructor: no implementation!
   ReAppender(const ReAppender& source);
   // Prohibits assignment operator: no implementation!
   ReAppender& operator =(const ReAppender& source);
public:
   virtual void log(ReLoggerLevel level, int location, const char* message,
                    ReLogger* logger) = 0;
   bool isActive(ReLoggerLevel level);
   void setLevel(ReLoggerLevel level);
   void setAutoDelete(bool onNotOff);
   bool isAutoDelete() const;
   ReLoggerLevel getLevel() const;
   const char* getName() const;

private:
   // Name of the appender. Used to find the appender in a list of appenders
   QByteArray m_name;
   // only locations with a lower or equal level will be logged
   ReLoggerLevel m_level;
   // true: the logger destroys the instance. false: the deletion must be done outside of the logger
   bool m_autoDelete;
};

class ReLogger {
public:
   ReLogger(bool isGlobal = true);
   virtual ~ReLogger();
private:
   // No copy constructor: no implementation!
   ReLogger(const ReLogger& source);
   // Prohibits assignment operator: no implementation!
   ReLogger& operator =(const ReLogger& source);
public:
   bool log(ReLoggerLevel level, int location, const char* message);
   bool log(ReLoggerLevel level, int location, const QByteArray& message);
   bool log(ReLoggerLevel level, int location, const ReString& message);
   bool logv(ReLoggerLevel level, int location, const char* format, ...);
   bool logv(ReLoggerLevel level, int location, const QByteArray& format, ...);
   bool log(ReLoggerLevel level, int location, const char* format,
            va_list& varlist);
   void addAppender(ReAppender* appender);
   ReAppender* findAppender(const char* name) const;
   void buildStandardAppender(ReConfig* config,
                              const char* prefix = "logfile.", const char* defaultLoggerName =
                                 "logger");
   void buildStandardAppender(const QByteArray& prefix,
                              int maxSize = 10 * 1024 * 1024, int maxCount = 5);
   QByteArray buildStdPrefix(ReLoggerLevel level, int location);
   const QByteArray& getStdPrefix(ReLoggerLevel level, int location);
   char getPrefixOfLevel(ReLoggerLevel level) const;
   bool isActive(ReLoggerLevel level) const;
   void setLevel(ReLoggerLevel level);
   void setWithLocking(bool onNotOff);
public:
   static ReLogger* globalLogger();
   static void destroyGlobalLogger();
private:
   // the standard logger, can be called (with globalLogger()) from each location
   static ReLogger* m_globalLogger;
private:
   // the assigned appenders:
   ReAppender* m_appenders[16];
   // the number of appenders in m_appenders:
   size_t m_countAppenders;
   // "" or the cache of the prefix of the current logging line: This can be reused by any appender.
   QByteArray m_stdPrefix;
   QMutex m_mutex;
   bool m_withLocking;
};

/**
 * Implements an appender which puts the messages to a standard stream: stdout or stderr
 */
class ReStreamAppender: public ReAppender {
public:
   ReStreamAppender(FILE* stream, const char* appenderName = "FileAppender");
   virtual ~ReStreamAppender();
public:
   virtual void log(ReLoggerLevel level, int location, const char* message,
                    ReLogger* logger);
private:
   // stdout or stderr:
   FILE* m_fp;
};

/**
 * Implements an appender which puts the messages to a file
 */
class ReFileAppender: public ReAppender {
public:
   ReFileAppender(const QByteArray& name, int maxSize, int maxCount,
                  const char* appenderName = "FileAppender");
   virtual ~ReFileAppender();
public:
   void open();
   virtual void log(ReLoggerLevel level, int location, const char* message,
                    ReLogger* logger);

private:
   // prefix of the log file name. Will be appended by ".<no>.log"
   QByteArray m_prefix;
   // maximal size of a logging file:
   int m_maxSize;
   // maximal count of logging files. If neccessary the oldest file will be deleted.
   int m_maxCount;
   // the size of the current log file:
   int m_currentSize;
   // the number of the current log file:
   int m_currentNo;
   // the current log file:
   FILE* m_fp;
};

/**
 * Stores the log messages in a list.
 */
class ReMemoryAppender: public ReAppender {
public:
   ReMemoryAppender(int maxLines = 1024, const char* appenderName =
                       "MemoryAppender");
   ~ReMemoryAppender();
public:
   virtual void log(ReLoggerLevel level, int location, const char* message,
                    ReLogger* logger);
   const QList<QByteArray>& getLines() const;
   void clear();
private:
   QList<QByteArray> m_lines;
   // maximum count of m_lines. If larger the oldest lines will be deleted.
   int m_maxLines;
   // true: standard prefix (level + datetime) will be stored too.
   bool m_addPrefix;
};

/**
 * Write the log messages to the QT debug output.
 */
class ReDebugAppender: public ReAppender {
public:
   ReDebugAppender(const char* appenderName = "DebugAppender");
   ~ReDebugAppender();
public:
   virtual void log(ReLoggerLevel level, int location, const char* message,
                    ReLogger* logger);
};

class ReMemoryLogger : public ReLogger, public ReMemoryAppender {
public:
   ReMemoryLogger();
};


#endif // RELOGGER_HPP
