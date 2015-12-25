/*
 * ReSettings.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef STORAGE_HPP
#define STORAGE_HPP
#ifndef REBASE_HPP
#include "base/rebase.hpp"
#endif

enum RePropertyType {
   PT_UNDEF,
   PT_INT,
   PT_STRING,
   PT_BOOL
};

class ReProperty {
public:
   ReProperty(const char* name, const QString& title,
              const QString& description, const QString& defaultValue,
              RePropertyType type, const char* limits = NULL);
public:
   bool isValid(const QString& value, QString* error = NULL);
public:
   const char* m_name;
   QString m_title;
   QString m_description;
   QString m_value;
   QString m_defaultValue;
   RePropertyType m_type;
   const char* m_limits;
};
class ReLogger;
class QMainWindow;
class ReDelayedStorage : protected QObject {
   enum ObjType {
      OT_UNDEF,
      OT_WINDOW
   };
   typedef struct {
      ObjType m_type;
      const QWidget* m_widget;
   } ObjInfo;
public:
   ReDelayedStorage(const QString& file, ReLogger* m_logger, int delay = 5);
   virtual ~ReDelayedStorage() {}
public:
   void storeWindow(const QMainWindow* window);
public:
   void timeout();
private:
   int indexOf(const QWidget* widget) const;
private:
   QTimer m_timer;
   QList<ObjInfo*> m_list;
   int m_delay;
   QString m_file;
   ReLogger* m_logger;
};

typedef QMap<QByteArray, ReProperty*> RePropertyMap;
typedef QMap<QByteArray, QList<ReProperty*>*> ReChapterMap;

class ReSettings {
public:
   static QString TRUE;
   static QString FALSE;
public:
   ReSettings(const QString& path, const QString& prefix, ReLogger* logger);
   ~ReSettings();
public:
   void addHistoryEntry(const char* key, const QString& value, char separator,
                        int maxEntries);
   bool boolValue(const char* name);
   void changeValue(const char* name, const QString& value);
   const QString& fileHistory() const;
   QStringList& historyAsList(const char* key, QStringList& list,
                              const char* form = NULL);
   void insertProperty(ReProperty* property);
   int intValue(const char* name);
   const QString& path() const;
   void readSettings();
   void setPath(const QString& path);
   QString stringValue(const char* name);
   QString topOfHistory(const char* key, const QString& defaultValue = "");
   void writeSettings();


protected:
   QString m_prefix;
   QString m_path;
   QString m_fileHistory;
   QString m_fileSettings;
   RePropertyMap m_settings;
   ReChapterMap m_chapters;
   ReLogger* m_logger;
};

#endif // STORAGE_HPP
