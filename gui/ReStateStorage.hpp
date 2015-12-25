/*
 * ReStateStorage.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef GUI_RESTATESTORAGE_HPP_
#define GUI_RESTATESTORAGE_HPP_

#include <QComboBox>
#include <QTextStream>
#include <QMainWindow>

class ReStateStorage {
public:
   ReStateStorage(const QString& filename, ReLogger* logger);
   virtual ~ReStateStorage();
public:
   void addHistoryEntry(const char* key, const QString& value, char separator,
                        int maxEntries, const char* form = NULL);
   void close();
   void flushMap();
   const QByteArray& form() const;
   QByteArray fullname(const char* name);
   QStringList& historyAsList(const char* key, QStringList& list,
                              const char* form = NULL);
   bool initForRead();
   bool initForWrite();
   /** Returns the map containing the storage content.
    * @return	the map
    */
   QMap<QByteArray, QString>& map() {
      return m_map;
   }
   void restore(QComboBox* combo, const char* name, bool withCurrentText =
                   false);
   QString restore(const char* name, int index = -1);
   void restore(QMainWindow* window);
   void setForm(const char* form);
   void store(const QComboBox* combo, const char* name, bool withCurrentText =
                 true);
   void store(const char* name, const QString& value, int index = -1);
   void store(const QMainWindow* window);
private:
   QString m_filename;
   FILE* m_fp;
   QTextStream* m_stream;
   QByteArray m_form;
   QMap<QByteArray, QString> m_map;
   ReLogger* m_logger;
};

#endif /* GUI_RESTATESTORAGE_HPP_ */
