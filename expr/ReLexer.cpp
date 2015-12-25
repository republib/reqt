/*
 * ReLexer.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Configurable scanner, which separates syntactic symbols from an input media.
 */
/** @file expr/ReLexer.hpp
 * @brief Definitions for a configurable lexical analyser.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

#define CHAR_INFO_SIZE (int(sizeof m_charInfo / sizeof m_charInfo[0]))

/** @class ReToken ReLexer.hpp "expr/ReLexer.hpp"
 *
 * @brief Implements specific exception for the lexer.
 *
 */

/**
 * @brief Constructor.
 *
 * @param position  describes the position of the error/warning
 * @param format    the reason of the exception
 * @param ...       the values for the placeholders in the format.
 */
ReLexException::ReLexException(const ReSourcePosition& position,
                               const char* format, ...) :
   ReException("") {
   char buffer[64000];
   m_message = I18N::s2b(position.toString());
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   m_message += buffer;
}

/** @class ReToken ReLexer.hpp "expr/ReLexer.hpp"
 *
 * @brief Implements a token which is the smallest unit for a parser.
 *
 */
/**
 * @brief Constructor.
 * @param type  token type
 */
ReToken::ReToken(RplTokenType type) :
   m_tokenType(type),
   m_string(),
   m_printableString()
// m_value
{
   memset(&m_value, 0, sizeof m_value);
}

/**
 * @brief Destructor.
 */
ReToken::~ReToken() {
}
/**
 * @brief Copy constructor.
 *
 * @param source    source to copy
 */
ReToken::ReToken(const ReToken& source) :
   m_tokenType(source.m_tokenType),
   m_string(source.m_string),
   m_printableString(source.m_printableString),
   m_value(source.m_value) {
}
/**
 * @brief Assignment operator.
 *
 * @param source    source to copy
 * @return
 */
ReToken& ReToken::operator =(const ReToken& source) {
   m_tokenType = source.m_tokenType;
   m_string = source.m_string;
   m_value = source.m_value;
   return *this;
}

/**
 * @brief Returns the string representation of the instance
 * @return a string representing the instance
 */
const QByteArray& ReToken::toString() {
   return m_string;
}

/**
 * @brief Returns the integer value of the token
 *
 * Only relevant if a TOKEN_NUMBER.
 *
 * @return the value of the token as integer
 */
int ReToken::asInteger() const {
   return (int) m_value.m_integer;
}

/**
 * @brief Returns the integer value of the token
 *
 * Only relevant if a TOKEN_NUMBER.
 *
 * @return the value of the token as unsigned integer (64 bit)
 */
quint64 ReToken::asUInt64() const {
   return m_value.m_integer;
}

/**
 * @brief Returns the floating point value of the token
 *
 * Only relevant if a TOKEN_REAL.
 *
 * @return the value of the token as floating point value
 */
qreal ReToken::asReal() const {
   return m_value.m_real;
}

/**
 * @brief Returns the floating point value of the token
 *
 * Only relevant if a TOKEN_NUMBER.
 *
 * @return the value of the token as floating point value
 */
const QByteArray& ReToken::rawString() const {
   return m_printableString;
}
/**
 * @brief Returns the id of the token.
 *
 * Ids are more handy than string, e.g. allowing switch statements.
 *
 * Only relevant for TOKEN_KEYWORD and TOKEN_OPERATOR.
 *
 * @return the id of the token
 */
int ReToken::id() const {
   return m_value.m_id;
}
/**
 * @brief Returns the token type.
 * @return the token type
 */
RplTokenType ReToken::tokenType() const {
   return m_tokenType;
}

/**
 * @brief Checks whether the instance has a given token type.
 *
 * @param expected  the token type to compare
 *
 * @return  true: the expected type is the current<br>
 *          false: otherwise
 */
bool ReToken::isTokenType(RplTokenType expected) const {
   return m_tokenType == expected;
}

/**
 * @brief Checks whether the instance is a given operator.
 *
 * @param expected      the expected operator
 * @param alternative   0 or a second possibility
 *
 * @return  true: the instance is an operator and the expected or the alternative<br>
 *          false: otherwise
 */
bool ReToken::isOperator(int expected, int alternative) const {
   return m_tokenType == TOKEN_OPERATOR
          && (m_value.m_id == expected || m_value.m_id == alternative);
}

/**
 * @brief Checks whether the instance is a given keyword.
 *
 * @param expected      the expected keyword
 * @param alternative   0 or a second possibility
 *
 * @return  true: the instance is a keyword and the expected or the alternative<br>
 *          false: otherwise
 */

