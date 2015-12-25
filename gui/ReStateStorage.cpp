/*
 * ReStateStorage.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "QIODevice"
#include "base/rebase.hpp"
#include "gui/regui.hpp"
#include <QDesktopWidget>
enum {
   LOC_INIT_FOR_WRITE_1 = LOC_FIRST_OF(LOC_STATESTORAGE), // 12001
   LOC_INIT_FOR_READ_1,	// 12002
   LOC_RESTORE_WINDOW_1,	// 12003
};
/**
 * Constructor.
 *
 * @param filename  filename with path of the storage file
 * @param logger	the logger
 */
ReStateStorage::ReStateStorage(const QString& filename, ReLogger* logger) :
   m_filename(filename),
   m_fp(NULL),
   m_stream(NULL),
   m_form(),
   m_logger(logger) {
}

/**
 * Destructor.
 */
ReStateStorage::~ReStateStorage() {
   close();
}

/*
 * Adds an entry to a history item at the first position.
 *
 * The entry will removed from the other positions.
 *
 * @param key		the key in the map
 * @param value		the value to add
 * @param separator	separates the entries in the history item
 * @param maxEntries	the maximal count of entries in the history item.<br>
 * If the number exceeds the last entries will be removed
 * @param form	the prefix of the key. If NULL the current form will be taken
 */
void ReStateStorage::addHistoryEntry(const char* key, const QString& value,
                                     char separator, int maxEntries, const char* form) {
   if (form != NULL)
      setForm(form);
   QByteArray key2;
   if (!m_form.isEmpty())
      key2 = m_form + ".";
   key2.append(key);
   QString values;
   QStringList listValues;
   // Note: the first entry remains empty:
   // the first char of the join is a separator
   if (!m_map.contains(key2))
      listValues.append("");
   else {
      values = m_map[key2];
      listValues = values.split(values[0]);
   }
   listValues.insert(1, value);
   for (int ix = listValues.size() - 1; ix > 1; ix--) {
      if (ix > maxEntries || listValues[ix] == value) {
         listValues.removeAt(ix);
      }
   }
   values = listValues.join(separator);
   m_map.insert(key2, values);
}
/**
 * Returns a history item as a list.
 *
 * @param key	key of the history item
 * @param list	OUT: the list is filled with the history entries
 * @param form	a common prefix of the key. If NULL the current form is used
 * @return <code>list</code> (for chaining)
 */
QStringList& ReStateStorage::historyAsList(const char* key, QStringList& list,
      const char* form) {
   list.clear();
   if (form != NULL)
      setForm(form);
   QString history = restore(key);
   if (!history.isEmpty()) {
      QChar separator = history[0];
      history = history.mid(1);
      if (!history.isEmpty())
         list = history.split(separator);
   }
   return list;
}

/**
 * Closes open stream/file and frees the resources.
 */
void ReStateStorage::close() {
   delete m_stream;
   m_stream = NULL;
   if (m_fp != NULL) {
      fclose(m_fp);
      m_fp = NULL;
   }
}

/**
 * Writes the content of the map into the file.
 */
void ReStateStorage::flushMap() {
   if (initForWrite()) {
      QMap<QByteArray, QString>::const_iterator it;
      for (it = m_map.constBegin(); it != m_map.constEnd(); ++it) {
         *m_stream << it.key() << "=" << it.value() << endl;
      }
      m_stream->flush();
   }
}
/**
 * Returns the name of the current form.
 *
 * @return  the name of the current form
 */
const QByteArray& ReStateStorage::form() const {
   return m_form;
}

/**
 * Returns the full name of a widget.
 *
 * @param name  the name of the widget
 * @return      &lt;form name&gt; '.' &lt;name&gt; or &lt;name&gt; (if no form name is set)
 */
QByteArray ReStateStorage::fullname(const char* name) {
   QByteArray rc;
   if (!m_form.isEmpty())
      rc.append(m_form).append(".");
   rc.append(name);
   return rc;
}

/**
 * Initializes the instance for writing the storage information.
 */
bool ReStateStorage::initForRead() {
   if (m_fp == NULL)
      if ((m_fp = fopen(I18N::s2b(m_filename).constData(), "rb")) == NULL) {
         if (m_logger != NULL)
            m_logger->logv(LOG_ERROR, LOC_INIT_FOR_READ_1,
                           "cannot open %s: %d", I18N::s2b(m_filename).constData(),
                           errno);
      }
   if (m_fp != NULL && m_stream == NULL) {
      m_stream = new QTextStream(m_fp, QIODevice::ReadOnly);
      QString line;
      m_map.clear();
      QString value;
      while (!m_stream->atEnd()) {
         line = m_stream->readLine(64 * 1024);
         int ixAssignment = line.indexOf('=');
         if (ixAssignment > 0) {
            value = line.mid(ixAssignment + 1);
            QByteArray key = I18N::s2b(line.left(ixAssignment));
            m_map.insert(key, value);
         }
      }
   }
   return m_stream != NULL;
}
/**
 * Initializes the instance for writing the storage information.
 */
