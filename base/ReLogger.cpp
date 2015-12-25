/*
 * ReLogger.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * A configurable logger for different output media.
 */
/** @file base/ReLogger.hpp
 *
 * Definitions for a configurable logger for different output media.
 */
#include "base/rebase.hpp"
#include <QDir>
#include <iostream>

enum {
   LOC_ADD_APPENDER_1 = LOC_FIRST_OF(LOC_LOGGER), // 10101
};

ReLogger* ReLogger::m_globalLogger = NULL;

/**
 * @brief Returns the global logger.
 *
 * If it does not exist it will be created (singleton).
 *
 * @return the global logger
 */
ReLogger* ReLogger::globalLogger() {
   if (m_globalLogger == NULL) {
      m_globalLogger = new ReLogger();
      m_globalLogger->buildStandardAppender("globallogger");
   }
   return m_globalLogger;
}
/**
 * @brief Frees the resources of the global logger.
 */
void ReLogger::destroyGlobalLogger() {
   delete m_globalLogger;
   m_globalLogger = NULL;
}

/** @class ReAppender ReLogger.hpp "base/ReLogger.hpp"
 *
 * @brief Puts the logging info to a medium (e.g. a file).
 *
 * This is an abstract base class.
 */

/**
 * @brief Constructor.
 *
 * @param name		identifies the logger. Useful for ReLogger::findLogger()
 */
ReAppender::ReAppender(const QByteArray& name) :
   m_name(name),
   m_level(LOG_INFO),
   m_autoDelete(false) {
}
/**
 * @brief Destructor.
 */
ReAppender::~ReAppender() {
}

/**
 * Returns the name.
 *
 * @return		the name of the instance
 */
const char* ReAppender::getName() const {
   return m_name.data();
}

/**
 * @brief Sets the level.
 *
 * @param level
 */
void ReAppender::setLevel(ReLoggerLevel level) {
   m_level = level;
}
/**
 * @brief Returns the level.
 *
 * @return 		the level
 */
ReLoggerLevel ReAppender::getLevel() const {
   return m_level;
}
/**
 * @brief Checks whether the current location should be logged.
 *
 * @param level		the level of the location.
 * @return			true: the location level is greater or equals to the appender's level
 */
bool ReAppender::isActive(ReLoggerLevel level) {
   return level <= m_level;
}

/**
 * @brief Sets or clears the automatic deletions.
 *
 * @param onNotOff		the state of the auto deletion
 */
void ReAppender::setAutoDelete(bool onNotOff) {
   m_autoDelete = onNotOff;
}

/**
 * @brief Returns the state of the auto deletion.
 *
 * @return	true: the logger destroys the instance
 */
bool ReAppender::isAutoDelete() const {
   return m_autoDelete;
}

/** @class ReLogger ReLogger.hpp "base/ReLogger.hpp"
 *
 * @brief Implements a logger.
 *
 * The logger takes the call from the calling location.
 * But the output assumes the class <code>ReAppender</code>,
 * more exactly: a subclass from the abstract class
 * <code>ReAppender</code>,
 *
 * For single threaded applications there is a possability of
 * a global logger. In this case the logger can be got with the static
 * method <code>ReLogger::globalLogger()</code>.
 *
 * <b>Note</b>: using the global logger is <b>not threadsafe</b>!
 *
 * Each call of the logger should be provided by a <b>unique identifier</b>
 * named the <b>location</b>. This allows to find the error quickly.
 */

/**
 * @brief Constructor.
 *
 * @param isGlobal  <code>true</code>: the logger becomes the global logger
 */
ReLogger::ReLogger(bool isGlobal) :
   // m_appenders(),
   m_countAppenders(0),
   m_stdPrefix(),
   m_mutex(),
   m_withLocking(false) {
   memset(m_appenders, 0, sizeof m_appenders);
   if (isGlobal) {
      m_globalLogger = this;
   }
}

/**
 * @brief Destructor.
 */
ReLogger::~ReLogger() {
   for (size_t ix = 0; ix < m_countAppenders; ix++) {
      ReAppender* appender = m_appenders[ix];
      if (appender->isAutoDelete()) {
         delete appender;
      }
      m_appenders[ix] = NULL;
   }
}
/**
 * @brief Returns the first char of a logging line displaying the logging level.
 *
 * @param level		the level to "convert"
 * @return 			the assigned prefix char
 */