bool ReToken::isKeyword(int expected, int alternative) const {
   return m_tokenType == TOKEN_KEYWORD
          && (m_value.m_id == expected || m_value.m_id == alternative);
}

/**
 * @brief Makes all members undefined.
 */
void ReToken::clear() {
   m_string.clear();
   m_printableString.clear();
   m_tokenType = TOKEN_UNDEF;
   m_value.m_integer = 0;
}

/**
 * @brief Returns whether the token is a capitalized id
 *
 * @return  true: the token is an id and the first char is an upper case char<br>
 *          false: otherwise
 */
bool ReToken::isCapitalizedId() const {
   bool rc = m_tokenType == TOKEN_ID && isupper(m_string.at(0))
             && (m_string.length() == 1 || islower(m_string.at(1)));
   return rc;
}

/**
 * @brief Returns the description of the current token.
 *
 * @return a description of the instance
 */
QByteArray ReToken::dump() const {
   QByteArray rc;
   rc = nameOfType(m_tokenType);
   rc.append(": ").append(this->asUtf8());
   return rc;
}
QByteArray ReToken::asUtf8() const {
   char buffer[4096];
   buffer[0] = '\0';
   switch (m_tokenType) {
   case TOKEN_UNDEF:
      break;
   case TOKEN_STRING:
      qsnprintf(buffer, sizeof buffer, "'%.*s'", int(sizeof buffer) - 1,
                m_printableString.constData());
      break;
   case TOKEN_NUMBER:
      qsnprintf(buffer, sizeof buffer, "%lld", m_value.m_integer);
      break;
   case TOKEN_REAL:
      qsnprintf(buffer, sizeof buffer, "%f", m_value.m_real);
      break;
   case TOKEN_KEYWORD:
   case TOKEN_OPERATOR:
      qsnprintf(buffer, sizeof buffer, "%lld", (int) m_value.m_id);
      break;
   case TOKEN_ID:
      qsnprintf(buffer, sizeof buffer, "'%.*s'", int(sizeof buffer) - 1,
                m_string.constData());
      break;
   case TOKEN_COMMENT_REST_OF_LINE:
   case TOKEN_COMMENT_START:
   case TOKEN_COMMENT_END:
   case TOKEN_SPACE:
   case TOKEN_END_OF_SOURCE:
   default:
      break;
   }
   return buffer;
}
/**
 * @brief Returns then name of a token type.
 * @param   type  the type to convert
 * @return  the token type name
 */
const char* ReToken::nameOfType(RplTokenType type) {
   const char* rc = "?";
   switch (type) {
   case TOKEN_UNDEF:
      rc = "undef";
      break;
   case TOKEN_STRING:
      rc = "String";
      break;
   case TOKEN_NUMBER:
      rc = "Number";
      break;
   case TOKEN_REAL:
      rc = "Real";
      break;
   case TOKEN_KEYWORD:
      rc = "Keyword";
      break;
   case TOKEN_OPERATOR:
      rc = "Operator";
      break;
   case TOKEN_ID:
      rc = "Id";
      break;
   case TOKEN_COMMENT_REST_OF_LINE:
      rc = "Comment-1-line";
      break;
   case TOKEN_COMMENT_START:
      rc = "Comment-m-line";
      break;
   case TOKEN_COMMENT_END:
      rc = "end of comment";
      break;
   case TOKEN_SPACE:
      rc = "space";
      break;
   case TOKEN_END_OF_SOURCE:
      rc = "end of source";
      break;
   default:
      break;
   }
   return rc;
}

/** @class ReLexer ReLexer.hpp "expr/ReLexer.hpp"
 *
 * @brief Implements a lexical analyser.
 *
 * A lexical analyser reads a text source and separates the tokens for a parser.
 * Tokens are the smallest elements of a parsing process.
 *
 */

static void itemsToVector(const char* items, ReLexer::StringList& vector,
                          int firstCharFlag, int secondCharFlag, int thirdCharFlag, int restCharFlag,
                          int charInfo[]) {
   QByteArray array2(items);
   QList < QByteArray > list = array2.split(' ');
   QList<QByteArray>::iterator it;
   int id = 0;
   for (it = list.begin(); it < list.end(); it++) {
      QByteArray& item2 = *it;
      QByteArray item(item2);
      id++;
      item.append(' ').append(id % 256).append(id / 256);
      vector.append(item);
      unsigned char cc = item2.at(0);
      if (cc < 128)
         charInfo[cc] |= firstCharFlag;
      if (item2.size() > 1) {
         cc = item2.at(1);
         if (cc < 128)
            charInfo[cc] |= secondCharFlag;
      }
      if (item2.size() > 2) {
         cc = item2.at(2);
         if (cc < 128)
            charInfo[cc] |= thirdCharFlag;
      }
      if (item2.size() > 3) {
         const char* ptr = item2.constData() + 3;
         while ((cc = *ptr++) != '\0') {
            if (cc < 128)
               charInfo[cc] |= restCharFlag;
         }
      }
   }
   qSort(vector.begin(), vector.end(), qLess<QByteArray>());
}

