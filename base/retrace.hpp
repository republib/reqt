/*
 * retrace.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef RETRACE_HPP
#define RETRACE_HPP

#ifdef WITH_TRACE
#define TRACE(format) printf(format);
#define TRACE1(format, a1) printf(format, a1)
#define TRACE2(format, a1, a2) printf(format, a1, a2)
#define TRACE_IT(args) printf args
#define IF_TRACE(statem) statem
#else
#define TRACE(m)
#define TRACE1(format, a1)
#define TRACE2(format, a1, a2)
#define TRACE_IT(args)
#define IF_TRACE(statem)
#endif

#ifdef WITH_TRACE
static QByteArray hexBytes(const void* arg, int length = 8) {
   char buffer[16+1];
   const unsigned char* src = reinterpret_cast<const unsigned char*>(arg);
   QByteArray rc;
   rc.reserve(length * 2);
   for (int ii = 0; ii < length; ii++) {
      snprintf(buffer, sizeof buffer, "%02x", src[ii]);
      rc += buffer;
   }
   return rc;
}
#endif // WITH_TRACE
#endif // RETRACE_HPP

