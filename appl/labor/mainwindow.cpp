/*
 * mainwindow.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#include "mainwindow.hpp"
#include "ui_mainwindow.h"

enum {
   COL_NO,
   COL_TYPE,
   COL_CONTENT
};
MainWindow::MainWindow(QWidget* parent) :
   QMainWindow(parent),
   ui(new Ui::MainWindow),
   m_dropNo(0) {
   ui->setupUi(this);
   ui->tableWidgetDragInfo->setMainWindow(this);
   connect(ui->comboBoxMimeType, SIGNAL(currentIndexChanged(const QString&)),
           this, SLOT(currentIndexChanged(const QString&)));
}

MainWindow::~MainWindow() {
   delete ui;
}

void MainWindow::log(const QString& type, const QString& data) {
   ui->tableWidgetDragInfo->insertRow(0);
   ui->tableWidgetDragInfo->setItem(0, COL_NO, new QTableWidgetItem(QString::number(m_dropNo)));
   ui->tableWidgetDragInfo->setItem(0, COL_TYPE, new QTableWidgetItem(type));
   ui->tableWidgetDragInfo->setItem(0, COL_CONTENT, new QTableWidgetItem(data));
}

bool MainWindow::isBinary(const QByteArray& data) {
   bool rc = false;
   for (int ix = 0; ix < data.length(); ix++) {
      if (data.at(ix) == 0) {
         rc = true;
         break;
      }
   }
   return rc;
}

double MainWindow::fontWidth() {
   QFont myFont;
   QString str("0005: 61 62 63 64 65 66 67 68 69 6a|abcdefghijk");
   QFontMetrics fm(myFont);
   double width = fm.width(str) / str.length();
   return width;
}
inline char toHex(int value) {
   return value < 10 ? '0' + value : 'a' + (value - 10);
}

void MainWindow::setData(const char* data, int dataSize, int lineLength) {
   if (lineLength < 0)
      lineLength = int (ui->plainTextEdit->width() / fontWidth()) - 1;
   int offsetWidth = dataSize >= 1000*1000 ? 10 : dataSize > 10000 ? 6 : 4;
   int charPerLine = (lineLength - offsetWidth - 1)  / 4;
   int countLines = (dataSize + charPerLine - 1) / charPerLine;
   int restLength = dataSize;
   char formatOffset[10];
   snprintf(formatOffset, sizeof formatOffset, "%%0%dd:", offsetWidth);
   const unsigned char* ptr = reinterpret_cast<const unsigned char*>(data);
   QByteArray buffer;
   buffer.reserve(countLines * (charPerLine + 1) + 100);
   char lineNo[32];
   int offset = 0;
   while (restLength > 0) {
      snprintf(lineNo, sizeof lineNo, formatOffset, offset);
      buffer.append(lineNo);
      for (int ix = 0; ix < charPerLine; ix++) {
         if (ix >= restLength)
            buffer.append("  ");
         else
            buffer.append(toHex(ptr[ix]/16)).append(toHex(ptr[ix]%16)).append(' ');
      }
      buffer[buffer.length() - 1] = '|';
      for (int ix = 0; ix < charPerLine; ix++) {
         if (ix >= restLength)
            buffer.append(" ");
         else if (ptr[ix] > 0x7f || ptr[ix] < ' ')
            buffer.append('.');
         else
            buffer.append((char) ptr[ix]);
      }
      buffer.append('\n');
      restLength -= charPerLine;
      ptr += charPerLine;
      offset += charPerLine;
   }
   ui->plainTextEdit->setPlainText(QString::fromUtf8(buffer));
}

QByteArray urlAsBytes(const QList<QUrl>& urls) {
   QByteArray rc;
   rc.reserve(urls.length() * 80);
   QList<QUrl>::const_iterator it;
   for (it = urls.begin(); it != urls.end(); ++it) {
      rc.append(it->path()).append('\n');
   }
   return rc;
}

void MainWindow::setMimeData(const QMimeData* mime) {
   m_mimeData.clear();
   QList<QString> formats = mime->formats();
   ui->comboBoxMimeType->clear();
   ui->comboBoxMimeType->addItems(formats);
   for (int ix = 0; ix < formats.length(); ix++) {
      QString name = formats.at(ix);
      if (name.indexOf("html") >= 0)
         m_mimeData[name] = I18N::s2b(mime->html());
      else if (name.indexOf("text") >= 0)
         m_mimeData[name] =  I18N::s2b(mime->text());
      else if (name.indexOf("image") >= 0)
         m_mimeData[name] = mime->imageData().toByteArray();
      else if (name.indexOf("uri-list") >= 0)
         m_mimeData[name] = urlAsBytes(mime->urls());
      else
         m_mimeData[name] = mime->data(name);
   }
}

void MainWindow::currentIndexChanged(const QString& text) {
   QByteArray data = m_mimeData[text];
   if (isBinary(data)) {
      setData(data.constData(), data.length());
   } else {
      ui->plainTextEdit->setPlainText(QString::fromUtf8(data.constData()));
   }
}
