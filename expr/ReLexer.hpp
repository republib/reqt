/*
 * ReLexer.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef RPLLEXER_HPP
#define RPLLEXER_HPP

//#define RPL_LEXER_TRACE

enum RplTokenType {
   TOKEN_UNDEF,
   TOKEN_STRING,
   TOKEN_NUMBER,
   TOKEN_REAL,
   TOKEN_KEYWORD,
   TOKEN_OPERATOR,
   TOKEN_ID,
   TOKEN_COMMENT_REST_OF_LINE,
   TOKEN_COMMENT_START,
   TOKEN_COMMENT_END,
   TOKEN_SPACE,
   TOKEN_END_OF_SOURCE,
   TOKEN_COUNT
};

class ReLexException: public ReException {
public:
   ReLexException(const ReSourcePosition& position, const char* message, ...);
};

class ReLexer;

class ReToken {
public:
   ReToken(RplTokenType type);
   ~ReToken();
   ReToken(const ReToken& source);
   ReToken& operator =(const ReToken& source);
public:
   friend class ReLexer;
   const QByteArray& toString();
   bool isInteger();
   int asInteger() const;
   quint64 asUInt64() const;
   qreal asReal() const;
   const QByteArray& rawString() const;
   int id() const;
   RplTokenType tokenType() const;
   bool isTokenType(RplTokenType expected) const;
   bool isOperator(int expected, int alternative = 0) const;
   bool isKeyword(int expected, int alternative = 0) const;
   void clear();
   bool isCapitalizedId() const;
   QByteArray dump() const;
   static const char* nameOfType(RplTokenType type);
   QByteArray asUtf8() const;
protected:
   RplTokenType m_tokenType;
   QByteArray m_string;
   // only for TOKEN_STRING: copy from source but with escaped chars like "\\n"
   QByteArray m_printableString;
   union {
      // only for TOKEN_KEYWORD and TOKEN_OPERATOR
      int m_id;
      quint64 m_integer;
      qreal m_real;
   } m_value;
};

class ReSource;
class ReLexer {
public:
   typedef QList<QByteArray> StringList;
   enum NumericType {
      NUMTYPE_UNDEF,
      NUMTYPE_DECIMAL = 1 << 0,
      NUMTYPE_OCTAL = 1 << 1,
      NUMTYPE_HEXADECIMAL = 1 << 2,
      NUMTYPE_FLOAT = 1 << 3,
      ///
      NUMTYPE_ALL_INTEGER = NUMTYPE_DECIMAL | NUMTYPE_OCTAL
                            | NUMTYPE_HEXADECIMAL,
      NUMTYPE_ALL = NUMTYPE_ALL_INTEGER | NUMTYPE_FLOAT
   };
   enum CharClassTag {
      CC_UNDEF = 0,
      /// this char is possible as first char of an id
      CC_FIRST_ID = 1 << 0,
      /// this char is possible as 2nd char of an id
      CC_2nd_ID = 1 << 1,
      /// this char is possible as 3rd char of an id
      CC_3rd_ID = 1 << 2,
      /// this char is possible as 4th... char of an id
      CC_REST_ID = 1 << 3,
      /// this char can start a comment
      CC_FIRST_COMMENT_START = 1 << 4,
      /// this char can be the 2nd char of a comment start
      CC_2nd_COMMENT_START = 1 << 5,
      /// this char can be the 3rd char of a comment start
      CC_3rd_COMMENT_START = 1 << 6,
      /// this char can be the 4th ... of a comment start
      CC_REST_COMMENT_START = 1 << 7,
      /// this char can start a keyword
      CC_FIRST_KEYWORD = 1 << 8,
      /// this char can be the 2nd char of a keyword
      CC_2nd_KEYWORD = 1 << 9,
      /// this char can be the 3rd char of a keyword
      CC_3rd_KEYWORD = 1 << 10,
      /// this char can be the 4th... char of a keyword
      CC_REST_KEYWORD = 1 << 11,
      /// this char can be the 1st char of an operator
      CC_FIRST_OP = 1 << 12,
      /// this char can be the 2nd char of an operator
      CC_2nd_OP = 1 << 13,
      /// this char can be the 3rd char of an operator
      CC_3rd_OP = 1 << 14,
      /// this char can be the 4th... char of an operator
      CC_REST_OP = 1 << 15,
      /// there is an operator with exactly this char
      /// and there is no other operator starting with this char
      CC_OP_1_ONLY = 1 << 16
   };
   enum StringFeatures {
      SF_UNDEF,
      /// ' can be a string delimiter
      SF_TICK = 1 << 1,
      /// " can be a string delimiter
      SF_QUOTE = 1 << 2,
      /// character escaping like in C: "\x" is "x"
      SF_C_ESCAPING = 1 << 3,
      /// special characters like in C: "\r" "\f" "\n" "\t" "\a"
      SF_C_SPECIAL = 1 << 4,
      /// characters can be written in hexadecimal notation: "\x20" is " "
      SF_C_HEX_CHARS = 1 << 5,
      /// A delimiter inside a string must be doubled (like in Pascal)
      SF_DOUBLE_DELIM = 1 << 6,
      // Redefinitions for better reading:
      SF_LIKE_C = SF_TICK | SF_QUOTE | SF_C_ESCAPING | SF_C_SPECIAL
                  | SF_C_HEX_CHARS
   };
   enum StorageFlags {
      S_UNDEF,
      /// the original string will be stored in m_string too
      /// (not only m_rawString)
      STORE_ORG_STRING = 1 << 1,
      /// comments will be stored in m_string
      STORE_COMMENT = 1 << 2,
      /// blanks will be stored in m_string
      STORE_BLANK = 1 << 3,
      /// redefinitions for better reading:
      STORE_NOTHING = 0,
      STORE_ALL = STORE_ORG_STRING | STORE_COMMENT | STORE_BLANK
   };

public:
   ReLexer(ReSource* source, const char* keywords, const char* operators,
           const char* rightAssociatives, const char* comments,
           const char* firstCharsId = "a-zA-Z_", const char* restCharsId =
              "a-zA-Z0-9_",
           int numericTypes = NUMTYPE_DECIMAL | NUMTYPE_HEXADECIMAL
                              | NUMTYPE_FLOAT,
           int stringFeatures = SF_TICK | SF_QUOTE | SF_C_ESCAPING | SF_C_SPECIAL
                                | SF_C_HEX_CHARS, int storageFlags = STORE_NOTHING);
   virtual ~ReLexer();
public:
   ReToken* nextToken();
   void undoLastToken();
   void undoLastToken2();
   void saveLastToken();
   ReToken* peekNonSpaceToken();
   ReToken* nextNonSpaceToken();
   size_t maxTokenLength() const;
   void setMaxTokenLength(size_t maxTokenLength);
   void startUnit(ReSourceUnitName unit);
   ReSource* source();
   int prioOfOp(int op) const;
   const QByteArray& nameOfOp(int op) const;
   bool isRightAssociative(int op) const;
   const ReSourcePosition* currentPosition() const;
   ReToken* currentToken() const;
#if defined RPL_LEXER_TRACE
   bool trace() const;
   void setTrace(bool trace);
#endif
private:
   void prepareOperators(const char* operators, const char* rightAssociatives);
   void initializeComments(const char* comments);
   bool fillInput();
   int findInVector(int tokenLength, const StringList& vector);
   ReToken* findTokenWithId(RplTokenType tokenType, int flag2,
                            StringList& names);
   ReToken* scanNumber();
   ReToken* scanString();
   void scanComment();
protected:
   ReSource* m_source;
   /// sorted, string ends with the id of the keyword
   StringList m_keywords;
   // sorted, string ends with the id of the operator
   StringList m_operators;
   // sorted, each entry ends with the id of the comment start
   StringList m_commentStarts;
   // index: id content: comment_end
   StringList m_commentEnds;
   // index: ord(char) content: a sum of CharClassTags
   int m_charInfo[128];
   // a list of QChars with ord(cc) > 127 and which can be the first char
   QByteArray m_idFirstRare;
   // index: ord(char) content: chr(ix) can be the non first char of an id
   QByteArray m_idRestRare;
   // a list of QChars with ord(cc) > 127 and which can be the first char
   int m_numericTypes;
   QByteArray m_idRest2;
   ReToken* m_currentToken;
   ReToken* m_waitingToken;
   ReToken* m_waitingToken2;
   ReToken m_token1;
   ReToken m_token2;
   const ReSourcePosition* m_currentPosition;
   const ReSourcePosition* m_waitingPosition1;
   const ReSourcePosition* m_waitingPosition2;
   int m_maxTokenLength;
   QByteArray m_input;
   int m_currentCol;
   bool m_hasMoreInput;
   int m_stringFeatures;
   int m_storageFlags;
   /// priority of the operators: index: id of the operator. content: prio
   char m_prioOfOp[128];
   char m_assocOfOp[128];
   QList<QByteArray> m_opNames;
#if defined (RPL_LEXER_TRACE)
   bool m_trace;
#endif
};

#endif // RPLLEXER_HPP
