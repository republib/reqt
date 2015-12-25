/*
 * ReSettings.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "gui/regui.hpp"

enum {
   LOC_BOOL_VALUE_1 = LOC_FIRST_OF(LOC_SETTINGS), // 11701
   LOC_BOOL_VALUE_2,	// 11702
   LOC_INT_VALUE_1,	// 11703
   LOC_INT_VALUE_2,	// 11704
   LOC_STRING_VALUE_1,	// 11705
   LOC_STRING_VALUE_2,	// 11706
   LOC_WRITE_SETTINGS_1,	// 11707
   LOC_READ_SETTINGS_1,	// 11708
   LOC_READ_SETTINGS_2,	// 11709
   LOC_READ_SETTINGS_3,	// 11710
   LOC_CHANGE_VALUE_1,		// 11711
   LOC_CHANGE_VALUE_2,		// 11712
};

QString ReSettings::TRUE = "t";
QString ReSettings::FALSE = "";

/**
 * Constructor.
 *
 * @param name	name of the property
 * @param title	title in the configuration form (translated)
 * @param description description in the form (translated)
 * @param defaultValue	the value if not user defined
 * @param type	the type of the property
 * @param limits	NULL or some rules for the property value
 */
ReProperty::ReProperty(const char* name, const QString& title,
                       const QString& description, const QString& defaultValue,
                       RePropertyType type, const char* limits) :
   m_name(name),
   m_title(title),
   m_description(description),
   m_value(defaultValue),
   m_defaultValue(defaultValue),
   m_type(type),
   m_limits(limits) {
}

/**
 * Tests whether a given value is allowed respecting the <code>m_limits</code>
 * @param value	the value to test
 * @param error	OUT: NULL or the error message
 * @return <code>true</code>: the value is allowed<br>
 */
bool ReProperty::isValid(const QString& value, QString* error) {
   bool rc = true;
   if (error != NULL)
      *error = "";
   if (m_limits != NULL) {
      switch (m_type) {
      case PT_BOOL:
         break;
      case PT_INT: {
         int nValue = value.toInt(&rc);
         if (!rc) {
            if (error != NULL)
               *error = QObject::tr("not an integer: ") + value;
         } else {
            int minimum, maximum;
            if (sscanf(m_limits, "[%d,%d]", &minimum, &maximum) == 2) {
               if (nValue < minimum) {
                  rc = false;
                  if (error != NULL)
                     *error = value + " < " + minimum;
               } else if (nValue > maximum) {
                  rc = false;
                  if (error != NULL)
                     *error = value + " < " + maximum;
               }
            }
         }
         break;
      }
      case PT_STRING:
         break;
      default:
         break;
      }
   }
   return rc;
}

/**
 * Constructor.
 *
 * @param path		the parent directory for the storage files
 * @param prefix	type of the storage: "proj" or "ws" (workspace)
 * @param logger	the logger
 */
ReSettings::ReSettings(const QString& path, const QString& prefix,
                       ReLogger* logger) :
   m_prefix(prefix),
   m_path(),
   m_fileHistory(),
   m_fileSettings(),
   m_settings(),
   m_chapters(),
   m_logger(logger) {
   setPath(path);
}

/**
 * Destructor.
 */
