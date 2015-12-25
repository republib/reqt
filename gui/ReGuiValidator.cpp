/*
 * ReGuiValidator.cpp
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

/**
 * Constructor.
 */
ReGuiValidator::ReGuiValidator() :
   m_errors(0) {
}

/**
 * Destructor.
 */
ReGuiValidator::~ReGuiValidator() {
}

/**
 * Returns the date given as formula in a combobox.
 *
 * @param combo the combobox with the date/time formula
 *
 * @return the date resulting from the formula or begin of the epoch (error case)
 */
QDateTime ReGuiValidator::comboDate(QComboBox* combo) {
   QDateTime rc;
   QString value = combo->currentText();
   if (value.isEmpty())
      rc.setMSecsSinceEpoch(0);
   else {
      ReDateTimeParser parser(value);
      if (parser.isValid()) {
         rc = parser.asDateTime();
         setInHistory(combo, value);
         combo->setCurrentText(rc.toString("yyyy.MM.dd/hh:mm"));
      } else {
         guiError(combo, parser.errorMessage());
         rc.setMSecsSinceEpoch(0);
      }
   }
   return rc;
}
/**
 * Returns an integer given in a combobox.
 *
 * @param combo         the combobox with the integer
 * @param defaultValue  the value if the combobox is empty or invalid
 * @param specialString NULL or a (non integer) string which is allowed
 * @param specialValue  the value which is returned if specialString is found
 * @return              <code>defaultValue</code>: empty or invalid input<br>
 *                      otherwise: or the size resulting from the formula
 */
int ReGuiValidator::comboInt(QComboBox* combo, int defaultValue,
                             const char* specialString, int specialValue) {
   QString value = combo->currentText();
   int rc = defaultValue;
   if (!value.isEmpty()) {
      if (specialString != NULL && value == specialString) {
         rc = specialValue;
      } else {
         uint nValue = 0;
         if (ReQStringUtils::lengthOfUInt(value, 0, 10, &nValue) == 0)
            guiError(combo, QObject::tr("not an integer: ") + value);
         else {
            setInHistory(combo, value);
            rc = (int) nValue;
         }
      }
   }
   return rc;
}

/**
 * Returns the size (in bytes) given as formula in a combobox.
 *
 * @param combo the combobox with the size formula
 *
 * @return      -1: empty or invalid input<br>
 *              otherwise: or the size resulting from the formula
 */
int64_t ReGuiValidator::comboSize(QComboBox* combo) {
   QString value = combo->currentText();
   int64_t rc = -1;
   if (!value.isEmpty()) {
      ReSizeParser parser(value);
      rc = parser.asInt64(-1);
      if (rc >= 0) {
         setInHistory(combo, value);
         combo->setCurrentText(QString("").sprintf("%ld", rc));
      } else
         guiError(combo, parser.errorMessage());
   }
   return rc;
}

/**
 * Returns the current text from a combobox and puts it into the history.
 *
 * @param combo combobox to handle
 *
 * @return      the current text
 */
QString ReGuiValidator::comboText(QComboBox* combo) {
   QString rc = combo->currentText();
   setInHistory(combo, rc);
   return rc;
}

/**
 * Handles an error found in an interactive widget.
 *
 * @param widget    the widget where the error was found
 * @param message   the error message
 */
void ReGuiValidator::guiError(QWidget* widget, const QString& message) {
   if (widget != NULL)
      widget->setFocus(Qt::OtherFocusReason);
   say(LOG_ERROR, message);
   m_errors++;
}

/**
 * @brief Sets a text in a combobox uses as history.
 *
 * Sets the text as the first entry. If the text can be found in other entries
 * of the combobox it will be deleted there.
 *
 * @param combo     the combobox to change
 * @param value     the text to set
 */
void ReGuiValidator::setInHistory(QComboBox* combo, const QString& value) {
   if (value.isEmpty()) {
      // nothing to do
   } else if (combo->count() == 0)
      combo->addItem(value);
   else {
      if (value != combo->itemText(0)) {
         combo->insertItem(0, value);
      }
      for (int ii = 1; ii < combo->count(); ii++) {
         if (value == combo->itemText(ii)) {
            combo->removeItem(ii);
         }
      }
      if (combo->count() > 20)
         combo->removeItem(20);
      if (combo->currentText() != value)
         combo->setCurrentText(value);
   }
}