static void charClassToCharInfo(const char* charClass, int flag,
                                int charInfo[]) {
   for (int ix = 0; charClass[ix] != '\0'; ix++) {
      unsigned char cc = (unsigned char) charClass[ix];
      if (cc < 128)
         charInfo[cc] |= flag;
      if (charClass[ix + 1] == '-') {
         unsigned char ubound = charClass[ix + 2];
         if (ubound == '\0')
            charInfo['-'] |= flag;
         else if (cc >= ubound)
            throw new ReException("wrong character class range: %c-%c (%s)",
                                  cc, ubound, charClass);
         else {
            for (int ii = cc + 1; ii <= ubound; ii++) {
               charInfo[ii] |= flag;
            }
         }
         ix += 2;
      }
   }
}

/**
 * @brief Constructor.
 *
 * @param source        the input source handler
 * @param keywords      a string with all keywords delimited by " ".
 *                      Example: "if then else fi while do done"
 * @param operators     a string with the operators separated by blank or "\n".
 *                      "\n" separates the operators with the same priority.
 *                      Lower position means lower priority
 * @param rightAssociatives
 *                      the operators which are right associative
 * @param comments      a string with pairs of comment begin and end delimited
 *                      by " ". The comment end can be "\n" for line end.
 *                      Example: "/ * * / // \n" (ignore the blank in "* /")
 * @param firstCharsId  string with the characters which are allowed as first
 *                      characters of an id
 * @param restCharsId   string with the characters which are allowed as non
 *                      first characters of an id
 * @param numericTypes  bit mask of allowed numeric,
 *                      e.g. NUMTYPE_DECIMAL | NUMTYPE_HEXADECIMAL
 * @param stringFeatures bit mask of the string features,
 *                      e.g. SF_QUOTE | SF_TICK
 * @param storageFlags  describes the things which should be stored, e.g.
 *                      S_ORG_STRINGS | S_COMMENTS | S_BLANKS
 */
ReLexer::ReLexer(ReSource* source, const char* keywords, const char* operators,
                 const char* rightAssociatives, const char* comments,
                 const char* firstCharsId, const char* restCharsId, int numericTypes,
                 int stringFeatures, int storageFlags) :
   m_source(source),
   m_keywords(),
   m_operators(),
   m_commentStarts(),
   m_commentEnds(),
   //m_charInfo()
   m_idFirstRare(),
   m_idRestRare(),
   m_numericTypes(numericTypes),
   m_idRest2(),
   m_currentToken(&m_token1),
   m_waitingToken(NULL),
   m_waitingToken2(NULL),
   m_token1(TOKEN_UNDEF),
   m_token2(TOKEN_UNDEF),
   m_currentPosition(NULL),
   m_waitingPosition1(NULL),
   m_waitingPosition2(NULL),
   m_maxTokenLength(64),
   m_input(),
   m_currentCol(0),
   m_hasMoreInput(false),
   m_stringFeatures(stringFeatures),
   m_storageFlags(storageFlags),
   // m_prioOfOp
   // m_assocOfOp
#if defined (RPL_LEXER_TRACE)
   m_trace(true),
#endif
   m_opNames() {
   memset(m_prioOfOp, 0, sizeof m_prioOfOp);
   memset(m_assocOfOp, 0, sizeof m_assocOfOp);
   memset(m_charInfo, 0, sizeof m_charInfo);
   itemsToVector(keywords, m_keywords, CC_FIRST_KEYWORD, CC_2nd_KEYWORD,
                 CC_3rd_KEYWORD, CC_REST_KEYWORD, m_charInfo);
   prepareOperators(operators, rightAssociatives);
   charClassToCharInfo(firstCharsId, CC_FIRST_ID, m_charInfo);
   charClassToCharInfo(restCharsId, CC_REST_ID, m_charInfo);
   initializeComments(comments);
   m_input.reserve(m_maxTokenLength * 2);
}
/**
 * @brief Destructor.
 */
ReLexer::~ReLexer() {
}

/**
 * @brief Returns the count of blanks in a given range of a string.
 *
 * @param start pointer to the first char to check
 * @param end   pointer to the last char to check
 * @return  the count of blanks
 */
