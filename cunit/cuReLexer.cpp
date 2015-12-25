/*
 * cuReLexer.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the syntax symbol extractor.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

class TestReLexer: public ReTest, public ReToken {
public:
   TestReLexer() :
      ReTest("ReLexer"),
      ReToken(TOKEN_ID) {
   }

public:
   void testReToken() {
      // test constructor values:
      checkEqu(TOKEN_ID, tokenType());
      checkEqu(0, m_value.m_id);
      checkT(m_string.isEmpty());
      checkT(m_printableString.isEmpty());
      m_value.m_id = 7422;
      checkEqu(7422, ReToken::id());
      m_string = "Wow!";
      checkEqu("Wow!", ReToken::toString());
      m_printableString = "GooGoo";
      checkEqu("GooGoo", rawString());
      m_tokenType = TOKEN_NUMBER;
      checkEqu(TOKEN_NUMBER, tokenType());
      clear();
      checkEqu(TOKEN_UNDEF, tokenType());
      checkEqu(0, m_value.m_id);
      checkT(m_string.isEmpty());
      checkT(m_printableString.isEmpty());
      m_value.m_integer = 773322;
      checkEqu(773322, asInteger());
      m_value.m_real = 0.25;
      checkEqu(0.25, asReal());
   }

   ReToken* checkToken(ReToken* token, RplTokenType type, int id = 0,
                       const char* string = NULL) {
      checkEqu(type, token->tokenType());
      if (id != 0)
         checkEqu(id, token->id());
      if (string != NULL)
         checkEqu(string, token->toString());
      return token;
   }
   enum {
      KEY_UNDEF,
      KEY_IF,
      KEY_THEN,
      KEY_ELSE,
      KEY_FI
   };
#   define KEYWORDS "if then else fi"
   enum {
      OP_UNDEF,
      OP_PLUS,
      OP_TIMES,
      OP_DIV,
      OP_GT,
      OP_LT,
      OP_GE,
      OP_LE,
      OP_EQ,
      OP_ASSIGN,
      OP_PLUS_ASSIGN,
      OP_DIV_ASSIGN,
      OP_TIMES_ASSIGN
   };
#   define OPERATORS "+\n* /\n> < >= <= ==\n= += /= *="
   enum {
      COMMENT_UNDEF,
      COMMENT_1,
      COMMENT_MULTILINE,
      COMMENT_2
   };
#   define COMMENTS "/* */ // \n"
   void testSpace() {
      ReSource source;
      ReStringReader reader(source);
#       define BLANKS1 "\t\t   \n"
#       define BLANKS2 " \n"
      reader.addSource("<main>", BLANKS1 BLANKS2);
      source.addReader(&reader);
      ReLexer lex(&source, KEYWORDS, OPERATORS, "=", COMMENTS, "A-Za-z_",
                  "A-Za-z0-9_", ReLexer::NUMTYPE_DECIMAL, ReLexer::SF_TICK,
                  ReLexer::STORE_ALL);
      checkToken(lex.nextToken(), TOKEN_SPACE, 0, BLANKS1);
      checkToken(lex.nextToken(), TOKEN_SPACE, 0, BLANKS2);
   }
   void testNumeric() {
      ReSource source;
      ReStringReader reader(source);
      const char* blanks = "321 0x73 7.8e+5";
      reader.addSource("<main>", blanks);
      source.addReader(&reader);
      ReLexer lex(&source, KEYWORDS, OPERATORS, "=", COMMENTS, "A-Za-z_",
                  "A-Za-z0-9_", ReLexer::NUMTYPE_ALL, ReLexer::SF_TICK,
                  ReLexer::STORE_ALL);
      ReToken* token = checkToken(lex.nextToken(), TOKEN_NUMBER);
      checkEqu(321, token->asInteger());
      token = checkToken(lex.nextNonSpaceToken(), TOKEN_NUMBER);
      checkEqu(0x73, token->asInteger());
      token = checkToken(lex.nextNonSpaceToken(), TOKEN_REAL);
      checkEqu(7.8e+5, token->asReal());
   }

   void testOperators() {
      ReSource source;
      ReStringReader reader(source);
      const char* ops = "<< < <<< <= == = ( ) [ ]";
      reader.addSource("<main>", ops);
      source.addReader(&reader);
      enum {
         UNDEF,
         SHIFT,
         LT,
         SHIFT2,
         LE,
         EQ,
         ASSIGN,
         LPARENT,
         RPARENT,
         LBRACKET,
         RBRACKET
      };
      ReLexer lex(&source, KEYWORDS, ops, "=", COMMENTS, "A-Za-z_",
                  "A-Za-z0-9_", ReLexer::NUMTYPE_ALL, ReLexer::SF_TICK,
                  ReLexer::STORE_ALL);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, SHIFT);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, LT);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, SHIFT2);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, LE);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, EQ);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, ASSIGN);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, LPARENT);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, RPARENT);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, LBRACKET);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, RBRACKET);
      checkToken(lex.nextNonSpaceToken(), TOKEN_END_OF_SOURCE);
      reader.addSource("<buffer2>", "(([[");
      lex.startUnit("<buffer2>");
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, LPARENT);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, LPARENT);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, LBRACKET);
      checkToken(lex.nextNonSpaceToken(), TOKEN_OPERATOR, LBRACKET);
      checkToken(lex.nextNonSpaceToken(), TOKEN_END_OF_SOURCE);
   }

   void testComments() {
      ReSource source;
      ReStringReader reader(source);
      reader.addSource("<main>", "/**/9//\n8/***/7// wow\n/*\n*\n*\n**/");
      source.addReader(&reader);
      enum {
         COMMENT_UNDEF,
         COMMENT_MULTILINE,
         COMMENT_1
      };
      ReLexer lex(&source, KEYWORDS, OPERATORS, "=", COMMENTS, "A-Za-z_",
                  "A-Za-z0-9_", ReLexer::NUMTYPE_ALL, ReLexer::SF_LIKE_C,
                  ReLexer::STORE_ALL);
      checkToken(lex.nextToken(), TOKEN_COMMENT_START, COMMENT_MULTILINE,
                 "/**/");
      checkToken(lex.nextToken(), TOKEN_NUMBER);
      checkToken(lex.nextToken(), TOKEN_COMMENT_START, COMMENT_1, "//\n");
      checkToken(lex.nextToken(), TOKEN_NUMBER);
      checkToken(lex.nextToken(), TOKEN_COMMENT_START, COMMENT_MULTILINE,
                 "/***/");
      checkToken(lex.nextToken(), TOKEN_NUMBER);
      checkToken(lex.nextToken(), TOKEN_COMMENT_START, COMMENT_1, "// wow\n");
      checkToken(lex.nextToken(), TOKEN_COMMENT_START, COMMENT_MULTILINE,
                 "/*\n*\n*\n**/");
   }
   void testStrings() {
      ReSource source;
      ReStringReader reader(source);
      reader.addSource("<main>", "\"abc\\t\\r\\n\\a\\v\"'1\\x9Z\\x21A\\X9'");
      source.addReader(&reader);
      ReLexer lex(&source, KEYWORDS, OPERATORS, "=", COMMENTS, "A-Za-z_",
                  "A-Za-z0-9_", ReLexer::NUMTYPE_ALL, ReLexer::SF_LIKE_C,
                  ReLexer::STORE_ALL);
      checkToken(lex.nextToken(), TOKEN_STRING, '"', "abc\t\r\n\a\v");
      checkToken(lex.nextToken(), TOKEN_STRING, '\'', "1\tZ!A\t");
   }
   void testKeywords() {
      ReSource source;
      ReStringReader reader(source);
      reader.addSource("<main>", "if\n\tthen else\nfi");
      source.addReader(&reader);
      ReLexer lex(&source, KEYWORDS, OPERATORS, "=", COMMENTS, "A-Za-z_",
                  "A-Za-z0-9_", ReLexer::NUMTYPE_ALL, ReLexer::SF_LIKE_C,
                  ReLexer::STORE_ALL);
      checkToken(lex.nextToken(), TOKEN_KEYWORD, KEY_IF);
      checkToken(lex.nextNonSpaceToken(), TOKEN_KEYWORD, KEY_THEN);
      checkToken(lex.nextNonSpaceToken(), TOKEN_KEYWORD, KEY_ELSE);
      checkToken(lex.nextNonSpaceToken(), TOKEN_KEYWORD, KEY_FI);
      checkToken(lex.nextNonSpaceToken(), TOKEN_END_OF_SOURCE);
   }

   void testIds() {
      ReSource source;
      ReStringReader reader(source);
      reader.addSource("<main>", "i\n\tifs\n"
                       "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
      source.addReader(&reader);
      ReLexer lex(&source, KEYWORDS, OPERATORS, "=", COMMENTS, "A-Za-z_",
                  "A-Za-z0-9_", ReLexer::NUMTYPE_ALL, ReLexer::SF_LIKE_C,
                  ReLexer::STORE_ALL);
      checkToken(lex.nextToken(), TOKEN_ID, 0, "i");
      checkToken(lex.nextNonSpaceToken(), TOKEN_ID, 0, "ifs");
      checkToken(lex.nextNonSpaceToken(), TOKEN_ID, 0,
                 "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
   }

   void testBasic() {
      ReSource source;
      ReStringReader reader(source);
      source.addReader(&reader);
      reader.addSource("<main>", "if i>1 then i=1+2*_x9 fi");
      ReLexer lex(&source, KEYWORDS, OPERATORS, "=", COMMENTS, "A-Za-z_",
                  "A-Za-z0-9_", ReLexer::NUMTYPE_ALL, ReLexer::SF_LIKE_C,
                  ReLexer::STORE_ALL);
      ReToken* token;
      checkToken(lex.nextToken(), TOKEN_KEYWORD, KEY_IF);
      checkToken(lex.nextToken(), TOKEN_SPACE, 0);
      checkToken(lex.nextToken(), TOKEN_ID, 0, "i");
      checkToken(lex.nextToken(), TOKEN_OPERATOR, OP_GT);
      token = checkToken(lex.nextToken(), TOKEN_NUMBER);
      checkEqu(1, token->asInteger());
      checkToken(lex.nextToken(), TOKEN_SPACE, 0);
      checkToken(lex.nextToken(), TOKEN_KEYWORD, KEY_THEN);
      checkToken(lex.nextToken(), TOKEN_SPACE, 0);
   }
   void testPrio() {
      ReSource source;
      ReStringReader reader(source);
      source.addReader(&reader);
      reader.addSource("x", "");
      enum {
         O_UNDEF,
         O_ASSIGN,
         O_PLUS,
         O_MINUS,
         O_TIMES,
         O_DIV
      };
      ReLexer lex(&source, KEYWORDS, "=\n+ -\n* /", "=",
                  COMMENTS, "A-Za-z_", "A-Za-z0-9_", ReLexer::NUMTYPE_ALL,
                  ReLexer::SF_LIKE_C, ReLexer::STORE_ALL);
      checkT(lex.prioOfOp(O_ASSIGN) < lex.prioOfOp(O_PLUS));
      checkEqu(lex.prioOfOp(O_PLUS), lex.prioOfOp(O_MINUS));
      checkT(lex.prioOfOp(O_MINUS) < lex.prioOfOp(O_TIMES));
      checkEqu(lex.prioOfOp(O_TIMES), lex.prioOfOp(O_DIV));
   }

   virtual void runTests(void) {
      testPrio();
      testBasic();
      testIds();
      testKeywords();
      testComments();
      testStrings();
      testOperators();
      testNumeric();
      testSpace();
      testReToken();
   }
};
void testReLexer() {
   TestReLexer test;
   test.runTests();
}
