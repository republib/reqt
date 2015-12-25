/*
 * converter.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "gui/regui.hpp"
#include "converter.hpp"
#include "mainwindow.hpp"
#include <QRegularExpression>
#include <QRegExp>

/** @file
 *
 * @section intro_sec Introduction
 *
 * The user can determine the maximum width/length for the three formats:
 * <ul>
 * <li>landscape: width greater than height</li>
 * <li>portrait: witdh lower then height</li>
 * <li>square: width equal to height</li>
 * </ul>
 * At a given directory all images will be converted by this rules.
 * The created files will be stored in another directory (normally a
 * subdirectory of the source directory).
 *
 * @subsection conv_sec Conversion Details
 *
 * If an image is lower than the given limits it will be copied without
 * conversion
 * This program is only the GUI of an Perl script which does the conversion.
 * The script uses the
 *
 * @section install_sec Installation
 *
 * Copy reimgconvert into a directory inserted in the executable
 * path, e.g. /usr/local/bin and give them the right "executable".
 *
 * @subsection req_sec Requirements
 *
 * <ul>
 * <li>ImageMagick</li>
 * </ul>
 *
 * @section prog_sec Programming Features
 *
 * This is a example for a complete QT application with the following features:
 * <ul>
 * <li>C++</li>
 * <li>QT</li>
 * <li>Threads</li>
 * <li>Using GUI wizzard</li>
 * <li>Abstract class as interface</li>
 * <li>Calling an external script</li>
 * <li>Lists the output of the external list line by line at the moment
 *  the line is created</li>
 * </ul>
 *
 * @section version_sec Release Notes
 *
 * <ul>
 * <li>V1.0 Basic version. Predefined (static) values of the combo boxes.
 *      User language is English only.
 * </li>
 * </ul>
 *
 * @section Licence
 *
 * You can use and modify this file without any restriction.
 * There is no warranty.
 * You also can use the licence from http://www.wtfpl.net/.
 * The original sources can be found on https://github.com/republib
 */

/** @class ConverterException converter.hpp "converter.hpp"
 *
 * @brief Implements a specific exception for the converter.
 *
 * Will be used on errors.
 */

/** @class ConvertLogger converter.hpp "converter.hpp"
 *
 * @brief Implements an abstract base class for logging.
 */

/** @class Converter converter.hpp "converter.hpp"
 *
 * @brief Implements a converter for images.
 *
 * Search all images in a given path and converts them into the given format.
 *
 * The task is done in a thread because it can take long time.
 *
 * The thread can be stopped from outside, then the conversion of the current
 * image will be finished and no more files will be processed.
 *
 */

QString sizeToString(qint64 size);

/**
 * @brief Constructor.
 *
 * @param directory         the source directory. The images will be searched here
 * @param targetDirectory   the target directory
 * @param sourcePattern		the pattern for the files to process
 * @param targetType		the target image type
 * @param landscapeX        0 or the given maximal width for landscape format
 * @param landscapeY        0 or the given maximal height for landscape format
 * @param portraitX         0 or the given maximal width for portrait format
 * @param portraitY         0 or the given maximal height for landscape format
 * @param squareX           0 or the given maximal width for square format
 * @param quality           the quality factor: 1..100. Higher value is better
 *                          but larger
 * @param mainWindow        the window. Will be used for output
 */
Converter::Converter(const QString& directory, const QString& targetDirectory,
                     const QString& sourcePattern, const QString& targetType, int landscapeX,
                     int landscapeY, int portraitX, int portraitY, int squareX, int quality,
                     MainWindow* mainWindow) :
   m_dir(directory),
   m_targetDir(
      targetDirectory.indexOf(QDir::separator()) >= 0 ?
      targetDirectory :
      directory + QDir::separator() + targetDirectory),
   m_sourcePattern(sourcePattern),
   m_targetType(targetType),
   m_landscapeWidth(landscapeX),
   m_landscapeHeight(landscapeY),
   m_portraitWidth(portraitX),
   m_portraitHeight(portraitY),
   m_squareWidth(squareX),
   m_quality(targetType == "jpg" ? quality : 0),
   m_mainWindows(mainWindow),
   m_shouldStop(false),
   m_imageInfo(
      new QRegularExpression(" (PNG|GIF|JPEG) (\\d+)x(\\d+) ")),
   m_groupWidth(2),
   m_groupHeight(3) {
}

/**
 * @brief Destructor.
 */
