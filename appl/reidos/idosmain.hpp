/*
 * idosmain.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef IDOSMAIN_HPP
#define IDOSMAIN_HPP

#include <QMainWindow>

namespace Ui {
class IDosMain;
}
#include "reidos.hpp"
class IDosMain : public QMainWindow, public ReGuiValidator, public FileCommander {
   Q_OBJECT

public:
   explicit IDosMain(const QString& startDir, const QString& homeDir,
                     QWidget* parent = 0);
   ~IDosMain();

public:
   void error(const QString& message);
   virtual bool say(ReLoggerLevel level, const QString& message);
protected:
   void initializeHome();
   void restoreState();
   void saveState();
protected slots:

private:
   Ui::IDosMain* ui;
   QLabel* m_statusMessage;
   QString m_homeDir;
   QString m_storageFile;
};

#endif // IDOSMAIN_HPP