char ReLogger::getPrefixOfLevel(ReLoggerLevel level) const {
   char rc = ' ';
   switch (level) {
   case LOG_ERROR:
      rc = '!';
      break;
   case LOG_WARNING:
      rc = '+';
      break;
   case LOG_INFO:
      rc = ' ';
      break;
   case LOG_DEBUG:
      rc = '=';
      break;
   default:
      rc = '?';
      break;
   }
   return rc;
}

/**
 * @brief Tests whether at least one appender is active for a given level.
 *
 * @param level     level to test
 * @return          false: all appenders are not activated by this level<br>
 *                  true: otherwise
 */
bool ReLogger::isActive(ReLoggerLevel level) const {
   bool rc = false;
   for (size_t ix = 0; ix < m_countAppenders; ix++) {
      ReAppender* appender = m_appenders[ix];
      if (appender->isActive(level)) {
         rc = true;
         break;
      }
   }
   return rc;
}

/**
 * @brief Sets the log level for all appenders.
 *
 * @param level     level to set
 */
void ReLogger::setLevel(ReLoggerLevel level) {
   for (size_t ix = 0; ix < m_countAppenders; ix++) {
      ReAppender* appender = m_appenders[ix];
      appender->setLevel(level);
   }
}

/**
 * @brief Sets or clears the state "with locking".
 *
 * @param onNotOff   true: the logger is thread save.<br>
 *                   false: not thread save
 */
void ReLogger::setWithLocking(bool onNotOff) {
   m_withLocking = onNotOff;
}

/**
 * @brief Returns the standard prefix of a logging line.
 *
 * If it does not exist it will be created.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @return 			the standard logging line prefix
 */
const QByteArray& ReLogger::getStdPrefix(ReLoggerLevel level, int location) {
   if (m_stdPrefix.isEmpty())
      m_stdPrefix = buildStdPrefix(level, location);
   return m_stdPrefix;
}

/**
 * @brief Logs (or not) the calling location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param message	the logging message
 * @return			true: for chaining
 */
bool ReLogger::log(ReLoggerLevel level, int location, const char* message) {
   m_stdPrefix = "";
   bool first = true;
   for (size_t ix = 0; ix < m_countAppenders; ix++) {
      ReAppender* appender = m_appenders[ix];
      if (appender->isActive(level)) {
         if (first && m_withLocking)
            m_mutex.lock();
         appender->log(level, location, message, this);
      }
   }
   if (!first && m_withLocking)
      m_mutex.unlock();
   return true;
}
/**
 * @brief Logs (or not) the calling location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param message	the logging message
 * @return			true: for chaining
 */
bool ReLogger::log(ReLoggerLevel level, int location,
                   const QByteArray& message) {
   return log(level, location, message.data());
}

/**
 * @brief Logs (or not) the calling location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param message	the logging message
 * @return			true: for chaining
 */
bool ReLogger::log(ReLoggerLevel level, int location, const ReString& message) {
   return log(level, location, I18N::s2b(message).data());
}

/**
 * @brief Logs (or not) the calling location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param format	the logging message with placeholders (like printf).
 * @param ...		the values of the placeholders (varargs)
 * @return			true: for chaining
 */
bool ReLogger::logv(ReLoggerLevel level, int location, const char* format,
                    ...) {
   char buffer[64000];
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   return log(level, location, buffer);
}

/**
 * @brief Logs (or not) the calling location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param format	the logging message with placeholders (like printf).
 * @param ...		the values of the placeholders (varargs)
 * @return			true: for chaining
 */
bool ReLogger::logv(ReLoggerLevel level, int location, const QByteArray& format,
                    ...) {
   char buffer[64000];
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   return log(level, location, buffer);
}

/**
 * @brief Logs (or not) the calling location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param format	the logging message with placeholders (like printf).
 * @param varlist	variable arguments
 * @return			true: for chaining
 */
bool ReLogger::log(ReLoggerLevel level, int location, const char* format,
                   va_list& varlist) {
   char buffer[64000];
   qvsnprintf(buffer, sizeof buffer, format, varlist);
   return log(level, location, buffer);
}

/**
 * @brief Builds the standard prefix of a logging line.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 */
QByteArray ReLogger::buildStdPrefix(ReLoggerLevel level, int location) {
   time_t now = time(NULL);
   struct tm* now2 = localtime(&now);
   char buffer[64];
   qsnprintf(buffer, sizeof buffer, "%c%d.%02d.%02d %02d:%02d:%02d (%d): ",
             getPrefixOfLevel(level), now2->tm_year + 1900, now2->tm_mon + 1,
             now2->tm_mday, now2->tm_hour, now2->tm_min, now2->tm_sec, location);
   return QByteArray(buffer);
}

