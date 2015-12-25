/*
 * ReConfigurator.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RECONFIGURATOR_HPP
#define RECONFIGURATOR_HPP

class ReConfigurator {
public:
   virtual int asInt(const char* key, int defaultValue) const = 0;
   virtual bool asBool(const char* key, bool defaultValue) const = 0;
   virtual QByteArray asString(const char* key, const char* defaultValue) = 0;
};

#endif // RECONFIGURATOR_HPP
