/*
 * dialogglobalplaceholder.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef DIALOGGLOBALPLACEHOLDER_HPP
#define DIALOGGLOBALPLACEHOLDER_HPP

#include <QDialog>

namespace Ui {
class DialogGlobalPlaceholder;
}

class DialogGlobalPlaceholder: public QDialog {
   Q_OBJECT
public:
   enum {
      COL_VAR, COL_DESCR
   };

public:
   explicit DialogGlobalPlaceholder(QWidget* parent = 0);
   ~DialogGlobalPlaceholder();
public:
   QString var() const;

public slots:
   void ok();
private:
   Ui::DialogGlobalPlaceholder* ui;
   QString m_var;
};

#endif // DIALOGGLOBALPLACEHOLDER_HPP