Converter::~Converter() {
   delete m_imageInfo;
}

/**
 * @brief Appends an argument to the argument list.
 *
 * @param args      IN/OUT: the argument list
 * @param prefix    the string starting the argument
 * @param value     the value of the argument
 */
void addArg(QStringList& args, const char* prefix, int value) {
   QString arg;
   arg.sprintf("%s%d", prefix, value);
   args << arg;
}

/**
 * @brief Changes the state of the thread.
 *
 * @param state     the new state
 * @param info      an info about the change
 */
void Converter::changeState(Converter::State state, const QString& info) {
   m_mainWindows->on_threadStateChanged(state, info);
}

/**
 * Converts an image into another format.
 *
 * @param source    the source filename with path
 * @param target    the target filename with path
 * @param width     the old width of the image
 * @param height    the old height of the image
 * @param widthNew  the new width of the image
 * @param heightNew the new height of the image
 * @param quality   0 or quality in % (only for JPEG targets)
 */
void Converter::convert(const QString& source, const QString& target, int width,
                        int height, int widthNew, int heightNew, int quality) {
   QStringList args;
   // convert -size 100x200 source.png -resize 50x100 target.jpg";
   args << "-size" << QString::number(width) + "x" + QString::number(height);
   args << source;
   if (quality > 0)
      args << "-quality" << QString::number(quality);
   args << "-resize"
        << QString::number(widthNew) + "x" + QString::number(heightNew);
   args << target;
   QProcess process;
   process.start("/usr/bin/convert", args);
   QByteArray output;
   while (process.waitForReadyRead()) {
      output = process.readAll();
   }
   output = process.readAllStandardError();
   if (!output.isEmpty())
      error(output);
   process.close();
}

/**
 * @brief Converts one file.
 *
 * @param source    the file's name with path
 * @param target    the new filename with path
 * @param size  the size of the file (in byte)
 */
void Converter::convertOneFile(const QString& source, const QString& target,
                               qint64 size) {
   int width = 0;
   int height = 0;
   QString info;
   clock_t start = clock();
   if (readProperties(source, width, height, info)) {
      bool doConvert = false;
      int widthNew, heightNew;
      if (abs(width - height) < 5) {
         // Square format:
         doConvert = width > m_squareWidth;
         if (doConvert)
            widthNew = heightNew = m_squareWidth;
      } else if (width > height) {
         // Landscape:
         doConvert = width > m_landscapeWidth || height > m_landscapeHeight;
         if (doConvert) {
            if (width > m_landscapeWidth && m_landscapeWidth > 0) {
               widthNew = m_landscapeWidth;
               heightNew = height * m_landscapeWidth / width;
            } else {
               heightNew = m_landscapeHeight;
               widthNew = width * m_landscapeHeight / height;
            }
         }
      } else {
         // Portrait
         doConvert = width > m_portraitWidth || height > m_portraitHeight;
         if (doConvert) {
            if (width > m_portraitWidth && m_portraitWidth > 0) {
               widthNew = m_portraitWidth;
               heightNew = height * m_portraitWidth / width;
            } else {
               heightNew = m_portraitHeight;
               widthNew = width * m_portraitHeight / height;
            }
         }
      }
      log(
         source + " " + info + " " + sizeToString(size)
         + QString(" -> %1x%2 ").arg(widthNew).arg(heightNew));
      convert(source, target, width, height, widthNew, heightNew, m_quality);
      struct stat info;
      if (stat(I18N::s2b(target).constData(), &info) == 0)
         m_mainWindows->logAppendLast(sizeToString(info.st_size) + " ");
   }
   m_mainWindows->logAppendLast(
      QString("").sprintf("%.3f sec",
                          double(clock() - start) / CLOCKS_PER_SEC));
}

/**
 * @brief Logs an error message.
 *
 * @param message   the message to log
 * @return          <code>false</code>
 * @throws ConverterException
 */
bool Converter::error(const QString& message) {
   m_mainWindows->log(message);
   throw ConverterException(message);
   return false;
}

/**
 * @brief Logs a message.
 *
 * @param message   the message to log
 * @return          <code>true</code>
 */
bool Converter::log(const QString& message) {
   printf("%s\n", I18N::s2b(message).constData());
   m_mainWindows->log(message);
   return true;
}

/**
 * Reads the image properties using the external program "identify".
 *
 * @param name      the filename with path
 * @param width     OUT: the width of the image
 * @param height    OUT: the height of the image
 * @param info      OUT: an info about the image
 * @return          <code>true</code>: success
 *                  <code>false</code>: properties not readable
 */
