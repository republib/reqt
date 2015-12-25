/*
 * reos.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef OS_REOS_HPP_
#define OS_REOS_HPP_
#ifndef REBASE_HPP
#include "base/rebase.hpp"
#endif
#if defined __linux__
#include "unistd.h"
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#elif defined __WIN32__
#include <tchar.h>
#include "windows.h"
#include <winnt.h>
#else
#error "unknown os"
#endif

#if defined __linux__
typedef struct timespec ReFileTime_t;
typedef __off_t ReFileSize_t;
#elif defined __WIN32__
typedef FILETIME ReFileTime_t;
typedef int64_t ReFileSize_t;
#endif
/** Returns whether a time is greater (younger) than another.
 * @param time1		first operand
 * @param time2		second operand
 * @return 			<code>true</code>: time1 > time2
 */
inline bool operator >(const ReFileTime_t& time1, const ReFileTime_t& time2) {
#if defined __linux__
   return time1.tv_sec > time2.tv_sec
          || (time1.tv_sec == time2.tv_sec && time1.tv_nsec > time2.tv_nsec);
#else
   return time1.dwHighDateTime > time2.dwHighDateTime
          || (time1.dwHighDateTime == time2.dwHighDateTime
              && time1.dwLowDateTime > time2.dwLowDateTime);
#endif
}
#include "os/ReFileSystem.hpp"
#include "os/ReCryptFileSystem.hpp"

#endif /* OS_REOS_HPP_ */