int countBlanks(const char* start, const char* end) {
   int rc = 0;
   while (start != end) {
      if (*start++ == ' ') {
         rc++;
      }
   }
   return rc;
}

/**
 * @brief Stores the operators in the internal members
 *
 * @param operators     a string with the operators separated by blank or '\n'.
 *                      '\n' separates the operators with the same priority.
 *                      Lower position means lower priority
 */
void ReLexer::prepareOperators(const char* operators,
                               const char* rightAssociatives) {
   QByteArray op2(operators);
   QByteArray rightAssociatives2(" ");
   rightAssociatives2 += rightAssociatives;
   op2.replace("\n", " ");
   itemsToVector(op2.constData(), m_operators, CC_FIRST_OP, CC_2nd_OP,
                 CC_3rd_OP, CC_REST_OP, m_charInfo);
   // m_operators is now sorted:
   // test whether the successor of 1 char operators is starting with this char:
   // if not this operator will be marked with CC_OP_1_ONLY:
   for (int ix = 0; ix < m_operators.size() - 1; ix++) {
      // the entries of m_operators end with ' ' and id:
      if (m_operators.at(ix).size() == 1 + 2
            && m_operators.at(ix).at(0) != m_operators.at(ix + 1).at(0)) {
         int cc = (char) m_operators[ix].at(0);
         m_charInfo[cc] |= CC_OP_1_ONLY;
      }
   }
   m_opNames.reserve(m_operators.size() + 1);
   op2 = " " + op2;
   m_opNames = op2.split(' ');
   QByteArray rAssoc = QByteArray(" ") + rightAssociatives + " ";
   for (int opId = m_opNames.size() - 1; opId >= 1; opId--) {
      QByteArray item = " " + m_opNames[opId] + " ";
      if (rAssoc.indexOf(item) >= 0)
         m_assocOfOp[opId] = true;
   }
   const char* start = operators;
   const char* end;
   int prio = 0;
   int endId = 0;
   int startId = 1;
   bool again = true;
   while (again) {
      if ((end = strchr(start, '\n')) == NULL) {
         end = strchr(start, '\0');
         again = false;
      }
      prio++;
      endId = startId + countBlanks(start, end) + 1 - 1;
      while (startId <= endId) {
         m_prioOfOp[startId++] = prio;
      }
      start = end + 1;
   }
}

void ReLexer::initializeComments(const char* comments) {
   if (comments != NULL) {
      QByteArray starters;
      QByteArray comments2(comments);
      int ix = comments2.indexOf("  ");
      if (ix >= 0)
         throw ReException(
            "more than one blank between comment pair(s): col %d %s",
            ix + 1, comments + ix);
      // the index of m_commentEnds is the position number: we need a dummy entry:
      m_commentEnds.append("");
      QList < QByteArray > items = comments2.split(' ');
      QList<QByteArray>::iterator it;
      ix = 0;
      for (it = items.begin(); it != items.end(); it++, ix++) {
         if (ix % 2 == 0) {
            if (ix > 0)
               starters += " ";
            starters += *it;
         } else {
            m_commentEnds.append(*it);
         }
      }
      if (ix % 2 != 0)
         throw ReException("not only pairs in the comment list");
      itemsToVector(starters, m_commentStarts, CC_FIRST_COMMENT_START,
                    CC_2nd_COMMENT_START, CC_3rd_COMMENT_START, CC_REST_COMMENT_START,
                    m_charInfo);
   }
}
/**
 * @brief Searches the prefix of <code>m_input</code> in the vector.
 *
 * @param tokenLength   the length of the prefix in <code>m_input</code>
 * @param vector        the vector to search. Each element contains the id
 *                      as last entry
 * @param id            the id of the entry in the vector. Only set if found
 * @return
 */
int ReLexer::findInVector(int tokenLength, const StringList& vector) {
   int id = 0;
   int lbound = 0;
   int ubound = vector.size() - 1;
   // binary search over the sorted vector:
   while (lbound <= ubound) {
      int half = (ubound + lbound) / 2;
      int compareRc = 0;
      int ix = 0;
      const QByteArray& current = vector[half];
      // vector items end with ' ' and id (2 byte):
      int currentLength = current.size() - 3;
      while (ix < tokenLength && compareRc == 0) {
         if (ix >= currentLength)
            // current is shorter:
            compareRc = 1;
         else
            compareRc = m_input.at(ix) - (int) current.at(ix);
         ix++;
      }
      if (compareRc == 0 && current.at(ix) != ' ')
         // token.size() < current.size():
         compareRc = -1;
      if (compareRc < 0)
         ubound = half - 1;
      else if (compareRc > 0)
         lbound = half + 1;
      else {
         id = current[currentLength + 1] + current[currentLength + 2] * 256;
         break;
      }
   }
   return id;
}
/**
 * @brief Reads data until enough data are available for one token.
 *
 * Data will be read from the current source unit.
 * If this unit does not contain more data the next source unit from the stack
 * will be used until the stack is empty.
 *
 * @return  false: no more input is available<br>
 *          true: data are available
 */
