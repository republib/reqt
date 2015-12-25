/*
 * projectselection.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef PROJECTSELECTION_HPP
#define PROJECTSELECTION_HPP
#include <QDialog>
#include <QTableWidget>
#if ! defined REBASE_HPP
#include "base/rebase.hpp"
#endif
#if ! defined REGUI_HPP
#include "gui/regui.hpp"
#endif
namespace Ui {
class ProjectSelection;
}
class MainWindow;
class ProjectSelection: public QDialog {
   Q_OBJECT

public:
   explicit ProjectSelection(MainWindow* mainWindow, QWidget* parent = 0);
   ~ProjectSelection();

public slots:
   void open();
   void selectDir();
   void selectFile();
protected:
   void buildTable(const QString& filter, const QStringList& lines,
                   QTableWidget* table);
   void buildTableInfo(ReSettings* settings, const char* key, bool withDate,
                       QStringList& tableContent);
   QString fileOfTable(QTableWidget* table, int row);
   void textChanged(const QString& text, QTableWidget* table,
                    const QStringList& lines);
protected slots:
   void cellEnteredFiles(int row, int col);
   void cellEnteredProjects(int row, int col);
   void textChangedFilterFiles(const QString& text);
   void textChangedFilterProjects(const QString& text);
private:
   void error(const QString& message);
private:
   Ui::ProjectSelection* ui;
   MainWindow* m_mainWindow;
   QStringList m_files;
   QStringList m_projects;
};

#endif // PROJECTSELECTION_HPP