bool ReStateStorage::initForWrite() {
   if (m_fp == NULL)
      m_fp = fopen(I18N::s2b(m_filename).constData(), "wb");
   if (m_fp == NULL) {
      if (m_logger != NULL)
         m_logger->logv(LOG_ERROR, LOC_INIT_FOR_WRITE_1,
                        "cannot open %s: %d", I18N::s2b(m_filename).constData(), errno);
   } else
      m_stream = new QTextStream(m_fp, QIODevice::ReadWrite);
   return m_stream != NULL;
}
/**
 * Restores the data of a combobox.
 *
 * @param combo             the combobox to restore
 * @param name              the name of the combobox
 * @param withCurrentText   <code>true</code>: the current text will be set too
 */
void ReStateStorage::restore(QComboBox* combo, const char* name,
                             bool withCurrentText) {
   if (initForRead()) {
      QByteArray keyPrefix = fullname(name) + ".item";
      int ix = 0;
      QByteArray key;
      while (true) {
         key = keyPrefix + QByteArray::number(ix);
         if (!m_map.contains(key))
            break;
         combo->addItem(m_map.value(key));
         ix++;
      }
      key = fullname(name) + ".text";
      if (!withCurrentText)
         combo->setCurrentText("");
      else {
         if (m_map.contains(key))
            combo->setCurrentText(m_map.value(key));
      }
   }
}
/**
 * Reads a value from the storage.
 *
 * @param name  name of the variable
 * @param index -1 (no array) or the index of the entry
 * @return      "": not found<br>
 *              otherwise: the stored value of the variable
 */
QString ReStateStorage::restore(const char* name, int index) {
   QString rc;
   if (initForRead()) {
      QByteArray key(fullname(name));
      if (index >= 0)
         key += QString::number(index);
      if (m_map.contains(key))
         rc = m_map.value(key);
   }
   return rc;
}

/**
 * Reads the window geometry.
 *
 * Adaptions will be taken if the stored settings exceeds
 * the current screen.
 *
 * @param window	OUT: the geometry will be taken from the settings
 */
void ReStateStorage::restore(QMainWindow* window) {
   if (initForRead()) {
      QByteArray key("window");
      if (m_map.contains(key)) {
         QString line = m_map.value(key);
         int x, y, width, height;
         x = y = width = height = 0;
         QTextStream stream(&line);
         stream >> x >> y >> width >> height;
         QDesktopWidget desktop;
         if (x > desktop.screen()->width() - 100)
            x = 50;
         if (y > desktop.screen()->height() - 100)
            y = 50;
         if (width > desktop.screen()->width()) {
            width = desktop.screen()->width() - 50;
            x = 50;
         }
         if (height > desktop.screen()->height()) {
            height = desktop.screen()->height() - 50;
            x = 50;
         }
         if (x + width > desktop.screen()->width())
            x = desktop.screen()->width() - width;
         if (y + height > desktop.screen()->height()) {
            y = desktop.screen()->height() - height;
         }
         window->move(x, y);
         window->resize(width, height);
      }
   }
}

/**
 * Sets the name of the current form.
 *
 * @param form  the name of the current form
 */
void ReStateStorage::setForm(const char* form) {
   m_form = form;
}

/**
 * Stores the data of a combobox.
 *
 * @param combo             the combobox to store
 * @param name              the name of the combobox
 * @param withCurrentText   <code>true</code>: the current text will be saved too
 */
void ReStateStorage::store(const QComboBox* combo, const char* name,
                           bool withCurrentText) {
   if (initForWrite()) {
      QByteArray key(fullname(name));
      for (int ii = 0; ii < combo->count(); ii++) {
         *m_stream << key << ".item" << ii << "=" << combo->itemText(ii)
                   << endl;
      }
      if (withCurrentText) {
         *m_stream << key << ".text=" << combo->currentText() << endl;
      }
      m_stream->flush();
   }
}

/**
 * Stores the data of a combobox.
 *
 * @param name	the key of the key-value-pair. May be extended by <code>m_form</code>
 * @param value	the value to store
 * @param index	-1 or the index in the array
 */
void ReStateStorage::store(const char* name, const QString& value, int index) {
   if (initForWrite()) {
      QByteArray key(fullname(name));
      if (index >= 0)
         key += QString::number(index);
      *m_stream << key << "=" << value << endl;
      m_stream->flush();
   }
}

/**
 * Stores geometry of a main window.
 *
 * @param window             the combobox to store
 */
void ReStateStorage::store(const QMainWindow* window) {
   if (initForWrite()) {
      *m_stream << "window=" << window->x() << ' ' << window->y()
                << ' ' << window->width() << ' ' << window->height() << endl;
      m_stream->flush();
   }
}