bool ReLexer::fillInput() {
   if (m_hasMoreInput) {
      if (m_input.size() < m_maxTokenLength) {
         m_source->currentReader()->fillBuffer(m_maxTokenLength, m_input,
                                               m_hasMoreInput);
      }
   }
   while (m_input.size() == 0 && m_source->currentReader() != NULL) {
      if (m_source->currentReader()->nextLine(m_maxTokenLength, m_input,
                                              m_hasMoreInput)) {
         m_currentCol = 0;
      }
   }
   return m_input.size() > 0;
}

/**
 * @brief Finds a token with an id: TOKEN_OP, TOKEN_KEYWORD, ...
 *
 * @post    the token is removed from the input
 *
 * @param tokenType the token type
 * @param flag2     the flag of the 2nd char
 * @param names     the vector with the names, sorted
 * @return          NULL: not found<br>
 *                  otherwise: the token
 */
ReToken* ReLexer::findTokenWithId(RplTokenType tokenType, int flag2,
                                  StringList& names) {
   int length = 1;
   int inputLength = m_input.size();
   int cc;
   if (inputLength > 1) {
      cc = m_input[1];
      if (cc < CHAR_INFO_SIZE && (m_charInfo[cc] & flag2)) {
         length++;
         if (inputLength > 2) {
            cc = m_input[2];
            // the 3rd char flag is the "successor" of the 2nd char flag:
            int flag = (flag2 << 1);
            if (cc < CHAR_INFO_SIZE && (m_charInfo[cc] & flag)) {
               length++;
               // the rest char flag is the "successor" of the 3nd char flag:
               flag <<= 1;
               while (length < inputLength) {
                  cc = m_input[length];
                  if (cc < CHAR_INFO_SIZE && (m_charInfo[cc] & flag))
                     length++;
                  else
                     break;
               }
            }
         }
      }
   }
   ReToken* rc = NULL;
   if (!(tokenType == TOKEN_KEYWORD && length < inputLength && (cc =
            m_input[length]) < CHAR_INFO_SIZE && (m_charInfo[cc] & CC_REST_ID))) {
      int id;
      // the length could be too long: the CC_2nd_.. flag could be ambigous
      while ((id = findInVector(length, names)) <= 0) {
         if (length == 1 || tokenType == TOKEN_KEYWORD) {
            break;
         }
         length--;
      }
      if (id > 0) {
         rc = m_currentToken;
         rc->m_tokenType = tokenType;
         rc->m_value.m_id = id;
         if (tokenType == TOKEN_COMMENT_START
               && (m_storageFlags & STORE_COMMENT) != 0)
            rc->m_string.append(m_input.mid(0, length));
         m_input.remove(0, length);
         m_currentCol += length;
      }
   }
   return rc;
}

/**
 * @brief Converts the number into a token.
 *
 * @return  the token with the number
 */
ReToken* ReLexer::scanNumber() {
   int inputLength = m_input.size();
   int cc;
   int length;
   quint64 value = 0;
   if ((cc = m_input[0]) == '0' && inputLength > 1
         && (m_numericTypes & NUMTYPE_HEXADECIMAL)
         && (m_input[1] == 'x' || m_input[1] == 'X')) {
      length = ReStringUtils::lengthOfUInt64(m_input.constData() + 2, 16,
                                             &value);
      if (length > 0)
         length += 2;
      else
         throw ReException("invalid hexadecimal number: no digit behind 'x");
   } else if (cc == '0' && (m_numericTypes & NUMTYPE_OCTAL)
              && inputLength > 1) {
      length = 1;
      while (length < inputLength) {
         if ((cc = m_input[length]) >= '0' && cc <= '7')
            value = value * 8 + cc - '0';
         else if (cc >= '8' && cc <= '9')
            throw ReLexException(*m_currentPosition,
                                 "invalid octal digit: %c", cc);
         else
            break;
         length++;
      }
   } else {
      length = 1;
      value = cc - '0';
      while (length < inputLength) {
         if ((cc = m_input[length]) >= '0' && cc <= '9')
            value = value * 10 + cc - '0';
         else
            break;
         length++;
      }
   }
   m_currentToken->m_value.m_integer = value;
   m_currentToken->m_tokenType = TOKEN_NUMBER;
   if (length + 1 < inputLength
         && ((cc = m_input[length]) == '.' || toupper(cc) == 'E')) {
      qreal realValue;
      int realLength = ReStringUtils::lengthOfReal(m_input.constData(),
                       &realValue);
      if (realLength > length) {
         m_currentToken->m_tokenType = TOKEN_REAL;
         m_currentToken->m_value.m_real = realValue;
         length = realLength;
      }
   }
   m_input.remove(0, length);
   m_currentCol += length;
   return m_currentToken;
}

