/*
 * filetablewidget.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef CUSTOMWIDGETS_HPP
#define CUSTOMWIDGETS_HPP
#include <QTableWidget>

class MainWindow;
class FileTableWidget: public QTableWidget {
   Q_OBJECT
public:
   FileTableWidget(QWidget* parent);
public:
   void setMainWindow(MainWindow* mainWindow);
protected slots:
   void mouseMoveEvent(QMouseEvent* event);
   void mousePressEvent(QMouseEvent* event);
private:
   MainWindow* m_mainWindow;
   QPoint m_dragStartPosition;
};

#endif // CUSTOMWIDGETS_HPP