/**
 * @brief Adds an appender.
 *
 * @param appender		appender to add
 */
void ReLogger::addAppender(ReAppender* appender) {
   if (m_countAppenders < sizeof m_appenders / sizeof m_appenders[0]) {
      m_appenders[m_countAppenders++] = appender;
   } else {
      log(LOG_ERROR, LOC_ADD_APPENDER_1, "too many appenders");
   }
}

/**
 * @brief Returns the appender with a given name.
 *
 * @param name  the appender's name
 *
 * @return      NULL: no appender with this name is registered<br>
 *              otherwise: the wanted appender
 */
ReAppender* ReLogger::findAppender(const char* name) const {
   ReAppender* rc = NULL;
   for (size_t ix = 0; ix < m_countAppenders; ix++) {
      ReAppender* current = m_appenders[ix];
      if (strcmp(name, current->getName()) == 0) {
         rc = current;
         break;
      }
   }
   return rc;
}

/**
 * @brief Builds the standard appender configured by a configuration file.
 *
 * @param config		configuration file
 * @param prefix        the prefix of the key in the config file
 *                      (in front of "name")
 * @param defaultLogfilePrefix
 *                      the prefix of the log file if no entry in the
 *                      configuration file
 */
void ReLogger::buildStandardAppender(ReConfig* config, const char* prefix,
                                     const char* defaultLogfilePrefix) {
   QByteArray sPrefix(prefix);
   QByteArray logFilePrefix = config->asString(sPrefix + "name",
                              defaultLogfilePrefix);
   int maxSize = config->asInt(+"maxsize", 10100100);
   int maxCount = config->asInt(sPrefix + "maxfiles", 5);
   buildStandardAppender(logFilePrefix, maxSize, maxCount);
   QByteArray sLevel = config->asString(sPrefix + "level", "info");
   ReLoggerLevel level = LOG_INFO;
   if (_strcasecmp(sLevel.constData(), "error") == 0)
      level = LOG_ERROR;
   else if (_strcasecmp(sLevel, "warning") == 0)
      level = LOG_WARNING;
   else if (_strcasecmp(sLevel, "debug") == 0)
      level = LOG_DEBUG;
   setLevel(level);
}

/**
 * @brief Builds the standard appender for the instance: a console logger and a file logger.
 *
 * @param prefix		the prefix of the log file name, e.g. /var/log/server
 * @param maxSize		the maximum of the file size
 * @param maxCount		the maximal count of files. If neccessary the oldest file will be deleted
 */
void ReLogger::buildStandardAppender(const QByteArray& prefix, int maxSize,
                                     int maxCount) {
   ReStreamAppender* streamAppender = new ReStreamAppender(stderr);
   streamAppender->setAutoDelete(true);
   addAppender((ReAppender*) streamAppender);
   ReFileAppender* fileAppender = new ReFileAppender(prefix, maxSize,
         maxCount);
   fileAppender->setAutoDelete(true);
   addAppender((ReAppender*) fileAppender);
}

/** @class ReStreamAppender ReLogger.hpp "base/ReLogger.hpp"
 *
 * @brief Puts the logging info to a standard output stream.
 *
 * The possible streams are <code>std::stdout</code> or  <code>std::stderr</code>
 */

/**
 * @brief Constructor.
 */
ReStreamAppender::ReStreamAppender(FILE* file, const char* appenderName) :
   ReAppender(QByteArray(appenderName)),
   m_fp(file) {
}

/**
 * @brief Destructor.
 */
ReStreamAppender::~ReStreamAppender() {
   fflush(m_fp);
}

/**
 * @brief Logs (or not) the current location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param message	the logging message
 * @param logger    the calling logger
 */
void ReStreamAppender::log(ReLoggerLevel level, int location,
                           const char* message, ReLogger* logger) {
   const QByteArray& prefix = logger->getStdPrefix(level, location);
   fputs(prefix, m_fp);
   fputs(message, m_fp);
   fputc('\n', m_fp);
   fflush(m_fp);
}

/** @class ReFileAppender ReLogger.hpp "base/ReLogger.hpp"
 *
 * @brief Puts the logging info to a file.
 *
 * The appender creates a collection of files to limit the used disk space.
 * Each logfile is limited to a given size. And the number of files is limited.
 * If the count exceeds the oldest file will be deleted.
 *
 * Each logfile's name has a given name prefix, a running number
 * and the suffix ".log", e.g. "globallogger.003.log".
 */

