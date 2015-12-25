/*
 * textfinder.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef TEXTFINDER_HPP
#define TEXTFINDER_HPP

#include <QRegularExpression>
class TextFinder {
public:
   TextFinder();
   TextFinder(const QString& fullName, int64_t length);
   ~TextFinder();
public:
   void getSearchParameter(const TextFinder& source);
   bool contains();
   bool isBinary();
   bool isText(const QByteArray& data, bool* trueAscii = NULL);
   bool isUTF8(const QByteArray& data, bool* trueAscii) const;
   bool isValid() const;
   void setSearchParameter(const QString& text, bool ignoreCase, bool isRegExpr,
                           bool ignoreBinary);
   QString regExprError();
private:
   bool m_ignoreBinary;
   QString m_filename;
   int64_t m_length;
   QFile m_file;
   bool m_valid;
   QRegularExpression* m_regExpr;
   bool m_isRegExpr;
   bool m_ignoreCase;
   bool m_ownsRegExpr;
   QString m_text;
};

#endif // TEXTFINDER_HPP