/**
 * @brief Reads a string into the internal data.
 * @return the token with the string
 */
ReToken* ReLexer::scanString() {
   int delim = m_input[0];
   int inputLength = m_input.size();
   int cc;
   int length = 1;
   m_currentToken->m_tokenType = TOKEN_STRING;
   m_currentToken->m_value.m_id = delim;
   bool again = false;
   do {
      while (length < inputLength && (cc = m_input[length]) != delim) {
         length++;
         if (cc != '\\'
               || (m_stringFeatures
                   & (SF_C_ESCAPING | SF_C_HEX_CHARS | SF_C_SPECIAL)) == 0) {
            m_currentToken->m_string.append(QChar(cc));
         } else {
            if (length >= inputLength)
               throw ReLexException(*m_currentPosition,
                                    "backslash without following character");
            cc = m_input[length++];
            if ((m_stringFeatures & SF_C_HEX_CHARS) && toupper(cc) == 'X') {
               if (length >= inputLength)
                  throw ReLexException(*m_currentPosition,
                                       "missing hexadecimal digit behind \\x");
               cc = m_input[length++];
               int hexVal = ReStringUtils::valueOfHexDigit(cc);
               if (hexVal < 0)
                  throw ReLexException(*m_currentPosition,
                                       "not a hexadecimal digit behind \\x: %lc",
                                       QChar(cc));
               if (length < inputLength) {
                  cc = m_input[length];
                  int nibble = ReStringUtils::valueOfHexDigit(cc);
                  if (nibble >= 0) {
                     length++;
                     hexVal = hexVal * 16 + nibble;
                  }
               }
               m_currentToken->m_string.append(QChar(hexVal));
            } else if ((m_stringFeatures & SF_C_SPECIAL)) {
               switch (cc) {
               case 'r':
                  cc = '\r';
                  break;
               case 'n':
                  cc = '\n';
                  break;
               case 't':
                  cc = '\t';
                  break;
               case 'a':
                  cc = '\a';
                  break;
               case 'v':
                  cc = '\v';
                  break;
               case 'f':
                  cc = '\f';
                  break;
               default:
                  break;
               }
               m_currentToken->m_string.append(QChar(cc));
            } else {
               m_currentToken->m_string.append(QChar(cc));
            }
         }
      }
      if (cc == delim) {
         length++;
      }
      if ((m_stringFeatures & SF_DOUBLE_DELIM) && length < inputLength
            && m_input[length] == (char) delim) {
         m_currentToken->m_printableString.append(delim);
         length++;
         again = true;
      }
   } while (again);
   if (m_storageFlags & STORE_ORG_STRING)
      m_currentToken->m_printableString.append(m_input.mid(0, length));
   m_input.remove(0, length);
   m_currentCol += length;
   return m_currentToken;
}

/**
 * @brief Reads a comment into the internal data.
 *
 * precondition: the current token is prepared yet
 */
void ReLexer::scanComment() {
   int inputLength = m_input.size();
   int length = 1;
   QByteArray& commentEnd = m_commentEnds[m_currentToken->id()];
   int ix;
   if (commentEnd[0] == '\n') {
      // single line comment:
      if (m_storageFlags & STORE_COMMENT)
         m_currentToken->m_string.append(m_input);
      length = inputLength;
   } else {
      // multiline comment:
      while ((ix = m_input.indexOf(commentEnd)) < 0) {
         if (m_storageFlags & STORE_COMMENT)
            m_currentToken->m_string.append(m_input);
         m_input.clear();
         if (!fillInput())
            throw ReLexException(*m_currentPosition,
                                 "comment end not found");
      }
      length = ix + commentEnd.size();
      if (m_storageFlags & STORE_COMMENT)
         m_currentToken->m_string.append(m_input.mid(0, length));
   }
   m_input.remove(0, length);
   m_currentCol += length;
}
#if defined (RPL_LEXER_TRACE)
bool ReLexer::trace() const {
   return m_trace;
}

void ReLexer::setTrace(bool trace) {
   m_trace = trace;
}
#endif
/**
 * @brief Returns the last read token.
 *
 * @return  the current token
 */
ReToken* ReLexer::currentToken() const {
   return m_currentToken;
}

