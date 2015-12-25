/*
 * ReWriter.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief A writer to an output media.
 *
 * Implementation of the abstract base class <code>ReWriter</code> and
 * the concrete derivation <code>ReFileWriter</code>.
 */
/** @file base/ReWriter.hpp
 *
 * @brief Definitions for a writer to an output media.
 */
#include "base/rebase.hpp"

const char* ReWriter::m_tabs =
   "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
int ReWriter::m_maxIndention = strlen(ReWriter::m_tabs);

/** @class ReWriter ReWriter.hpp "base/ReWriter.hpp"
 *
 * @brief Implements an abstract base class for producing text lines.
 *
 */

/**
 * @brief Destructor.
 *
 * Closes the output medium.
 * Ensures that the destructors of the derived classes are virtual.
 */
ReWriter::~ReWriter() {
   close();
}

/**
 * @brief Closes the output medium.
 *
 * This method does nothing, but overriding methods should free the resources.
 *
 * @note The method must be designed so that it can be called multiple times.
 */
void ReWriter::close() {
}
/**
 * @brief Puts a given count of "\t" to the output medium.
 *
 * @param indent    indention level, number of "\t"
 */
void ReWriter::indent(int indent) {
   if (indent > m_maxIndention)
      indent = m_maxIndention;
   format("%.*s", indent, m_tabs);
}

/**
 * @brief Formats a string and write it to the output medium.
 *
 * @param format    format string with placeholders like <code>sprintf()</code>
 * @param ...       variable arguments, values for the placeholders
 */
void ReWriter::format(const char* format, ...) {
   va_list ap;
   va_start(ap, format);
   write(ap, format);
   va_end(ap);
}
/**
 * @brief Formats a line and write it to the output medium.
 *
 * @param format    format string with placeholders like <code>sprintf()</code>
 * @param ...       variable arguments, values for the placeholders
 */
void ReWriter::formatLine(const char* format, ...) {
   char buffer[64000];
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   writeLine(buffer);
}

/**
 * @brief Formats a message and writes it to the output medium.
 *
 * @param ap        variable argument list (like in <code>vsprintf</code>)
 * @param format    format string with placeholders
 */
void ReWriter::write(va_list ap, const char* format) {
   char buffer[64000];
   qvsnprintf(buffer, sizeof buffer, format, ap);
   write(buffer);
}

/**
 * @brief Writes a line with indention to the output medium.
 *
 * @param indent    indention level. Indention is limited to 20
 * @param line      the line to write
 */
void ReWriter::writeIndented(int indent, const char* line) {
   ReWriter::indent(indent);
   writeLine(line);
}

/**
 * @brief Writes a line with indention to the output medium.
 *
 * @param indent    indention level. Indention is limited to 20
 * @param format    format string with placeholders like <code>sprintf</code>
 * @param ...       the values for the placeholders (variable arguments)
 */
void ReWriter::formatIndented(int indent, const char* format, ...) {
   ReWriter::indent(indent);
   char buffer[64000];
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   writeLine(buffer);
}

/** @class ReWriter ReWriter.hpp "base/ReWriter.hpp"
 *
 * @brief Implements a class which writes lines into a file.
 */

/**
 * @brief Constructor.
 *
 * @param filename          the file's name
 * @param mode              write mode, "w" for write or "a" for append
 * @param additionalStream  if not NULL the content will be written to this
 *                          stream too. Normal usage: <code>stdout</code> or
 *                          <code>stderr</code>
 * @param eoln              line end: "\n" or "\r\n"
 */
ReFileWriter::ReFileWriter(const char* filename, const char* mode,
                           FILE* additionalStream, const char* eoln) :
   m_fp(fopen(filename, mode)),
   m_name(filename),
   m_eoln(eoln),
   m_additionalStream(additionalStream) {
}

/**
 * @brief Writes a string to the file.
 * @param message   the string to write
 */
void ReFileWriter::write(const char* message) {
   if (m_fp != NULL)
      fputs(message, m_fp);
   if (m_additionalStream != NULL)
      fputs(message, m_additionalStream);
}

/**
 * @brief Writes a line to the file.
 * @param line   the line to write. If NULL an empty line will be written
 */
void ReFileWriter::writeLine(const char* line) {
   if (m_fp != NULL) {
      if (line != NULL)
         fputs(line, m_fp);
      fputs(m_eoln, m_fp);
   }
   if (m_additionalStream != NULL) {
      if (line != NULL)
         fputs(line, m_additionalStream);
      fputc('\n', m_additionalStream);
   }
}

/**
 * @brief Closes the output file.
 */
void ReFileWriter::close() {
   if (m_fp != NULL) {
      fclose(m_fp);
      m_fp = NULL;
   }
   m_additionalStream = NULL;
}
