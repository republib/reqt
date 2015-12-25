/*
 * ReParser.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 *
 * @brief Generally usable parts of an parser, e.g. error handling.
 */
/** @file expr/ReParser.hpp
 *
 * @brief Definition of a generally usable parts of an parser.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

/** @class ReSyntaxError ReParser.hpp "expr/ReParser.hpp"
 *
 * @brief Implements an exception used for jumping out from many nested calls.
 *
 * We don't want to cancel the parse process if an syntax error has been
 * occurred. Therefore we want to recreate after it.
 * A relative simple solution:
 * Ignoring the rest of the statement and start again with the next statement.
 * Often the detection is done deep in an expression and we must jump out.
 * This allows this exception.
 */

/**
 * @brief Constructor.
 * @param reason        the reason of the exception
 * @return
 */
ReSyntaxError::ReSyntaxError(const char* reason) :
   m_reason(reason) {
}
/**
 * @brief Returns the description of the exception.
 *
 * @return  the reason
 */
const char* ReSyntaxError::reason() const {
   return m_reason;
}

/** @class RplParserStop ReParser.hpp "expr/ReParser.hpp"
 *
 * @brief Implements an exception used for jumping out from many nested calls.
 *
 * In some situation we want to abort the parsing process.
 * This exception allows this without high costs even the abort position
 * is in a deep nested call.
 */

/**
 * @brief Constructor.
 * @param reason        the reason of the exception
 */
RplParserStop::RplParserStop(const char* reason) :
   ReSyntaxError(reason) {
}

/** @class ReParser ReParser.hpp "expr/ReParser.hpp"
 *
 * @brief Implements a base class for parsers.
 *
 * This class offers common things for all parsers, e.g. error handling.
 */
/**
 * @brief Constructor.
 *
 * @param lexer     the tokenizer
 * @param tree      the abstract syntax tree
 */
ReParser::ReParser(ReLexer& lexer, ReASTree& tree) :
   m_lexer(lexer),
   m_tree(tree),
   m_messages(),
   m_errors(0),
   m_warnings(0),
   m_maxErrors(20),
   m_maxWarnings(20) {
}

/**
 * @brief Common actions for the error/warning functions.
 *
 * @param prefix    first char in the message: 'E' (error) or 'W' (warning)
 * @param location  unique id of the error/warning message
 * @param position  position of the error/warning
 * @param message   message with placeholdes like sprintf()
 * @return          false (for chaining)
 */
bool ReParser::addSimpleMessage(LevelTag prefix, int location,
                                const ReSourcePosition* position, const char* message) {
   char buffer[2048];
   QByteArray msg;
   qsnprintf(buffer, sizeof buffer, "%c%04d %s:%d-%d: ", prefix, location,
             position->sourceUnit()->name(), position->lineNo(), position->column());
   int used = strlen(buffer);
   int length = strlen(message);
   if (length >= (int) sizeof buffer - used)
      length = sizeof buffer - used - 1;
   memcpy(buffer + used, message, length);
   buffer[used + length] = '\0';
   m_messages.append(buffer);
   return false;
}

/**
 * @brief Common actions for the error/warning functions.
 *
 * @param prefix    first char in the message: 'E' (error) or 'W' (warning)
 * @param location  unique id of the error/warning message
 * @param position  position of the error/warning
 * @param format    message with placeholdes like sprintf()
 * @param varList   the variable argument list
 * @return          false (for chaining)
 */
bool ReParser::addMessage(LevelTag prefix, int location,
                          const ReSourcePosition* position, const char* format, va_list varList) {
   char buffer[2048];
   qvsnprintf(buffer, sizeof buffer, format, varList);
   return addSimpleMessage(prefix, location, position, buffer);
}

/**
 * @brief Adds an error message and throws an exception.
 *
 * The exception will be catched at a position where error recovery can take place.
 *
 * @param location  unique id of the error/warning message
 * @param message   error message
 */

void ReParser::syntaxError(int location, const char* message) {
   addSimpleMessage(LT_ERROR, location, m_lexer.currentPosition(), message);
   throw ReSyntaxError(message);
}

/**
 * @brief Adds an error message and throws an exception.
 *
 * This method is used if a closing symbol (e.g. a ')' or 'end') is missed.
 * The message contains a %s as an placeholder for the position of the
 * starting symbol.
 * The exception will be catched at a position where error recovery
 * can take place.
 *
 * @param location  unique id of the error
 * @param message   message describing the error
 * @param symbol    starting symbol corresponding to the missed closing symbol
 * @param position  position of the starting symbol
 */

void ReParser::syntaxError(int location, const char* message,
                           const char* symbol, const ReSourcePosition* position) {
   char buffer[256];
   char buffer2[512];
   qsnprintf(buffer2, sizeof buffer2,
             "The starting symbol %s is located here. Missing point: %s", symbol,
             m_lexer.currentPosition()->utf8(buffer, sizeof buffer));
   addSimpleMessage(LT_ERROR, location, m_lexer.currentPosition(), message);
   addSimpleMessage(LT_INFO, location + 1, position, buffer2);
   throw ReSyntaxError(message);
}

/**
 * @brief Adds an error message.
 *
 * If too much errors an exception will be thrown to stop parsing.
 *
 * @param location  unique id of the error/warning message
 * @param format    message with placeholdes like sprintf()
 * @param ...       optional: the variable argument list
 * @return          false (for chaining)
 */
bool ReParser::error(int location, const char* format, ...) {
   va_list ap;
   va_start(ap, format);
   addMessage(LT_ERROR, location, m_lexer.currentPosition(), format, ap);
   va_end(ap);
   if (++m_errors >= m_maxErrors)
      throw RplParserStop("too many errors");
   return false;
}
/**
 * @brief Adds an error message with an additional info message.
 *
 * If too much errors an exception will be thrown to stop parsing.
 *
 * @param location  unique id of the error/warning message
 * @param position  source position of the additional message
 * @param message   describes the error
 * @param message2  the additional message
 * @param ...       optional: the variable argument list
 * @return          false (for chaining)
 */
bool ReParser::error(int location, const ReSourcePosition* position,
                     const char* message, const char* message2) {
   addSimpleMessage(LT_ERROR, location, m_lexer.currentPosition(), message);
   addSimpleMessage(LT_INFO, location + 1, position, message2);
   if (++m_errors >= m_maxErrors)
      throw RplParserStop("too many errors");
   return false;
}

/**
 * @brief Adds a warning message.
 *
 * If too much warnings an exception will be thrown to stop parsing.
 *
 * @param location  unique id of the error/warning message
 * @param format    message with placeholdes like sprintf()
 * @param ...       optional: the variable argument list
 */
void ReParser::warning(int location, const char* format, ...) {
   va_list ap;
   va_start(ap, format);
   addMessage(LT_WARNING, location, m_lexer.currentPosition(), format, ap);
   va_end(ap);
   if (++m_warnings >= m_maxWarnings)
      throw RplParserStop("too many warnings");
}
/**
 * @brief Return the number of errors.
 *
 * @return the count of errors occurred until now
 */
int ReParser::errors() const {
   return m_errors;
}
/**
 * @brief Return the number of warnings.
 *
 * @return the count of errors occurred until now
 */
int ReParser::warnings() const {
   return m_warnings;
}

