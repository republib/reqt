/*
 * converter.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QProcess>
#include <QList>
#include <QThread>

class MainWindow;
class ConverterException {
public:
   ConverterException(const QString& message) :
      m_message(message) {
   }
   const QString& message() const {
      return m_message;
   }

private:
   QString m_message;
};

class ConvertLogger {
public:
   virtual bool log(const QString& message) = 0;
   virtual bool error(const QString& message) = 0;
};

class Converter: public QThread {
   Q_OBJECT
public:
   enum State {
      STATE_UNDEF, STATE_STARTING, STATE_SUB_TASK_STOPPED, STATE_READY,
   };
public:
   Converter(const QString& directory, const QString& targetDirectory,
             const QString& sourcePattern, const QString& targetType, int landscapeX,
             int landscapey, int portraitX, int portraitY, int squareX, int quality,
             MainWindow* mainWindows);
   ~Converter();
public:
   bool error(const QString& message);
   bool log(const QString& message);
   void run();
   void stop() {
      m_shouldStop = true;
   }
protected:
   bool buildArgs();
   void changeState(State state, const QString& info);
   void convert(const QString& source, const QString& target, int width,
                int height, int widthNew, int heightNew, int quality);
   void convertOneFile(const QString& source, const QString& target,
                       qint64 size);
   bool readProperties(const QString& name, int& width, int& height,
                       QString& info);
private:
   QDir m_dir;
   QDir m_targetDir;
   QString m_sourcePattern;
   QString m_targetType;
   int m_landscapeWidth;
   int m_landscapeHeight;
   int m_portraitWidth;
   int m_portraitHeight;
   int m_squareWidth;
   int m_quality;
   MainWindow* m_mainWindows;
   bool m_shouldStop;
   QRegularExpression* m_imageInfo;
   int m_groupWidth;
   int m_groupHeight;
};

#endif // CONVERTER_HPP
