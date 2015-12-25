/*
 * mainwindow.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QMimeData>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
   Q_OBJECT

public:
   explicit MainWindow(QWidget* parent = 0);
   ~MainWindow();
public:
   double fontWidth();
   void log(const QString& type, const QString& data);
   void nextDrop() {
      m_dropNo++;
   }
   bool isBinary(const QByteArray& data);
   void setData(const char* data, int dataSize, int lineLength = -1);
   void setMimeData(const QMimeData* data);
private slots:
   void currentIndexChanged(const QString& text);
private:
   Ui::MainWindow* ui;
   int m_dropNo;
   QMap<QString, QByteArray> m_mimeData;
};

#endif // MAINWINDOW_HPP