bool Converter::readProperties(const QString& name, int& width, int& height,
                               QString& info) {
   QStringList args;
   args << name;
   QProcess process;
   process.start("/usr/bin/identify", args);
   QByteArray output;
   bool rc = false;
   while (process.waitForReadyRead()) {
      output = process.readAll();
      QRegularExpressionMatch match = m_imageInfo->match(output);
      if (!match.hasMatch())
         error(
            QObject::tr("I am confused (wrong image data):\n%1\nExpected: %2").arg(
               output.constData()).arg(m_imageInfo->pattern()));
      else {
         width = match.captured(m_groupWidth).toInt();
         height = match.captured(m_groupHeight).toInt();
         rc = true;
         info = QString("%1x%2").arg(width).arg(height);
         break;
      }
   }
   output = process.readAll();
   output = process.readAllStandardError();
   if (!output.isEmpty())
      error(output);
   process.close();
   return rc;
}

/**
 * @brief Runs the thread's task.
 *
 * <ul>
 *<li>Makes the target directory (if necessary)</li>
 *<li>Search the images *.png / *.jpg and converts them</li>
 *</ul>
 */
void Converter::run() {
   QString msg;
   int no = 0;
   try {
      if (!m_dir.exists())
         error(
            QObject::tr("Directory does not exist: ") + m_dir.absolutePath());
      if (!m_targetDir.exists()) {
         QString parentName = m_targetDir.path();
         QString subdir = m_targetDir.dirName();
         QDir parent(m_targetDir.path());
         parent.cdUp();
         parent.mkdir(subdir);
      }
      if (!m_targetDir.exists()) {
         error(
            QObject::tr("Cannot create the target directory: ")
            + m_targetDir.absolutePath());
      }
      changeState(Converter::STATE_STARTING, "");
      m_shouldStop = false;
      QDirIterator it(m_dir.absolutePath());
      QRegExp regExpr(m_sourcePattern, Qt::CaseInsensitive, QRegExp::Wildcard);
      while (it.hasNext()) {
         if (m_shouldStop) {
            log(QObject::tr("Canceled by the user"));
            break;
         }
         it.next();
         if (it.fileInfo().isDir())
            continue;
         QString node = it.fileName();
         if (regExpr.indexIn(node) >= 0) {
            no++;
            QString path = m_dir.absoluteFilePath(node);
            qint64 length = it.fileInfo().size();
            QString nodeTarget = ReQStringUtil::replaceExtension(node,
                                 "." + m_targetType);
            QString target = m_targetDir.absoluteFilePath(nodeTarget);
            convertOneFile(path, target, length);
         }
      }
      changeState(Converter::STATE_SUB_TASK_STOPPED, msg);
   } catch (ConverterException exc) {
      log(
         QObject::tr("Execution stopped because of error(s): ")
         + exc.message());
   }
   msg = QObject::tr("%1 file(s) converted").arg(no);
   changeState(Converter::STATE_READY, msg);
   m_mainWindows->switchRun(true);
}

/**
 * @brief Search for the Perl script which make the conversion itself.
 *
 * @param node  the name of script without path.
 * @return      the full name of the script (with path)
 */
QString findScript(const QString& node) {
   static QString rc;
   if (rc.isEmpty()) {
      QDir dir = QDir::current();
      QFile scriptFile(dir.filePath(node));
      if (!scriptFile.exists()) {
         extern char** g_argv;
         dir.setPath(g_argv[0]);
         dir.cdUp();
         scriptFile.setFileName(dir.filePath(node));
      }
      if (scriptFile.exists())
         rc = scriptFile.fileName();
   }
   return rc;
}
/**
 * @brief Converts the size into a human readable string.
 *
 * @param size  the size in bytes
 * @return      the size as human readable string, e.g. "2MiByte"
 */
QString sizeToString(qint64 size) {
   QString rc;
   if (size < 10 * 1024)
      rc.sprintf("%d Bytes", (int) size);
   else if (size < qint64(10 * 1024 * 1024))
      rc.sprintf("%d KiBytes", (int) (size / 1024));
   else if (size < qint64(10 * 1024 * 1024) * 1024)
      rc.sprintf("%d MiBytes", (int) (size / 1024 / 1024));
   else
      rc.sprintf("%d GiBytes", (int) (size / 1024 / 1024 / 1024));
   return rc;
}

