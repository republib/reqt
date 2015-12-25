/*
 * rebase.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef REBASE_HPP
#define REBASE_HPP

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#else
#include <io.h>
#include <direct.h>
#endif
#include <QThread>
#include <QIODevice>
#include <QTextStream>
#include <QMap>
#include <QVector>
#include <QDataStream>
#include <QMutex>
#include <QRegularExpression>
#include <QDateTime>
#include <QtCore/qmath.h>
#include <QTranslator>
#include <QDateTime>
#include <QDir>
#include <QTimer>
#include <QWidget>
#include <QMainWindow>
#include <QComboBox>
#include <QComboBox>
#include <QProcess>
#include <QMutex>

typedef unsigned char uint8_t;
#if !defined __linux__
typedef qint64 int64_t;
#endif
typedef quint64 uint64_t;
typedef qint32 int32_t;
typedef quint32 uint32_t;
typedef qreal real_t;
typedef QString ReString;
#define RE_UNUSED(x) (void)(x)

#ifdef __linux__
#define _strcasecmp strcasecmp
#define OS_SEPARATOR '/'
#define OS_SEPARATOR_STR "/"
#define OS_2nd_SEPARATOR '\\'
#define OS_2nd_SEPARATOR_STR "\\"
#define _memicmp memicmp
#define _mkdir(path) mkdir(path, -1)
#define _rmdir rmdir
#define _unlink unlink
#else
#define _strcasecmp _stricmp
#define OS_SEPARATOR '\\'
#define OS_SEPARATOR_STR "\\"
#define OS_2nd_SEPARATOR '/'
#define OS_2nd_SEPARATOR_STR "/"
#endif

typedef union {
public:
   inline void fromBytes(const uint8_t* source) {
#if ! defined __BIG_ENDIAN__
      m_int = *(int64_t*) source;
#else
      m_bytes[0] = source[7];
      m_bytes[1] = source[6];
      m_bytes[2] = source[5];
      m_bytes[3] = source[4];
      m_bytes[4] = source[3];
      m_bytes[5] = source[2];
      m_bytes[6] = source[1];
      m_bytes[7] = source[0];
#endif
   }
   inline void toBytes(uint8_t* target) {
#if ! defined __BIG_ENDIAN__
      *(int64_t*) target = m_int;
#else
      target[0] = m_bytes[7];
      target[1] = m_bytes[6];
      target[2] = m_bytes[5];
      target[3] = m_bytes[4];
      target[4] = m_bytes[3];
      target[5] = m_bytes[2];
      target[6] = m_bytes[1];
      target[7] = m_bytes[0];
#endif
   }
public:
   int64_t m_int;
   uint8_t m_bytes[sizeof(int64_t)];
} int64_converter_t;


#define UNUSED_VAR(var) (void) var
inline
int max(int a, int b) {
   return a > b ? a : b;
}
inline
int min(int a, int b) {
   return a < b ? a : b;
}
inline double max(double a, double b) {
   return a > b ? a : b;
}
inline double min(double a, double b) {
   return a < b ? a : b;
}

/** Returns the integer value of a hex digit.
 * @param hex			the hex digit
 * @param defaultValue	the value if hex is out of range
 * @return				<code>defaultValue</code>: <code>hex</code> is not a hex digit<br>
 *						otherwise: the value of the hex digit,
 */
inline int hexToInt(char hex, int defaultValue = -1) {
   return hex > 'f' ? defaultValue
          : hex > 'a' ? 10 + hex - 'a'
          : hex > 'F' ? defaultValue
          : hex >= 'A' ? 10 + hex - 'A'
          : hex > '9' ? defaultValue
          : hex >= '0' ? hex - '0' : defaultValue;
}
/**
 * Rounds a double value to an integer.
 *
 * @param value the value to round
 */
inline int roundInt(double value) {
   return (int) value < 0.0 ? ceil(value - 0.5) : floor(value + 0.5);
}
/** An observer can be informed about state changes.
 * Pure abstract class.
 */
class ReObserver {
public:
   enum ReturnCode { SUCCESS, STOP, EXIT };
   /** Informs the observer about a state change.
    * @param	message	NULL or an info about the state change
    * @return	SUCCESS: success<br>
    *			STOP: the current action should be stopped<br>
    *			EXIT: the process should be stopped
    */
   virtual ReturnCode notify(const char* message) = 0;
};

#define ReUseParameter(var) (void) var

#include "remodules.hpp"
#include "base/ReProcess.hpp"
#include "base/ReByteStorage.hpp"
#include "base/ReCharPtrMap.hpp"
#include "base/ReWriter.hpp"
#include "base/ReLogger.hpp"
#include "base/ReException.hpp"
#include "base/ReContainer.hpp"
#include "base/ReStringUtils.hpp"
#include "base/ReQStringUtils.hpp"
#include "base/ReProgramArgs.hpp"
#include "base/ReConfigurator.hpp"
#include "base/ReConfig.hpp"
#include "base/ReTerminator.hpp"
#include "base/ReLineSource.hpp"
#include "base/ReFileUtils.hpp"
#include "base/ReFile.hpp"
#include "base/ReDiff.hpp"
#include "base/ReMatcher.hpp"
#include "base/ReTest.hpp"
#include "base/ReRandomizer.hpp"
#endif // REBASE_HPP
