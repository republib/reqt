/*
 * ReException.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef REEXCEPTION_HPP
#define REEXCEPTION_HPP

// the sources generated from QT include this file directly:
#ifndef RPLCORE_HPP
#include <QByteArray>
#endif

class ReException {
protected:
   ReException();
public:
   ReException(const char* message, ...);
   ReException(ReLoggerLevel level, int location, const char* message,
               ReLogger* logger = NULL);
   ReException(ReLoggerLevel level, int location, ReLogger* logger,
               const char* message, ...);
   const QByteArray& getMessage() const {
      return m_message;
   }
protected:
   QByteArray m_message;
};

class ReRangeException: public ReException {
public:
   ReRangeException(ReLoggerLevel level, int location, size_t current,
                    size_t lbound, size_t ubound, const char* message = NULL,
                    ReLogger* logger = NULL);
};

class RplInvalidDataException: public ReException {
public:
   RplInvalidDataException(ReLoggerLevel level, int location,
                           const char* message, const void* data = NULL, size_t dataSize = 0,
                           ReLogger* logger = NULL);
};

class ReNotImplementedException: public ReException {
public:
   ReNotImplementedException(const char* message);
};

class ReQException {
public:
   ReQException(const QString message);
   ~ReQException();
public:
   QString message() const;

protected:
   QString m_message;
};

#endif // REEXCEPTION_HPP
