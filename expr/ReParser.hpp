/*
 * ReParser.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REPARSER_HPP
#define REPARSER_HPP

class ReSyntaxError {
public:
   ReSyntaxError(const char* reason);
public:
   const char* reason() const;
private:
   const char* m_reason;
};

class RplParserStop: public ReSyntaxError {
public:
   RplParserStop(const char* reason);
};

class ReParser {
public:
   enum LevelTag {
      LT_ERROR = 'E',
      LT_WARNING = 'W',
      LT_INFO = 'I'
   };

public:
   typedef QList<QByteArray> MessageList;
public:
   ReParser(ReLexer& lexer, ReASTree& ast);
public:
   bool addSimpleMessage(LevelTag prefix, int location,
                         const ReSourcePosition* pos, const char* message);
   bool addMessage(LevelTag prefix, int location, const ReSourcePosition* pos,
                   const char* format, va_list varList);
   void syntaxError(int location, const char* message);
   void syntaxError(int location, const char* message, const char* symbol,
                    const ReSourcePosition* position);
   bool error(int location, const char* format, ...);
   bool error(int location, const ReSourcePosition* position,
              const char* message, const char* message2);
   void warning(int location, const char* format, ...);
   int errors() const;
   int warnings() const;
protected:
   ReLexer& m_lexer;
   ReASTree& m_tree;
   MessageList m_messages;
   int m_errors;
   int m_warnings;
   int m_maxErrors;
   int m_maxWarnings;
};

#endif // REPARSER_HPP
