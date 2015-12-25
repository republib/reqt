/*
 * ReTerminator.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Implements a thread stopper.
 */
/** @file
 * @brief Definitions for a thread stopper.
 */
#include "base/rebase.hpp"

enum {
   LOC_CAUSE_TERMINATION_1 = LOC_FIRST_OF(LOC_TERMINATOR), // 10901
};

/**
 * @class ReTerminator ReTerminator.hpp "base/ReTerminator.hpp"
 *
 * @brief Implements a thread stopper.
 *
 * Allows to terminate a thread avoiding unfreeing resources, deadlocks etc.
 *
 * The application must create one instance of a <code>ReTerminator</code>.
 * All threads get this instance and call them periodically if the application should stop.
 * If yes they finish their work, free the resources and stop.
 *
 */

/**
 * @brief Constructor.
 * @param logger    NULL or the logger. Used to protocol the termination
 */
ReTerminator::ReTerminator(ReLogger* logger) :
   m_stop(false),
   m_logger(logger) {
}

/**
 * @brief Destructor.
 */
ReTerminator::~ReTerminator() {
}

/**
 * @brief Defines the stop of all threads.
 *
 * @param reason    the reason of the termination. Will be logged (if a logger is defined)
 * @param file      NULL or the file of the caller. Normally set with <code>__FILE__</code>
 * @param lineNo    0 or the line number of the caller. Normally set with <code>__LINE__</code>
 * @param level     log level
 * @param location  0 or the location of the caller
 */
void ReTerminator::causeTermination(const char* reason, const char* file,
                                    int lineNo, ReLoggerLevel level, int location) {
   if (m_logger != NULL) {
      QByteArray message(reason);
      if (file != NULL) {
         message.append(" [").append(file).append(lineNo).append("]");
      }
      m_logger->log(level, location == 0 ? LOC_CAUSE_TERMINATION_1 : location,
                    message);
   }
   m_stop = true;
}

/**
 * @brief Tests whether the thread should be stopped.
 * @return  true: the thread should be stopped.<br>
 *          false: otherwise
 */
bool ReTerminator::isStopped() const {
   return m_stop;
}

