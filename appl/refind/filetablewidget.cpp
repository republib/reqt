/*
 * filetablewidget.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include <QMouseEvent>
#include <QApplication>
#include "mainwindow.hpp"
#include "filetablewidget.hpp"

/**
 * Constructor.
 */
FileTableWidget::FileTableWidget(QWidget* parent) :
   QTableWidget(parent), m_mainWindow(NULL), m_dragStartPosition(0, 0) {
}

/**
 * Reimplements the mouse press event (for dragging).
 *
 * @param event the event data
 */
void FileTableWidget::mousePressEvent(QMouseEvent* event) {
   if (event->button() == Qt::LeftButton)
      m_dragStartPosition = event->pos();
}

void FileTableWidget::setMainWindow(MainWindow* mainWindow) {
   m_mainWindow = mainWindow;
}

/**
 * Reimplements the mouse move event (for dragging).
 *
 * @param event the event data
 */
void FileTableWidget::mouseMoveEvent(QMouseEvent* event) {
   Qt::MouseButton button = event->button();
   if (button == Qt::LeftButton || button == Qt::NoButton) {
      int distance = (event->pos() - m_dragStartPosition).manhattanLength();
      if (distance < QApplication::startDragDistance())
         m_mainWindow->fileDragging();
   }
}
