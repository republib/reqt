/*
 * ReMFParser.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REMFPARSER_HPP
#define REMFPARSER_HPP

class ReMFParser: public ReParser {
public:
   enum Keyword {
      K_UNDEF,
      K_IF,
      K_THEN,
      K_ELSE,
      K_FI,
      K_WHILE, // 5
      K_DO,
      K_OD,
      K_REPEAT,
      K_UNTIL,
      K_FOR, // 10
      K_FROM,
      K_TO,
      K_STEP,
      K_IN,
      K_CASE, // 15
      K_OF,
      K_ESAC,
      K_LEAVE,
      K_CONTINUE,
      K_PASS, // 20
      K_CLASS,
      K_ENDC,
      K_ENDF,
      K_FUNCTION,
      K_GENERATOR, // 25
      K_IMPORT,
      K_CONST,
      K_LAZY,
      K_NONE,
      K_TRUE, // 30
      K_FALSE
   };
#define MF_KEYWORDS "if then else fi while do od repeat until" \
    " for from to step in case of esac leave continue pass" \
    " class endc endf func generator import" \
    " const lazy none true false"
   enum Operator {
      O_UNDEF,
      O_SEMI_SEMICOLON,
      O_SEMICOLON,
      O_COMMA,
      O_COLON, // 4
      O_ASSIGN,
      O_PLUS_ASSIGN,
      O_MINUS_ASSIGN,
      O_DIV_ASSIGN,
      O_TIMES_ASSIGN, // 8
      O_MOD_ASSIGN,
      O_POWER_ASSIGN,
      O_OR_ASSIGN,
      O_AND_ASSIGN, // 13
      O_LSHIFT_ASSIGN,
      O_RSHIFT_ASSIGN,
      O_RSHIFT2_ASSIGN, // 16
      O_OR,
      O_AND, // 18
      O_EQ,
      O_NE, // 20
      O_LT,
      O_GT,
      O_LE,
      O_GE, // 24
      O_QUESTION, // 25
      O_PLUS,
      O_MINUS, // 27
      O_DIV,
      O_MOD,
      O_TIMES, // 30
      O_POWER, // 31
      O_XOR,
      O_BIT_OR,
      O_BIT_AND, // 34
      O_LSHIFT,
      O_RSHIFT,
      O_RSHIFT2, // 37
      O_NOT,
      O_BIT_NOT, // 39
      O_INC,
      O_DEC, // 41
      O_DOT,
      O_LPARENTH,
      O_RPARENTH,
      O_LBRACKET,
      O_RBRACKET,
      O_LBRACE,
      O_RBRACE // 48
   };
#define IS_BINARY_OP(op) (Operator(op) >= O_ASSIGN && Operator(op) <= O_DOT)
#define IS_UNARY_OP(op) (op==O_PLUS || op==O_MINUS || (op>=O_NOT && op<=O_DEC))

/// \n separates the priority classes
#define MF_OPERATORS ";; ; , :\n" \
    "= += -= /= *= %= **= |= &= <<= >>= >>>=\n" \
    "||\n" \
    "&&\n" \
    "== !=\n" \
    "< > <= >=\n" \
    "?\n" \
    "+ -\n" \
    "/ % *\n" \
    "**\n" \
    "^ | &\n" \
    "<< >> >>>\n" \
    "! ~\n" \
    "++ --\n" \
    ". ( ) [ ] { }"
#define MF_RIGHT_ASSOCIATIVES "= += -= /= *= %= **= |= &= <<= >>= >>>= ** ."
public:
   ReMFParser(ReSource& source, ReASTree& ast);
public:
   ReASItem* parseIf();
   ReASItem* parseWhile();
   ReASItem* parseRepeat();
   ReASItem* parseFor();
   ReASVarDefinition* parseVarDefinition(ReASNamedValue::Attributes attribute);
   ReASItem* parseExpr(int depth);
   ReASItem* parseBody(Keyword keywordStop, Keyword keywordStop2 = K_UNDEF,
                       int builtinVars = 0);
   void parseMethod();
   void parseClass();
   void parseImport();
   ReASItem* parseModule(ReSourceUnitName name);
   void parse();
   ReASItem* parseExprStatement(bool eatSemicolon = true);
   ReASItem* parseList();
   ReASItem* parseMap();
protected:
   ReASExprStatement* parseArguments();
   ReASItem* parseOperand(int level, ReASItem* parent = NULL);
   ReASVariant* tokenToVariant(ReToken* token, bool endsWithComma,
                               ReASNode1* parent);
   ReASVariant* createFormula(ReASNode1* parent);
   ReASItem* buildVarOrField(const QByteArray& name,
                             const ReSourcePosition* position, ReASItem* parent);
   ReASVarDefinition* parseParameterList();
   ReASItem* parseLocalVar();
   ReASVarDefinition* buildVarDef(ReASNamedValue* var);
protected:
   static ReASBinaryOp::BinOperator convertBinaryOp(int op);
   static ReASUnaryOp::UnaryOp convertUnaryOp(int op);
private:
   ///syntax token builder.
   /// Note: the super class contains a reference with the same name
   ReLexer m_lexer;
};

#endif // REMFPARSER_HPP
