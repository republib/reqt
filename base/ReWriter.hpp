/*
 * ReWriter.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REWRITER_HPP
#define REWRITER_HPP

class ReWriter {
public:
   virtual ~ReWriter();
public:
   /**
    * @brief Writes a text to the output medium.
    *
    * @param message   the message
    */
   virtual void write(const char* message) = 0;
   /**
    * @brief Writes a text line to the output medium.
    *
    * @param line   the text line. If NULL an empty line will be written
    */
   virtual void writeLine(const char* line = NULL) = 0;
   virtual void close();
public:
   void indent(int indent);
   void format(const char* format, ...);
   void formatLine(const char* format, ...);
   void write(va_list ap, const char* format);
   void writeIndented(int indent, const char* line);
   void formatIndented(int indent, const char* format, ...);
protected:
   static const char* m_tabs;
   static int m_maxIndention;
};

class ReFileWriter: public ReWriter {
public:
   ReFileWriter(const char* filename, const char* mode = "w",
                FILE* additionalStream = NULL, const char* eoln = "\n");
public:
   virtual void write(const char* line);
   virtual void writeLine(const char* line = NULL);
   virtual void close();
protected:
   FILE* m_fp;
   QByteArray m_name;
   QByteArray m_eoln;
   FILE* m_additionalStream;
};

#endif // REWRITER_HPP
