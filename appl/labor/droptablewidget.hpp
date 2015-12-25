/*
 * droptablewidget.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef DROPTABLEWIDGET_HPP
#define DROPTABLEWIDGET_HPP
#include <QTableWidget>
class DropTableWidget : public QTableWidget {
public:
   DropTableWidget(QWidget* widget);
protected:
#if 1
   void dragEnterEvent(QDragEnterEvent* event);
   void dragLeaveEvent(QDragLeaveEvent* event);
   void dragMoveEvent(QDragMoveEvent* event);
#endif
   void dropEvent(QDropEvent* event);
public:
   void setMainWindow(MainWindow* mainWindow);

private:
   MainWindow* m_mainWindow;
};

#endif // DROPTABLEWIDGET_HPP
