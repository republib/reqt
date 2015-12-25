/*
 * dialogfileplaceholder.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef DIALOGLINEPLACEHOLDERS_HPP
#define DIALOGLINEPLACEHOLDERS_HPP

#include <QDialog>

namespace Ui {
class DialogFilePlaceHolders;
}

class DialogFilePlaceholder: public QDialog {
   Q_OBJECT

public:
   enum {
      COL_VAR, COL_EXAMPLE, COL_DESCR
   };
public:
   explicit DialogFilePlaceholder(QWidget* parent = 0);
   ~DialogFilePlaceholder();
public:
   QString var() const;
public slots:
   void ok();

private:
   Ui::DialogFilePlaceHolders* ui;
   QString m_var;
};

#endif // DIALOGLINEPLACEHOLDERS_HPP