ReSettings::~ReSettings() {
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
void ReSettings::addHistoryEntry(const char* key, const QString& value,
                                 char separator, int maxEntries) {
   ReStateStorage store(m_fileHistory, m_logger);
   store.initForRead();
   store.addHistoryEntry(key, value, separator, maxEntries);
   store.close();
   store.flushMap();
}

/*
 * Returns the first item of a history entry.
 *
 * A history entry contains a list of items.
 *
 * @param key			the key in the map
 * @param defaultValue	if the key does not exist this value is returned
 * @return				<code>defaultValue</code>: the key does not exist<br>
 *						otherwise: the first item of the history entry
 */
QString ReSettings::topOfHistory(const char* key, const QString& defaultValue) {
   ReStateStorage store(m_fileHistory, m_logger);
   store.initForRead();
   QString rc = store.map().value(key, "\t");
   if (rc == "\t")
      rc = defaultValue;
   else if (rc.isEmpty())
      rc = "";
   else {
      QChar separator = rc.at(0);
      int end = rc.indexOf(separator, 1);
      if (end < 0)
         rc = rc.mid(1);
      else
         rc = rc.mid(1, end - 1);
   }
   store.close();
   store.flushMap();
   return rc;
}

/**
 * Returns the value of a boolean property.
 *
 * @param name	the name of the property
 * @return the value of the property
 */
bool ReSettings::boolValue(const char* name) {
   bool rc = false;
   ReProperty* property = m_settings.value(name, NULL);
   if (property == NULL)
      m_logger->logv(LOG_ERROR, LOC_BOOL_VALUE_1, "missing bool property %s",
                     name);
   else if (property->m_type != PT_BOOL)
      m_logger->logv(LOG_ERROR, LOC_BOOL_VALUE_2, "not a  bool property %s",
                     name);
   else
      rc = !property->m_value.isEmpty();
   return rc;
}

/**
 * Changes the value of an existing property.
 *
 * @param name	the name of the property
 * @param value	the new value
 */
void ReSettings::changeValue(const char* name, const QString& value) {
   QString error;
   ReProperty* property = m_settings.value(name, NULL);
   if (property == NULL)
      m_logger->logv(LOG_ERROR, LOC_CHANGE_VALUE_1, "unknown property: %s",
                     name);
   else if (!property->isValid(value, &error))
      m_logger->logv(LOG_ERROR, LOC_CHANGE_VALUE_2,
                     "invalid value for %s: %s\n+++ %s", name,
                     I18N::s2b(value).constData(), I18N::s2b(error).constData());
   else
      property->m_value = value;
}

/**
 * Returns the name of the history file.
 * @return	the full name of the history file
 */
const QString& ReSettings::fileHistory() const {
   return m_fileHistory;
}

/**
 * Returns a history item as a list.
 *
 * @param key	key of the history item
 * @param list	OUT: the list is filled with the history entries
 * @param form	a common prefix of the key. If NULL the current form is used
 * @return <code>list</code> (for chaining)
 */
QStringList& ReSettings::historyAsList(const char* key, QStringList& list,
                                       const char* form) {
   ReStateStorage store(m_fileHistory, m_logger);
   store.initForRead();
   QStringList& rc = store.historyAsList(key, list, form);
   store.close();
   return rc;
}

/**
 * Inserts a property.
 *
 * @param property	the property to insert
 */
void ReSettings::insertProperty(ReProperty* property) {
   m_settings.insert(property->m_name, property);
   QByteArray chapter(property->m_name);
   int ix = chapter.lastIndexOf('.');
   chapter = chapter.left(ix);
   QList<ReProperty*>* list = m_chapters.value(chapter, NULL);
   if (list == NULL) {
      m_chapters.insert(chapter, list = new QList<ReProperty*>());
   }
   list->append(property);
}

/**
 * Returns the value of an integer property.
 *
 * @param name	the name of the property
 * @return the value of the property
 */
int ReSettings::intValue(const char* name) {
   int rc = 0;
   ReProperty* property = m_settings.value(name, NULL);
   if (property == NULL)
      m_logger->logv(LOG_ERROR, LOC_INT_VALUE_1, "missing int property %s",
                     name);
   else if (property->m_type != PT_INT)
      m_logger->logv(LOG_ERROR, LOC_INT_VALUE_2, "not a  int property %s",
                     name);
   else
      rc = property->m_value.toInt();
   return rc;
}

/**
 * Returns the directory containing the configuration data.
 *
 * @return the name of the storage's directory
 */
const QString& ReSettings::path() const {
   return m_path;
}

/**
 * Reads the configuration file.
 */
void ReSettings::readSettings() {
   QFile file(m_fileSettings);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      m_logger->logv(LOG_ERROR, LOC_READ_SETTINGS_1, "cannot open (%d): ",
                     errno, I18N::s2b(m_fileSettings).constData());
   else {
      QTextStream input(&file);
      int lineNo = 0;
      while (!input.atEnd()) {
         lineNo++;
         QString line = input.readLine();
         int ix = line.indexOf('=');
         if (ix < 0)
            m_logger->logv(LOG_ERROR, LOC_READ_SETTINGS_2,
                           "missing '=': %s-%d: %s",
                           I18N::s2b(m_fileSettings).constData(), lineNo,
                           I18N::s2b(line.mid(0, 20)).constData());
         else if (ix == 0 || (ix == 1 && line.at(0) == '!'))
            m_logger->logv(LOG_ERROR, LOC_READ_SETTINGS_3,
                           "line starts with '=': %s-%d: %s",
                           I18N::s2b(m_fileSettings).constData(), lineNo,
                           I18N::s2b(line.mid(0, 20)).constData());
         else {
            QByteArray name;
            QString value;
            if (line.at(ix - 1) == '!') {
               name = I18N::s2b(line.left(ix - 1));
               value = line.mid(ix + 1);
               value.replace("\\\\", "\01").replace("\\n", "\n").replace(
                  "\\r", "\r").replace('\01', '\\');
            } else {
               name = I18N::s2b(line.left(ix));
               value = line.mid(ix + 1);
            }
            ReProperty* property = m_settings.value(name, NULL);
            if (property != NULL) {
               switch (property->m_type) {
               case PT_BOOL:
                  property->m_value =
                     value.isEmpty() ?
                     ReSettings::FALSE : ReSettings::TRUE;
                  break;
               case PT_INT:
                  if (property->isValid(value))
                     property->m_value = value;
                  break;
               case PT_STRING:
                  if (property->isValid(value))
                     property->m_value = value;
                  break;
               default:
                  break;
               }
            }
         }
      }
      file.close();
   }
}

/**
 * Sets the path containing the settings files.
 *
 * @param path	the new path
 */
void ReSettings::setPath(const QString& path) {
   m_path = path;
   m_fileHistory = path + OS_SEPARATOR + m_prefix + ".history";
   m_fileSettings = path + OS_SEPARATOR + m_prefix + ".settings";
}

/**
 * Returns the value of a string property.
 *
 * @param name	the name of the property
 * @return the value of the property
 */
QString ReSettings::stringValue(const char* name) {
   QString rc = "";
   ReProperty* property = m_settings.value(name, NULL);
   if (property == NULL)
      m_logger->logv(LOG_ERROR, LOC_STRING_VALUE_1,
                     "missing string property %s", name);
   else if (property->m_type != PT_STRING)
      m_logger->logv(LOG_ERROR, LOC_STRING_VALUE_2,
                     "not a string property %s", name);
   else
      rc = property->m_value;
   return rc;
}

/**
 * Writes the values into the configuration file.
 */
void ReSettings::writeSettings() {
   QFile file(m_fileSettings);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      m_logger->logv(LOG_ERROR, LOC_WRITE_SETTINGS_1, "cannot open (%d): ",
                     errno, I18N::s2b(m_fileSettings).constData());
   else {
      QTextStream out(&file);
      QMap<QByteArray, ReProperty*>::const_iterator it;
      for (it = m_settings.cbegin(); it != m_settings.cend(); ++it) {
         ReProperty* property = it.value();
         if (property->m_value == property->m_defaultValue) {
            // we do not write the default value
         } else if (it.value()->m_value.indexOf('\n') <= 0)
            out << it.key() << "=" << it.value()->m_value << "\n";
         else {
            QString value = it.value()->m_value;
            out << it.key() << "!="
                << value.replace("\\", "\\\\").replace("\n", "\\n").replace(
                   "\r", "\\r") << "\n";
         }
      }
      file.close();
   }
}

/**
 * Constructor.
 *
 * @param file		the file to store the settings
 * @param logger	the logger
 * @param delay		time between first change of setting and storage in seconds
 */
ReDelayedStorage::ReDelayedStorage(const QString& file, ReLogger* logger,
                                   int delay) :
   m_timer(NULL),
   m_list(),
   m_delay(delay),
   m_file(file),
   m_logger(logger) {
   m_timer.setSingleShot(true);
   connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

/**
 * Finds the index of the widget in the list.
 *
 * @param widget	widget to search
 * @return			-1: not found<br>
 *					otherwise: the index in <code>m_list</code>
 */
int ReDelayedStorage::indexOf(const QWidget* widget) const {
   int rc = -1;
   for (int ix = 0; rc < 0 && ix < m_list.size(); ix++)
      if (m_list.at(ix)->m_widget == widget)
         rc = ix;
   return rc;
}

/**
 * Initializes the storage of the window geometry.
 *
 * @param window	the window to store
 */
void ReDelayedStorage::storeWindow(const QMainWindow* window) {
   int ix = indexOf(window);
   if (ix < 0) {
      ObjInfo* info = new ObjInfo;
      info->m_type = OT_WINDOW;
      info->m_widget = window;
      m_list.append(info);
      if (! m_timer.isActive()) {
         m_timer.start(m_delay*1000);
      }
   }
}

/**
 * Handles the event timeout of the timer.
 */
void ReDelayedStorage::timeout() {
   ReStateStorage storage(m_file, m_logger);
   storage.initForRead();
   for (int ix = 0; ix < m_list.size(); ix++) {
      ObjInfo* info = m_list.at(ix);
      switch(info->m_type) {
      case OT_WINDOW: {
         const QMainWindow* window = reinterpret_cast<const QMainWindow*>
                                     (info->m_widget);
         storage.store(window);
         break;
      }
      default:
         break;
      }
   }
   storage.flushMap();
}

