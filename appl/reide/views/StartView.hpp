/*
 * StartView.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef STARTVIEW_HPP
#define STARTVIEW_HPP
#include <QTableWidget>
#include "View.hpp"
namespace Ui {
class StartView;
}
class View;
/**
 * Form to open/create a file/project.
 *
 * A view is a widget displayed as a dock in the window displaying a perspective.
 */
class StartView: public QWidget, public View {
   Q_OBJECT

public:
   explicit StartView(MainWindow* mainWindow);
   ~StartView();
public:
   virtual QWidget* widget();

protected:
   void buildTable(const QString& filter, const QStringList& lines,
                   QTableWidget* table);
   void buildTableInfo(ReSettings* settings, const char* key, bool withDate,
                       QStringList& tableContent);
   QString fileOfTable(QTableWidget* table, int row);
public slots:
   void open();
   void selectDir();
   void selectFile();
protected slots:
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
   Ui::StartView* ui;
   QStringList m_files;
   QStringList m_projects;
};

#endif // STARTVIEW_HPP
