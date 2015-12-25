/*
 * ReConfig.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RECONFIG_HPP
#define RECONFIG_HPP

class ReConfig: public ReConfigurator, public QHash<QByteArray, QByteArray> {
public:
   ReConfig(const char* file = NULL, bool readOnly = true, ReLogger* logger =
               NULL);
   virtual ~ReConfig();

public:
   bool read(const char* file);
   bool write(const char* file);
   void clear();
   const QList<QByteArray>& getLines() const;

   virtual bool asBool(const char* key, bool defaultValue) const;
   virtual int asInt(const char* key, int defaultValue) const;
   virtual QByteArray asString(const char* key, const char* defaultValue);
private:
   void initLogger();
private:
   const char* m_file;
   QList<QByteArray> m_lineList;
   bool m_readOnly;
   ReLogger* m_logger;
   // true: the logger must be destroyed in the destructor
   bool m_ownLogger;
};

#endif // RECONFIG_HPP
