/*
 * ReTerminator.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RPLTERMINATOR_HPP
#define RPLTERMINATOR_HPP

class ReTerminator {
public:
   ReTerminator(ReLogger* logger = NULL);
   virtual ~ReTerminator();
private:
   // No copy constructor: no implementation!
   ReTerminator(const ReTerminator& source);
   // Prohibits assignment operator: no implementation!
   ReTerminator& operator =(const ReTerminator& source);
public:
   void causeTermination(const char* reason, const char* file = NULL,
                         int lineNo = 0, ReLoggerLevel level = LOG_ERROR, int location = 0);
   bool isStopped() const;
private:
   bool m_stop;
   ReLogger* m_logger;
};

#endif // RPLTERMINATOR_HPP