/**
 * @brief Returns the current position.
 *
 * @return  the current source code position
 */
const ReSourcePosition* ReLexer::currentPosition() const {
   return m_currentPosition;
}

/**
 * @brief Returns the next token.
 *
 * @return the next token
 */
ReToken* ReLexer::nextToken() {
   ReToken* rc = NULL;
   int ix;
   if (m_waitingToken != NULL) {
      rc = m_currentToken = m_waitingToken;
      m_waitingToken = m_waitingToken2;
      m_waitingToken2 = NULL;
      m_currentPosition = m_waitingPosition1;
      m_waitingPosition1 = m_waitingPosition2;
      m_waitingPosition2 = NULL;
   } else {
      m_currentToken->clear();
      ReReader* reader = m_source->currentReader();
      if (reader == NULL)
         m_currentToken->m_tokenType = TOKEN_END_OF_SOURCE;
      else {
         m_waitingPosition2 = m_waitingPosition1;
         m_waitingPosition1 = m_currentPosition;
         m_currentPosition = m_source->newPosition(m_currentCol);
         if (!fillInput()) {
            m_currentToken->m_tokenType = TOKEN_END_OF_SOURCE;
         } else {
            int cc = m_input.at(0);
            if (isspace(cc)) {
               //waitingPosition = m_currentPosition;
               m_currentToken->m_tokenType = TOKEN_SPACE;
               ix = 1;
               while (ix < m_input.size() && isspace(m_input.at(ix)))
                  ix++;
               if (m_storageFlags & STORE_BLANK) {
                  m_currentToken->m_string.append(m_input.mid(0, ix));
               }
               m_currentCol += ix;
               m_input.remove(0, ix);
               rc = m_currentToken;
            } else if (isdigit(cc)) {
               rc = scanNumber();
            } else if ((cc == '"' && (m_stringFeatures & SF_QUOTE) != 0)
                       || (cc == '\'' && (m_stringFeatures & SF_TICK) != 0)) {
               rc = scanString();
            } else {
               if (cc >= CHAR_INFO_SIZE)
                  throw ReLexException(*m_currentPosition,
                                       "no lexical symbol can start with this char: %lc",
                                       cc);
               else {
                  if (rc == NULL
                        && (m_charInfo[cc] & CC_FIRST_COMMENT_START)) {
                     rc = findTokenWithId(TOKEN_COMMENT_START,
                                          CC_2nd_COMMENT_START, m_commentStarts);
                     if (rc != NULL)
                        scanComment();
                     //waitingPosition = m_currentPosition;
                  }
                  if (rc == NULL && (m_charInfo[cc] & CC_FIRST_OP)) {
                     if ((m_charInfo[cc] & CC_OP_1_ONLY) == 0) {
                        rc = findTokenWithId(TOKEN_OPERATOR, CC_2nd_OP,
                                             m_operators);
                     } else {
                        rc = m_currentToken;
                        rc->m_tokenType = TOKEN_OPERATOR;
                        rc->m_value.m_id = findInVector(1, m_operators);
                        m_input.remove(0, 1);
                        m_currentCol += 1;
                     }
                  }
                  if (rc == NULL && (m_charInfo[cc] & CC_FIRST_KEYWORD)) {
                     rc = findTokenWithId(TOKEN_KEYWORD, CC_2nd_KEYWORD,
                                          m_keywords);
                  }
                  if (rc == NULL && (m_charInfo[cc] & CC_FIRST_ID)) {
                     int length = 1;
                     while (length < m_input.size() && (cc =
                                                           m_input[length]) < CHAR_INFO_SIZE
                            && (m_charInfo[cc] & CC_REST_ID) != 0)
                        length++;
                     rc = m_currentToken;
                     rc->m_tokenType = TOKEN_ID;
                     rc->m_string.append(m_input.mid(0, length));
                     m_input.remove(0, length);
                     m_currentCol += length;
                  }
               }
            }
         }
      }
   }
   if (rc == NULL || rc->tokenType() == TOKEN_UNDEF) {
      if (m_input.size() == 0) {
         rc = m_currentToken;
         rc->m_tokenType = TOKEN_END_OF_SOURCE;
      } else {
         QByteArray symbol = m_input.mid(0, qMin(20, m_input.size() - 1));
         throw ReLexException(*m_currentPosition,
                              "unknown lexical symbol: %s", symbol.constData());
      }
   }
#if defined (RPL_LEXER_TRACE)
   if (m_trace) {
      char buffer[256];
      printf("token: %s pos: %s\n", m_currentToken->dump().constData(),
             m_currentPosition->utf8(buffer, sizeof buffer));
      if (strstr(buffer, "0:28") != NULL)
         buffer[0] = 0;
   }
#endif
   return rc;
}
/**
 * @brief Reverses the last <code>nextToken()</code>.
 *
 * Makes that <code>nextToken()</code> returns the current token again.
 */
