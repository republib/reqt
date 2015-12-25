/*
 * ReGuiValidator.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef GUI_REVALIDATOR_HPP_
#define GUI_REVALIDATOR_HPP_

#include <QComboBox>
#include <QStatusBar>

class ReGuiValidator : public ReAnnouncer {
public:
   ReGuiValidator();
   ~ReGuiValidator();
public:
   QDateTime comboDate(QComboBox* combo);
   int comboInt(QComboBox* combo, int defaultValue, const char* specialString =
                   NULL, int specialValue = 0);
   int64_t comboSize(QComboBox* combo);
   QString comboText(QComboBox* combo);
   virtual void guiError(QWidget* widget, const QString& message);
   void setInHistory(QComboBox* combo, const QString& value);

protected:
   int m_errors;
};

#endif /* GUI_REVALIDATOR_HPP_ */
