/*
 * ReException.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @mainpage
 *
 * @note A real public library for QT.
 *
 * This library contains the following module groups
 * <ul>
 * <li>base: basic definitions, used in all other module groups</li>
 * <li>os: Operating System specific definitions and tools</li>
 * <li>math: mathematic definitions and tools</li>
 * <li>expr: definition for parsing and interpretation of languages</li>
 * <li>net: definitions and tools for tcp/udp communication</li>
 * <li>appl: some applications</li>
 * </ul>
 *
 * Each module group has a central include file, which organizes the necessary
 * include files. You had to include only the central include file.
 *
 * Example:
 * <pre><code>
 * #include "base/rebase.hpp"
 * #include "expr/reexpr.hpp"
 * </code></pre>
 * In this case all definitions of base and expr are available.
 */
/** @file
 * @brief Generally usable exceptions.
 */
/** @file base/ReException.hpp
 *
 * @brief Definitions for a generally usable exceptions.
 */
#include "base/rebase.hpp"

enum {
   LOC_NOT_IMPLEMENTED_1 = LOC_FIRST_OF(LOC_EXCEPTION),
};
/** @class ReException ReException.hpp "base/ReException.hpp"
 *
 * @brief A generally usable exception with or without logging.
 *
 * <b>Note</b>: If the logger is not given by parameter
 * the usage of the global logger is not threadsafe.
 */
class ReException;

/**
 * @brief Constructor.
 *
 * For derived classes only!
 */
ReException::ReException() :
   m_message("") {
}

/**
 * @brief Constructor.
 *
 * @param format    the reason of the exception
 * @param ...       the values for the placeholders in the format.
 */
ReException::ReException(const char* format, ...) :
   m_message("") {
   char buffer[64000];
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   m_message = buffer;
}

/**
 * @brief Constructor.
 *
 * This constructor automatically logs the given data.
 *
 * @param level     the logging level, e.g. LOG_ERROR
 * @param location  an unique identifier for the location
 *                  where the exception was thrown
 * @param format    the reason of the exception.
 *                  Can contain placeholders (@see
 *                  std::printf())
 * @param ...       the values of the placeholders
 *                  in <code>format</code>
 * @param logger    if NULL the global logger will be used
 */
ReException::ReException(ReLoggerLevel level, int location, ReLogger* logger,
                         const char* format, ...) :
   m_message("") {
   char buffer[64000];
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   m_message = buffer;
   if (logger == NULL)
      logger = ReLogger::globalLogger();
   logger->log(level, location, buffer);
}

/** @class RplRangeException ReException.hpp "base/ReException.hpp"
 *
 * @brief An exception for integer range errors.
 *
 * The error will be logged.
 *
 * <b>Note</b>: If the logger is not given by parameter
 * the usage of the global logger is not threadsafe.
 */

/**
 * @brief Constructor.
 *
 * This exception can be used if a value does not be
 * inside a given range.
 *
 * This constructor automatically logs the given data.
 *
 * @param level     the logging level, e.g. LOG_ERROR
 * @param location  an unique identifier for the location
 *                  where the exception was thrown
 * @param current   the current value
 * @param lbound    the minimum of the allowed values
 * @param ubound    the maximum of the allowed values
 * @param message   the reason. If NULL a generic
 *                  message will be used
 * @param logger    if NULL the global logger will be used
 */

ReRangeException::ReRangeException(ReLoggerLevel level, int location,
                                   size_t current, size_t lbound, size_t ubound, const char* message,
                                   ReLogger* logger) :
   ReException("") {
   char buffer[64000];
   if (message == NULL)
      message = "value outside limits";
   qsnprintf(buffer, sizeof buffer, "%s: %lu [%lu, %lu]",
             message == NULL ? "" : message, current, lbound, ubound);
   if (logger == NULL)
      logger = ReLogger::globalLogger();
   logger->log(level, location, buffer);
}

/** @class RplInvalidDataException ReException.hpp "base/ReException.hpp"
 *
 * @brief An exception usable if binary data have the wrong structure.
 *
 * The data will be dumped as hex and ASCII dump.
 *
 * <b>Note</b>: If the logger is not given by parameter
 * the usage of the global logger is not threadsafe.
 */

/**
 * @brief Constructor.
 *
 * This exception can be used if data does not have a given fomat.
 *
 * This constructor automatically logs the given data. This data
 * will be dumped (hexadecimal dump and ASCII interpretation).
 *
 * @param level     the logging level, e.g. LOG_ERROR
 * @param location  an unique identifier for the location
 *                  where the exception was thrown
 * @param message   the reason
 * @param data      pointer to binary data
 * @param dataSize  the size of the data which should be dumped
 * @param logger    if NULL the global logger will be used
 */
RplInvalidDataException::RplInvalidDataException(ReLoggerLevel level,
      int location, const char* message, const void* data, size_t dataSize,
      ReLogger* logger) :
   ReException("") {
   char buffer[64000];
   if (message == NULL)
      message = "invalid data: ";
   if (data == NULL)
      data = "";
   if (dataSize > 16)
      dataSize = 16;
   size_t ix;
   char* ptr = buffer + strlen(buffer);
   for (ix = 0; ix < dataSize; ix++) {
      qsnprintf(ptr, sizeof(buffer) - (ptr - buffer) - 1, "%02x ",
                ((unsigned char*) data)[ix]);
      ptr += strlen(ptr);
   }
   for (ix = 0; ix < dataSize; ix++) {
      char cc = ((char*) data)[ix];
      if (cc > ' ' && cc <= '~')
         *ptr++ = cc;
      else
         *ptr++ = '.';
   }
   if (logger == NULL)
      logger = ReLogger::globalLogger();
   logger->log(level, location, buffer);
}

/**
 * @brief Constructor.
 *
 * @param message   describes what is not implemented
 */
ReNotImplementedException::ReNotImplementedException(const char* message) :
   ReException("not implemented: ", message) {
   ReLogger::globalLogger()->log(LOG_ERROR, LOC_NOT_IMPLEMENTED_1,
                                 getMessage());
}

/**
 * Constructor.
 *
 * @param message   the description of the exception
 */
ReQException::ReQException(const QString message) :
   m_message(message) {
}

/**
 * Destructor.
 */
ReQException::~ReQException() {
}
/**
 * Returns the message.
 *
 * @return the description of the exception
 */
QString ReQException::message() const {
   return m_message;
}

