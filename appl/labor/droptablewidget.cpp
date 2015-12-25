/*
 * droptablewidget.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "mainwindow.hpp"
#include "droptablewidget.hpp"
#include <QEvent>
#include <QDragEnterEvent>
#include <QMimeData>

DropTableWidget::DropTableWidget(QWidget* parent) :
   QTableWidget(parent),
   m_mainWindow(NULL) {
}
#if 1
void DropTableWidget::dragEnterEvent(QDragEnterEvent* event) {
   event->acceptProposedAction();
}

void DropTableWidget::dragMoveEvent(QDragMoveEvent* event) {
   event->acceptProposedAction();
}
void DropTableWidget::dragLeaveEvent(QDragLeaveEvent* event) {
   event->accept();
}
#endif

QString headOf(const QString& text) {
   QString rc;
   if (text.length() > 2000)
      rc = text.mid(0, 2000) + "...";
   else
      rc = text;
   return rc;
}

void DropTableWidget::dropEvent(QDropEvent* event) {
   const QMimeData* mimeData = event->mimeData();
   m_mainWindow->nextDrop();
   QList<QString> formats = mimeData->formats();
   QList<QString>::const_iterator it;
   for (it = formats.begin(); it != formats.end(); ++it) {
      m_mainWindow->log("format", *it);
   }
   if (mimeData->hasImage()) {
      m_mainWindow->log("image", NULL);
   }
   if (mimeData->hasHtml()) {
      m_mainWindow->log("html", headOf(mimeData->html()));
   }
   if (mimeData->hasText()) {
      m_mainWindow->log("text", headOf(mimeData->text()));
   }
   if (mimeData->hasUrls()) {
      QList<QUrl> urls = mimeData->urls();
      m_mainWindow->log("url-list", "count=" + QString::number(urls.size()));
      QList<QUrl>::const_iterator it;
      int count = 0;
      for (it = urls.begin(); count++ < 3 && it != urls.end(); ++it)
         m_mainWindow->log("url", (*it).path());
   }
   m_mainWindow->setMimeData(mimeData);
   event->acceptProposedAction();
}

void DropTableWidget::setMainWindow(MainWindow* mainWindow) {
   m_mainWindow = mainWindow;
}