/**
 * @brief Constructor.
 *
 * @param prefix		the prefix of the log file name, e.g. /var/log/server
 * @param maxSize		the maximum of the file size
 * @param maxCount		the maximal count of files. If neccessary the oldest file will be deleted
 * @param appenderName	the name of the appender. @see ReLogger::findAppender()
 */
ReFileAppender::ReFileAppender(const QByteArray& prefix, int maxSize,
                               int maxCount, const char* appenderName) :
   ReAppender(QByteArray(appenderName)),
   m_prefix(prefix),
   m_maxSize(maxSize),
   m_maxCount(maxCount),
   m_currentSize(0),
   m_currentNo(0),
   m_fp(NULL) {
   open();
}

/**
 * @brief Destructor.
 */
ReFileAppender::~ReFileAppender() {
   if (m_fp != NULL) {
      fclose(m_fp);
      m_fp = NULL;
   }
}

/**
 * @brief Opens the next log file.
 */
void ReFileAppender::open() {
   if (m_fp != NULL)
      fclose(m_fp);
   char fullName[512];
   qsnprintf(fullName, sizeof fullName, "%s.%03d.log", m_prefix.data(),
             ++m_currentNo);
   m_fp = fopen(fullName, "a");
   if (m_fp == NULL)
      fprintf(stderr, "cannot open: %s\n", fullName);
   else {
      //@ToDo
      m_currentSize = 0;
   }
}

/**
 * @brief Logs (or not) the current location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param message	the logging message
 * @param logger    the calling logger
 */
void ReFileAppender::log(ReLoggerLevel level, int location, const char* message,
                         ReLogger* logger) {
   if (m_fp != NULL) {
      const QByteArray& prefix = logger->getStdPrefix(level, location);
      fputs(prefix, m_fp);
      fputs(message, m_fp);
      fputc('\n', m_fp);
      fflush(m_fp);
   }
}

/** @class ReMemoryAppender ReLogger.hpp "base/ReLogger.hpp"
 *
 * @brief Puts the logging info to an internal buffer.
 *
 * This line list can be required: <code>getLines()</code>.
 */

/**
 * @brief Constructor.
 *
 * @param maxLines      the maximum of lines.
 *                      If the buffer is full the oldest lines will be deleted
 * @param appenderName  NULL or the name of the appender
 */
ReMemoryAppender::ReMemoryAppender(int maxLines, const char* appenderName) :
   ReAppender(appenderName),
   m_lines(),
   m_maxLines(maxLines),
   m_addPrefix(true) {
   m_lines.reserve(maxLines);
}

/**
 * @brief Destructor.
 */
ReMemoryAppender::~ReMemoryAppender() {
}

/**
 * Logs (or not) the current location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param message	the logging message
 * @param logger    the calling logger
 */
void ReMemoryAppender::log(ReLoggerLevel level, int location,
                           const char* message, ReLogger* logger) {
   if (m_lines.size() >= m_maxLines)
      m_lines.removeFirst();
   if (!m_addPrefix)
      m_lines.append(message);
   else {
      QByteArray msg(logger->getStdPrefix(level, location));
      msg += message;
      m_lines.append(msg);
   }
}

/**
 * @brief Returns the list of lines.
 *
 * @return the line list
 */
const QList<QByteArray>& ReMemoryAppender::getLines() const {
   return m_lines;
}

/**
 * @brief Deletes all log lines.
 */
void ReMemoryAppender::clear() {
   m_lines.clear();
}

/**
 * Constructor.
 *
 * @param appenderName  a name for the appender
 */
ReDebugAppender::ReDebugAppender(const char* appenderName) :
   ReAppender(appenderName) {
}
/**
 * Destructor.
 */
ReDebugAppender::~ReDebugAppender() {
}

/**
 * @brief Logs (or not) the current location.
 *
 * @param level		the level of the location
 * @param location	an unique identifier of the location
 * @param message	the logging message
 * @param logger    the calling logger
 */
void ReDebugAppender::log(ReLoggerLevel level, int location,
                          const char* message, ReLogger* logger) {
   QByteArray msg(logger->getStdPrefix(level, location));
   qDebug("%s%s", msg.constData(), message);
}

/**
 * Constructor.
 */
ReMemoryLogger::ReMemoryLogger() :
   ReLogger(true),
   ReMemoryAppender() {
   addAppender(this);
}