void ReLexer::undoLastToken() {
   m_waitingToken = m_currentToken;
   m_currentToken = m_currentToken == &m_token1 ? &m_token2 : &m_token1;
   m_waitingPosition1 = m_currentPosition;
#if defined (RPL_LEXER_TRACE)
   if (m_trace) {
      char buffer[256];
      printf("undo last token: waiting-token:  %s pos: %s\n",
             m_waitingToken->dump().constData(),
             m_waitingPosition1->utf8(buffer, sizeof buffer));
      if (strcmp(buffer, "<test>:2:6") == 0)
         buffer[0] = 0;
   }
#endif
}

/**
 * @brief Reverses the last <code>nextToken()</code>.
 *
 * Makes that <code>nextToken()</code> returns the current token again.
 */
void ReLexer::undoLastToken2() {
   m_waitingToken2 = m_currentToken;
   m_waitingToken = m_currentToken == &m_token1 ? &m_token2 : &m_token1;
   m_waitingPosition2 = m_waitingPosition1;
   m_waitingPosition1 = m_currentPosition;
}

/**
 * @brief Prevents the current token from data loss.
 *
 * Usage:
 * <pre><code>
 * token1 = nextToken();
 * saveLastToken();
 * token2 = nextToken();
 * </code></pre>
 * Then <code>token1</code> and <code>token2</code> contains the wanted content.
 */
void ReLexer::saveLastToken() {
   if (m_waitingToken == NULL)
      m_currentToken = m_currentToken == &m_token1 ? &m_token2 : &m_token1;
}

/**
 * @brief Returns the next relevant token, but the token remains "unread".
 *
 * @return the next token which is not a space/comment
 */
ReToken* ReLexer::peekNonSpaceToken() {
   ReToken* token = nextNonSpaceToken();
   undoLastToken();
   return token;
}
/**
 * @brief Returns the maximal length of a token
 * @return  the maximal length of a token
 */
size_t ReLexer::maxTokenLength() const {
   return m_maxTokenLength;
}

/**
 * @brief Sets the maximal length of a token
 *
 * @param maxTokenLength    the new maximal length of a token
 */
void ReLexer::setMaxTokenLength(size_t maxTokenLength) {
   m_maxTokenLength = maxTokenLength;
}

/**
 * @brief ReLexer::nextNonSpaceToken
 * @return
 */
ReToken* ReLexer::nextNonSpaceToken() {
   ReToken* rc = NULL;
   RplTokenType type;
   do {
      rc = nextToken();
   } while ((type = m_currentToken->tokenType()) == TOKEN_SPACE
            || type == TOKEN_COMMENT_START || type == TOKEN_COMMENT_END
            || type == TOKEN_COMMENT_REST_OF_LINE);
   return rc;
}

/**
 * @brief Prepares a given source unit for reading.
 *
 * Saves the current source position onto the top of stack.
 * Pushes the source unit onto the top of stack.
 *
 * Precondition: the unit must be known by exactly one reader
 *
 * @param unit  the new source unit
 */
void ReLexer::startUnit(ReSourceUnitName unit) {
   m_source->startUnit(unit, *m_currentPosition);
}
/**
 * @brief Returns the source of the instance.
 *
 * @return  the source of the instance
 */
ReSource* ReLexer::source() {
   return m_source;
}

/**
 * @brief Returns the priority of a given operator.
 *
 * @param op    the operator
 * @return      the priority of the op
 */
int ReLexer::prioOfOp(int op) const {
   int rc =
      op > 0 && (unsigned) op < sizeof m_prioOfOp / sizeof m_prioOfOp[0] ?
      m_prioOfOp[op] : 0;
   return rc;
}

/**
 * @brief Returns the name of an operator.
 *
 * @param op    the operator id
 * @return      the name of the operator
 */
const QByteArray& ReLexer::nameOfOp(int op) const {
   const QByteArray& rc = m_opNames.at(op);
   return rc;
}

/**
 * @brief Returns whether an operator is right associative
 * @param op    op to test
 * @return      true: the operator is right associative<br>
 *              false: otherwise
 */
bool ReLexer::isRightAssociative(int op) const {
   bool rc = false;
   if (op >= 0 && (unsigned) op < sizeof m_assocOfOp / sizeof m_assocOfOp[0]) {
      rc = m_assocOfOp[op];
   }
   return rc;
}

