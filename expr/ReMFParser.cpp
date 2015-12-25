/*
 * ReMFParser.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief A parser for the language <b>ML</b>.
 */
/** @file
 * @brief Definition for a parser for the language <b>ML</b>.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

enum MFLocations {
   L_PARSE_OPERAND_RPARENTH = 2001,
   L_PARSE_OPERAND_RPARENTH_INFO,
   L_TERM_WRONG_STRING,
   L_TERM_WRONG_NUMBER,
   L_PARSE_OPERAND_WRONG = 2005,
   L_DEFINITION_NO_ID,
   L_DEFINITION_WRONG_ID,
   L_DEFINITION_UNKNOWN_CLASS,
   L_DEFINITION_MISSING_ID,
   L_DEFINITION_NO_OP = 2010,
   L_DEFINITION_NO_SEMICOLON,
   L_PARSE_IF_NO_THEN,
   L_PARSE_IF_NO_ELSE,
   L_PARSE_IF_NO_FI,
   L_PARSE_WHILE_NO_DO = 2015,
   L_PARSE_WHILE_NO_OD,
   L_PARSE_REPEAT_NO_UNTIL,
   L_PARSE_REPEAT_NO_SEMI,
   L_PARSE_BODY_WRONG_ITEM,
   L_PARSE_FOR_NO_TO = 2020,
   L_PARSE_LIST_NO_COMMA,
   L_PARSE_MAP_BOOL,
   L_PARSE_MAP_NONE,
   L_PARSE_MAP_NUMERIC,
   L_PARSE_MAP_EXPR = 2025,
   L_PARSE_MAP_EXPR2,
   L_PARSE_MAP_NO_COLON,
   L_PARSE_MAP_NO_COMMA,
   L_PARSE_OPERAND_NOT_OPERAND = 2030,
   L_PARSE_BODY_NO_START,
   L_PARSE_OPERAND_NO_BRACKET,
   L_PARSE_ARGS_NO_COMMA_OR_PARENT,
   L_PARSE_OPERAND_NO_FIELD2,
   L_PARSE_OPERAND_NO_BRACKET2 = 2035,
   L_PARSE_OPERAND_NO_FIELD,
   L_PARSE_METH_NO_CLASS,
   L_PARSE_METH_NO_CLASS2,
   L_PARSE_METH_NO_NAME,
   L_PARSE_METH_NO_NAME2 = 2040,
   L_PARSE_METH_NO_LPARENTH,
   L_PARSE_METH_NO_COLON,
   L_PARSE_PARAMLIST_NO_PARENTH,
   L_PARSE_PARAMLIST_NO_PARENTH2,
   L_PARSE_METH_NO_END = 2045,
   L_PARSE_METH_NO_END2,
   L_PARSE_VAR_DEF_ALREADY_DEFINED,
   L_PARSE_VAR_DEF_ALREADY_DEFINED2,
   L_PARSE_CLASS_NO_NAME,
   L_PARSE_CLASS_LOWERCASE = 2050,
   L_PARSE_CLASS_ALREADY_DEFINED,
   L_PARSE_CLASS_ALREADY_DEFINED2

};

/** @class ReMFParser ReMFParser.hpp "expr/ReMFParser.hpp"
 *
 * @brief Implements a parser for the language MF.
 *
 * MF stands for Mathe Fan or Multiple Faces or ....
 * This is an universal object oriented programming language with extension
 * for matrix operations, simulation and graphics.
 */

ReMFParser::ReMFParser(ReSource& source, ReASTree& abstractSyntaxTree) :
   ReParser(m_lexer, abstractSyntaxTree),
   m_lexer(&source,
           MF_KEYWORDS, MF_OPERATORS, MF_RIGHT_ASSOCIATIVES, "/* */ // \n",
           "a-zA-Z_", "a-zA-Z0-9_", ReLexer::NUMTYPE_ALL, ReLexer::SF_LIKE_C) {
}

/**
 * @brief Parses an if statement.
 */
ReASItem* ReMFParser::parseIf() {
   ReASIf* rc = new ReASIf();
   rc->setPosition(m_lexer.currentPosition());
   ReASItem* condition = parseExpr(0);
   if (!m_lexer.currentToken()->isKeyword(K_THEN))
      syntaxError(L_PARSE_IF_NO_THEN, "'then' expected");
   rc->setChild2(condition);
   ReASItem* body = parseBody(K_ELSE, K_FI);
   rc->setChild3(body);
   if (!m_lexer.currentToken()->isKeyword(K_ELSE, K_FI))
      syntaxError(L_PARSE_IF_NO_ELSE, "'else' or 'fi' expected");
   if (m_lexer.currentToken()->isKeyword(K_ELSE)) {
      ReASItem* body = parseBody(K_FI);
      rc->setChild4(body);
   }
   if (!m_lexer.currentToken()->isKeyword(K_FI))
      syntaxError(L_PARSE_IF_NO_FI, "'fi' expected");
   m_lexer.nextNonSpaceToken();
   return rc;
}
/**
 * @brief Parses a while statement.
 */

ReASItem* ReMFParser::parseWhile() {
   ReASWhile* rc = new ReASWhile();
   rc->setPosition(m_lexer.currentPosition());
   ReASItem* condition = parseExpr(0);
   if (!m_lexer.currentToken()->isKeyword(K_DO))
      syntaxError(L_PARSE_WHILE_NO_DO, "'do' expected");
   rc->setChild2(condition);
   ReASItem* body = parseBody(K_OD);
   rc->setChild3(body);
   if (!m_lexer.currentToken()->isKeyword(K_OD))
      syntaxError(L_PARSE_WHILE_NO_OD, "'od'  expected");
   m_lexer.nextNonSpaceToken();
   return rc;
}

/**
 * @brief Parses a repeat statement.
 */
ReASItem* ReMFParser::parseRepeat() {
   ReASRepeat* rc = new ReASRepeat();
   rc->setPosition(m_lexer.currentPosition());
   ReASItem* body = parseBody(K_UNTIL);
   rc->setChild3(body);
   if (!m_lexer.currentToken()->isKeyword(K_UNTIL))
      syntaxError(L_PARSE_REPEAT_NO_UNTIL, "'until'  expected");
   ReASItem* condition = parseExpr(0);
   if (!m_lexer.currentToken()->isOperator(O_SEMICOLON))
      syntaxError(L_PARSE_REPEAT_NO_SEMI, "';' expected");
   rc->setChild2(condition);
   m_lexer.nextNonSpaceToken();
   return rc;
}

/**
 * @brief Creates a variable definition for a builtin variable.
 * @param var   the basic variable data
 * @return
 */
ReASVarDefinition* ReMFParser::buildVarDef(ReASNamedValue* var) {
   ReASVarDefinition* rc = new ReASVarDefinition();
   rc->setPosition(var->position());
   rc->setChild2(var);
   ReSymbolSpace* symbols = m_tree.currentSpace();
   int varNo;
   symbols->addVariable(rc, varNo);
   var->setVariableNo(varNo);
   return rc;
}

/**
 * @brief Parses a for statement.
 *
 * Syntax:
 * for [ VAR ] [ from START_EXPR ] to END_EXPR [ step STEP_EXPR ] do
 * BODY od
 *
 * for VAR in ITERABLE_EXPR do BODY od
 *
 * @post            the token behind the do is read
 * @return          the abstract syntax tree of the for statement
 */
ReASItem* ReMFParser::parseFor() {
   int builtinVars = 1;
   ReASNode2* rc = NULL;
   const ReSourcePosition* startPosition = m_lexer.currentPosition();
   ReToken* token = m_lexer.nextNonSpaceToken();
   ReASNamedValue* var = NULL;
   if (token->isTokenType(TOKEN_ID)) {
      var = new ReASNamedValue(ReASInteger::m_instance, m_tree.currentSpace(),
                               token->toString(), ReASNamedValue::A_LOOP);
      var->setPosition(m_lexer.currentPosition());
      token = m_lexer.nextNonSpaceToken();
   }
   if (token->isKeyword(K_IN)) {
      ReASVarDefinition* varDef = buildVarDef(var);
      ReASForIterated* node = new ReASForIterated(varDef);
      rc = node;
      node->setPosition(startPosition);
      node->setChild3(var);
      ReASItem* iterable = parseExpr(0);
      node->setChild4(iterable);
   } else {
      if (var == NULL) {
         char name[32];
         // Build a unique name inside the scope:
         qsnprintf(name, sizeof name, "$%d_%d", startPosition->lineNo(),
                   startPosition->column());
         var = new ReASNamedValue(ReASInteger::m_instance,
                                  m_tree.currentSpace(), name, ReASNamedValue::A_LOOP);
         var->setPosition(startPosition);
      }
      ReASVarDefinition* varDef = buildVarDef(var);
      ReASForCounted* node = new ReASForCounted(varDef);
      rc = node;
      node->setPosition(startPosition);
      node->setChild3(var);
      if (token->isKeyword(K_FROM)) {
         node->setChild4(parseExpr(0));
      }
      if (!m_lexer.currentToken()->isKeyword(K_TO)) {
         syntaxError(L_PARSE_FOR_NO_TO, "'to' expected");
      }
      node->setChild5(parseExpr(0));
      if (m_lexer.currentToken()->isKeyword(K_STEP)) {
         node->setChild6(parseExpr(0));
      }
   }
   if (!m_lexer.currentToken()->isKeyword(K_DO))
      syntaxError(L_PARSE_FOR_NO_TO, "'to' expected");
   rc->setChild2(parseBody(K_OD, K_UNDEF, builtinVars));
   m_lexer.nextNonSpaceToken();
   return rc;
}

/**
 * @brief Parses a variable definition.
 *
 * Syntax:
 * Variable: { "const" | "lazy" }* TYPE id [ = EXPR ] [ ";" ]
 * Parameter: { "const" | "lazy" }* TYPE id [ = EXPR ] { "," | ")" }
 *
 * @pre             first token of the definition is read
 * @post            token behind the definition is read: ';', ',', ')'
 * @param attribute attribute of the variable: A_PARAM...
 * @return          a variable/parameter definition
 */
ReASVarDefinition* ReMFParser::parseVarDefinition(
   ReASNamedValue::Attributes attribute) {
   int attributes = attribute;
   ReToken* token = m_lexer.currentToken();
   while (token->isKeyword(K_CONST, K_LAZY)) {
      switch (token->id()) {
      case K_CONST:
         attributes += ReASNamedValue::A_CONST;
         break;
      case K_LAZY:
         attributes += ReASNamedValue::A_LAZY;
         break;
      default:
         break;
      }
      token = m_lexer.nextNonSpaceToken();
   }
   if (!token->isTokenType(TOKEN_ID))
      syntaxError(L_DEFINITION_NO_ID, "class name expected, but no id found");
   if (!token->isCapitalizedId())
      syntaxError(L_DEFINITION_WRONG_ID,
                  "a class name must start with an upper case character");
   ReASClass* clazz = m_tree.currentSpace()->findClass(token->toString());
   if (clazz == NULL)
      syntaxError(L_DEFINITION_UNKNOWN_CLASS, "unknown class");
   token = m_lexer.nextNonSpaceToken();
   if (!token->isTokenType(TOKEN_ID))
      syntaxError(L_DEFINITION_MISSING_ID, "variable name expected");
   ReSymbolSpace* symbols = m_tree.currentSpace();
   // freed in the destructor of the nodes:
   ReASNamedValue* namedValue = new ReASNamedValue(clazz, symbols,
         token->toString(), attributes);
   namedValue->setPosition(m_lexer.currentPosition());
   ReASVarDefinition* rc = new ReASVarDefinition();
   rc->setPosition(m_lexer.currentPosition());
   rc->setChild2(namedValue);
   token = m_lexer.nextNonSpaceToken();
   if (token->id() == O_ASSIGN) {
      ReASItem* value = parseExpr(0);
      rc->setChild3(value);
      token = m_lexer.currentToken();
   }
   int varNo = 0;
   ReASItem* oldSymbol = symbols->addVariable(rc, varNo);
   if (oldSymbol != NULL)
      error(L_PARSE_VAR_DEF_ALREADY_DEFINED, oldSymbol->position(),
            "symbol already defined", "previous definition");
   namedValue->setVariableNo(varNo);
   return rc;
}

/**
 * @brief Reads the current tokens as an formula and returns the variant.
 *
 * @post            the token behind the formula is read
 * @param parent    the parent for the formula: because of the destroying
 *                  the new expression is chained into the parent
 * @return          the variant containing the formula
 */
ReASVariant* ReMFParser::createFormula(ReASNode1* parent) {
   ReASVariant* variant = NULL;
   m_lexer.undoLastToken2();
   ReASExprStatement* expr =
      dynamic_cast<ReASExprStatement*>(parseExprStatement(false));
   if (expr != NULL) {
      // chaining per m_child (= next statement) is for freeing while destruction:
      expr->setChild(parent->child());
      parent->setChild(expr);
      // freed in the destructor of varList (~ReASVariant()):
      variant = new ReASVariant();
      variant->setObject(expr, ReASFormula::m_instance);
   }
   return variant;
}

/**
 * @brief Converts the current expression into a <code>ReASVariant</code>.
 *
 * If the expression is a constant, the constant value will be the content
 * of the variant. Otherwise a formula will be stored.
 *
 * @pre                 the first token of the variant expression is read
 * @post                the token behind the variant expression is read
 * @param token         the token to convert
 * @param endsWithComma true: the 2nd token is a ','
 * @param parent        the parent node of the expression
 * @return              the variant with the token's content
 */
ReASVariant* ReMFParser::tokenToVariant(ReToken* token, bool endsWithComma,
                                        ReASNode1* parent) {
   ReASVariant* variant = NULL;
   if (endsWithComma) {
      switch (token->tokenType()) {
      case TOKEN_NUMBER:
         // freed in the destructor of varList (~ReASVariant()):
         variant = new ReASVariant();
         variant->setInt(token->asInteger());
         break;
      case TOKEN_STRING:
         // freed in the destructor of varList (~ReASVariant()):
         variant = new ReASVariant();
         variant->setString(token->toString());
         break;
      case TOKEN_REAL:
         // freed in the destructor of varList (~ReASVariant()):
         variant = new ReASVariant();
         variant->setFloat(token->asReal());
         break;
      case TOKEN_KEYWORD:
         switch (token->id()) {
         case K_TRUE:
         case K_FALSE:
            // freed in the destructor of varList (~ReASVariant()):
            variant = new ReASVariant();
            variant->setBool(token->id() == K_TRUE);
            break;
         case K_NONE:
            // freed in the destructor of varList (~ReASVariant()):
            variant = new ReASVariant();
            break;
         default:
            break;
         }
         break;
      default:
         break;
      }
   }
   if (variant == NULL)
      variant = createFormula(parent);
   return variant;
}

/**
 * @brief Parses a list.
 *
 * Syntax:<br>
 * '[' [ EXPR_1 [ ',' EXPR_2 ...]] ']'
 *
 * @pre    '[' is the current token
 * @post   the token behind the ']' is read
 * @return a node of the abstract syntax tree
 */
ReASItem* ReMFParser::parseList() {
   ReASListConstant* rc = new ReASListConstant();
   ReASVariant& varList = rc->value();
   ReASListOfVariants* list =
      static_cast<ReASListOfVariants*>(varList.asObject(NULL));
   rc->setPosition(m_lexer.currentPosition());
   ReASVariant* variant;
   bool again = true;
   ReToken* token;
   ReToken* token2;
   // read the token behind '[':
   token = m_lexer.nextNonSpaceToken();
   if (token->isOperator(O_RBRACKET)) {
      // read token behind ']':
      m_lexer.nextNonSpaceToken();
   } else {
      while (again) {
         m_lexer.saveLastToken();
         token2 = m_lexer.nextNonSpaceToken();
         variant = tokenToVariant(token, token2->isOperator(O_COMMA), rc);
         token = m_lexer.currentToken();
         if (token->isOperator(O_RBRACKET))
            again = false;
         else if (!token->isOperator(O_COMMA))
            syntaxError(L_PARSE_LIST_NO_COMMA, "',' or ']' expected");
         // read token behind ',' or ']'
         token = m_lexer.nextNonSpaceToken();
         if (variant != NULL)
            list->append(variant);
      }
   }
   return rc;
}

/**
 * @brief Parses a map.
 *
 * Syntax:<br>
 * '{' [ STRING_EXPR_1 ':' EXPR_1 [ ',' STRING_EXPR_2 ': EXPR_2 ...]] '}'
 *
 * @pre    '{' is the current token
 * @post   the token behind the '}' is read
 * @return a node of the abstract syntax tree
 */
ReASItem* ReMFParser::parseMap() {
   ReASMapConstant* rc = new ReASMapConstant();
   ReASVariant& varMap = rc->value();
   ReASMapOfVariants* map = static_cast<ReASMapOfVariants*>(varMap.asObject(
                               NULL));
   rc->setPosition(m_lexer.currentPosition());
   ReASVariant* variant;
   bool again = true;
   ReToken* token;
   ReToken* token2;
   QByteArray key;
   while (again) {
      token = m_lexer.nextNonSpaceToken();
      if (token->isOperator(O_RBRACE))
         again = false;
      else {
         key.clear();
         switch (token->tokenType()) {
         case TOKEN_STRING:
            // freed in the destructor of varList (~ReASVariant()):
            key = token->toString();
            break;
         case TOKEN_KEYWORD:
            switch (token->id()) {
            case K_TRUE:
            case K_FALSE:
               syntaxError(L_PARSE_MAP_BOOL,
                           "boolean value not allowed as key type. Use a string");
               break;
            case K_NONE:
               syntaxError(L_PARSE_MAP_NONE,
                           "'none' is  not allowed as key type. Use a string");
               break;
            default:
               syntaxError(L_PARSE_MAP_EXPR,
                           "a non constant expression is  not allowed as key type. Use a string");
               break;
            }
            break;
         case TOKEN_NUMBER:
         case TOKEN_REAL:
            syntaxError(L_PARSE_MAP_NUMERIC,
                        "numeric values not allowed as key type. Use a string");
            break;
         default:
            syntaxError(L_PARSE_MAP_EXPR2,
                        "a non constant expression is  not allowed as key type. Use a string");
            break;
         }
         token = m_lexer.nextNonSpaceToken();
         if (!token->isOperator(O_COLON)) {
            syntaxError(L_PARSE_MAP_NO_COLON, "':' expected");
         } else {
            token = m_lexer.nextNonSpaceToken();
            m_lexer.saveLastToken();
            token2 = m_lexer.nextNonSpaceToken();
            variant = tokenToVariant(token, token2->isOperator(O_COMMA),
                                     rc);
            (*map)[key] = variant;
            variant = NULL;
            token = m_lexer.currentToken();
            if (token->isOperator(O_RBRACE))
               again = false;
            else if (!token->isOperator(O_COMMA))
               syntaxError(L_PARSE_MAP_NO_COMMA, "',' or '}' expected");
         }
      }
   }
   m_lexer.nextNonSpaceToken();
   return rc;
}

/**
 * @brief Builds a node of a variable or a field (as an operand).
 *
 * @param name      name of the variable/field
 * @param position  source position
 * @param parent    NULL: result is a variable
 *
 * @return          a <code>ReASNamedValue</code> or a
 *                  <code>ReASNamedValue</code> instance
 */
ReASItem* ReMFParser::buildVarOrField(const QByteArray& name,
                                      const ReSourcePosition* position, ReASItem* parent) {
   ReASItem* rc = NULL;
   if (parent == NULL) {
      ReSymbolSpace* space = m_tree.currentSpace();
      ReASVarDefinition* var = space->findVariable(name);
      ReASClass* clazz = NULL;
      if (var != NULL)
         clazz = var->clazz();
      ReASNamedValue* var2 = new ReASNamedValue(clazz, space, name,
            ReASNamedValue::A_NONE);
      var2->setPosition(position);
      rc = var2;
   } else {
      ReASField* field = new ReASField(name);
      field->setPosition(position);
      rc = field;
      field->setChild(parent);
   }
   return rc;
}
/**
 * @brief Converts a MF specific unary operator into one of AST.
 *
 * @param op    operator known by MF parser
 *
 * @return      operator known by the abstract syntax tree
 */
ReASUnaryOp::UnaryOp ReMFParser::convertUnaryOp(int op) {
   ReASUnaryOp::UnaryOp rc;
   switch (op) {
   case O_PLUS:
      rc = ReASUnaryOp::UOP_PLUS;
      break;
   case O_MINUS:
      rc = ReASUnaryOp::UOP_MINUS_INT;
      break;
   case O_NOT:
      rc = ReASUnaryOp::UOP_NOT_BOOL;
      break;
   case O_BIT_NOT:
      rc = ReASUnaryOp::UOP_NOT_INT;
      break;
   case O_INC:
      rc = ReASUnaryOp::UOP_INC;
      break;
   case O_DEC:
      rc = ReASUnaryOp::UOP_DEC;
      break;
   default:
      throw ReException("unknown unary operator %d", op);
      break;
   }
   return rc;
}

/**
 * @brief Parses an operand.
 *
 * An operand is the first and the third part of a binary operation.
 * Examples: constant, variable, method call, an expression in parentheses
 *
 * @post    the token behind the operand is read
 * @return  the node with the operand
 */
ReASItem* ReMFParser::parseOperand(int level, ReASItem* parent) {
   ReToken* token = m_lexer.nextNonSpaceToken();
   const ReSourcePosition* startPosition = m_lexer.currentPosition();
   ReASItem* rc = NULL;
   bool readNext = true;
   switch (token->tokenType()) {
   case TOKEN_OPERATOR: {
      Operator opId = (Operator) token->id();
      if (parent != NULL && opId != O_LBRACKET)
         syntaxError(L_PARSE_OPERAND_NO_FIELD,
                     "field expected (behind a '.')");
      if (opId == O_LBRACKET) {
         if (parent == NULL) {
            rc = parseList();
            readNext = false;
         } else {
            ReASIndexedValue* value = new ReASIndexedValue();
            value->setPosition(startPosition);
            value->setChild(parent);
            rc = value;
            value->setChild2(parseExpr(level + 1));
            if (!m_lexer.currentToken()->isOperator(O_RBRACKET))
               syntaxError(L_PARSE_OPERAND_NO_BRACKET2, "']' expected");
         }
      } else if (opId == O_LBRACE) {
         rc = parseMap();
         readNext = false;
      } else if (opId == O_LPARENTH) {
         rc = parseExpr(level + 1);
         token = m_lexer.currentToken();
         if (!token->isOperator(O_RPARENTH)) {
            // this call never comes back (exception!)
            syntaxError(L_PARSE_OPERAND_RPARENTH, "')' expected", "(",
                        startPosition);
         }
      } else if (IS_UNARY_OP(opId)) {
         ReASUnaryOp* op = new ReASUnaryOp(convertUnaryOp(token->id()),
                                           AST_PRE_UNARY_OP);
         op->setPosition(m_lexer.currentPosition());
         op->setChild(parseOperand(level));
         readNext = false;
         rc = op;
      } else
         syntaxError(L_PARSE_OPERAND_NOT_OPERAND,
                     "operand expected, not an operator");
      break;
   }
   case TOKEN_STRING:
   case TOKEN_NUMBER:
   case TOKEN_REAL: {
      if (parent != NULL)
         syntaxError(L_PARSE_OPERAND_NO_FIELD2,
                     "field expected (behind a '.')");
      ReASConstant* constant = new ReASConstant();
      constant->setPosition(m_lexer.currentPosition());
      rc = constant;
      switch (token->tokenType()) {
      case TOKEN_STRING:
         constant->value().setString(token->toString());
         break;
      case TOKEN_NUMBER:
         constant->value().setInt(token->asInteger());
         break;
      case TOKEN_REAL:
         constant->value().setFloat(token->asReal());
         break;
      default:
         break;
      }
      break;
   }
   case TOKEN_ID: {
      QByteArray name = token->toString();
      if (name == "a")
         name = "a";
      token = m_lexer.nextNonSpaceToken();
      startPosition = m_lexer.currentPosition();
      if (token->tokenType() != TOKEN_OPERATOR) {
         rc = buildVarOrField(name, startPosition, parent);
         readNext = false;
      } else {
         if (token->id() == O_LPARENTH) {
            ReASMethodCall* call = new ReASMethodCall(name, parent);
            call->setPosition(startPosition);
            rc = call;
            token = m_lexer.nextNonSpaceToken();
            if (!token->isOperator(O_RPARENTH)) {
               m_lexer.undoLastToken();
               ReASExprStatement* args = parseArguments();
               call->setChild2(args);
               readNext = false;
            }
         } else {
            rc = buildVarOrField(name, startPosition, parent);
            if (token->id() == O_LBRACKET) {
               ReASItem* indexExpr = parseExpr(0);
               if (!m_lexer.currentToken()->isOperator(O_RBRACKET))
                  syntaxError(L_PARSE_OPERAND_NO_BRACKET, "']' expected");
               dynamic_cast<ReASNode1*>(rc)->setChild(indexExpr);
            } else {
               if (token->id() == O_INC || token->id() == O_DEC) {
                  ReASUnaryOp* op = new ReASUnaryOp(
                     convertUnaryOp(token->id()), AST_POST_UNARY_OP);
                  op->setChild(rc);
                  rc = op;
               } else {
                  readNext = false;
               }
            }
         }
      }
      break;
   }
   case TOKEN_END_OF_SOURCE:
      readNext = false;
      break;
   default:
      // this call never comes back (exception!)
      syntaxError(L_PARSE_OPERAND_WRONG,
                  "unexpected symbol detected. Operand expected");
      break;
   }
   if (readNext)
      m_lexer.nextNonSpaceToken();
   if (m_lexer.currentToken()->isOperator(O_DOT, O_LBRACKET)) {
      if (m_lexer.currentToken()->asInteger() == O_LBRACKET)
         m_lexer.undoLastToken();
      rc = parseOperand(level, rc);
   }
   return rc;
}
/**
 * @brief Converts a MF specific binary operator into one of AST.
 *
 * @param op    operator known by MF parser
 *
 * @return      operator known by the abstract syntax tree
 */
ReASBinaryOp::BinOperator ReMFParser::convertBinaryOp(int op) {
   ReASBinaryOp::BinOperator rc;
   switch (op) {
   case ReMFParser::O_ASSIGN:
      rc = ReASBinaryOp::BOP_ASSIGN;
      break;
   case ReMFParser::O_PLUS_ASSIGN:
      rc = ReASBinaryOp::BOP_PLUS_ASSIGN;
      break;
   case ReMFParser::O_MINUS_ASSIGN:
      rc = ReASBinaryOp::BOP_MINUS_ASSIGN;
      break;
   case ReMFParser::O_DIV_ASSIGN:
      rc = ReASBinaryOp::BOP_DIV_ASSIGN;
      break;
   case ReMFParser::O_TIMES_ASSIGN:
      rc = ReASBinaryOp::BOP_TIMES_ASSIGN;
      break;
   case ReMFParser::O_MOD_ASSIGN:
      rc = ReASBinaryOp::BOP_MOD_ASSIGN;
      break;
   case ReMFParser::O_POWER_ASSIGN:
      rc = ReASBinaryOp::BOP_POWER_ASSIGN;
      break;
   case ReMFParser::O_OR_ASSIGN:
      rc = ReASBinaryOp::BOP_LOG_OR_ASSIGN;
      break;
   case ReMFParser::O_AND_ASSIGN:
      rc = ReASBinaryOp::BOP_LOG_AND_ASSIGN;
      break;
   case ReMFParser::O_LSHIFT_ASSIGN:
      rc = ReASBinaryOp::BOP_LSHIFT_ASSIGN;
      break;
   case ReMFParser::O_RSHIFT_ASSIGN:
      rc = ReASBinaryOp::BOP_LOG_RSHIFT_ASSIGN;
      break;
   case ReMFParser::O_RSHIFT2_ASSIGN:
      rc = ReASBinaryOp::BOP_ARTITH_RSHIFT_ASSIGN;
      break;
   case ReMFParser::O_OR:
      rc = ReASBinaryOp::BOP_LOG_OR;
      break;
   case ReMFParser::O_AND:
      rc = ReASBinaryOp::BOP_LOG_AND;
      break;
   case ReMFParser::O_EQ:
      rc = ReASBinaryOp::BOP_EQ;
      break;
   case ReMFParser::O_NE:
      rc = ReASBinaryOp::BOP_NE;
      break;
   case ReMFParser::O_LT:
      rc = ReASBinaryOp::BOP_LT;
      break;
   case ReMFParser::O_GT:
      rc = ReASBinaryOp::BOP_GT;
      break;
   case ReMFParser::O_LE:
      rc = ReASBinaryOp::BOP_LE;
      break;
   case ReMFParser::O_GE:
      rc = ReASBinaryOp::BOP_GE;
      break;
   case ReMFParser::O_PLUS:
      rc = ReASBinaryOp::BOP_PLUS;
      break;
   case ReMFParser::O_MINUS:
      rc = ReASBinaryOp::BOP_MINUS;
      break;
   case ReMFParser::O_DIV:
      rc = ReASBinaryOp::BOP_DIV;
      break;
   case ReMFParser::O_MOD:
      rc = ReASBinaryOp::BOP_MOD;
      break;
   case ReMFParser::O_TIMES:
      rc = ReASBinaryOp::BOP_TIMES;
      break;
   case ReMFParser::O_POWER:
      rc = ReASBinaryOp::BOP_POWER;
      break;
   case ReMFParser::O_XOR:
      rc = ReASBinaryOp::BOP_LOG_XOR;
      break;
   case ReMFParser::O_BIT_OR:
      rc = ReASBinaryOp::BOP_BIT_OR;
      break;
   case ReMFParser::O_BIT_AND:
      rc = ReASBinaryOp::BOP_BIT_AND;
      break;
   case ReMFParser::O_LSHIFT:
      rc = ReASBinaryOp::BOP_LSHIFT;
      break;
   case ReMFParser::O_RSHIFT:
      rc = ReASBinaryOp::BOP_LOG_RSHIFT;
      break;
   case ReMFParser::O_RSHIFT2:
      rc = ReASBinaryOp::BOP_ARTITH_RSHIFT;
      break;
   default:
      throw ReException("unknown binary operator %d", op);
      break;
   }
   return rc;
}

/**
 * @brief Parses an expression.
 *
 * This method parses the part of an expression with the same parenthesis level.
 * The nested parts will be processed recursivly by calling parseOperand which
 * calls <code>parseExpr</code> in the case of inner parentheses.
 *
 * Example: a + (3 * 7 - 2)<br>
 * expr with level 1: 3*7-2<br>
 * expr with level 0: a + expr1<br>
 *
 * @pre         the nextNonSpaceToken() will return the first token of the expr.
 * @post        the token behind the expression is read
 *
 * @param depth the level of the parenthesis
 * @return      the abstract syntax tree representing the parsed expression
 */
ReASItem* ReMFParser::parseExpr(int depth) {
   ReToken* token;
   ReASItem* top = parseOperand(depth);
   if (top != NULL) {
      int lastPrio = INT_MAX;
      bool again = true;
      do {
         token = m_lexer.currentToken();
         switch (token->tokenType()) {
         case TOKEN_OPERATOR: {
            Operator opId = (Operator) token->id();
            if (IS_BINARY_OP(opId)) {
               ReASBinaryOp* op = new ReASBinaryOp();
               op->setPosition(m_lexer.currentPosition());
               op->setOperator(convertBinaryOp(opId));
               int prio = m_lexer.prioOfOp(token->id());
               if (prio < lastPrio
                     || (prio == lastPrio
                         && !m_lexer.isRightAssociative(opId))) {
                  op->setChild(top);
                  top = op;
               } else {
                  // right assoc or higher priority:
                  ReASBinaryOp* top2 = dynamic_cast<ReASBinaryOp*>(top);
                  op->setChild(top2->child2());
                  top2->setChild2(op);
               }
               lastPrio = prio;
               op->setChild2(parseOperand(depth));
            } else
               again = false;
            break;
         }
         case TOKEN_STRING:
            syntaxError(L_TERM_WRONG_STRING,
                        "Operator expected, not a string");
            break;
         case TOKEN_NUMBER:
         case TOKEN_REAL:
            syntaxError(L_TERM_WRONG_NUMBER,
                        "Operator expected, not a number");
            break;
         case TOKEN_KEYWORD:
         case TOKEN_ID:
         case TOKEN_END_OF_SOURCE:
         default:
            again = false;
            break;
         }
      } while (again);
   }
   return top;
}

/**
 * @brief Parses an expression as a statement.
 *
 * @pre     the nextNonSpaceToken() will return the first token of the expr.<br>
 *          Note: a ';' is part of the expression statement
 * @post    the token behind the expression is read
 * @param   eatSemicolon    true: a trailing ';' will be read
 * @return                  the abstract syntax tree of the expression statement
 */
ReASItem* ReMFParser::parseExprStatement(bool eatSemicolon) {
   ReASItem* item = parseExpr(0);
   ReASExprStatement* statement = NULL;
   if (item != NULL) {
      statement = new ReASExprStatement();
      statement->setPosition(item->position());
      statement->setChild2(item);
   }
   if (eatSemicolon && m_lexer.currentToken()->isOperator(O_SEMICOLON))
      m_lexer.nextNonSpaceToken();
   return statement;
}

/**
 * @brief Parses a local variable.
 *
 * @return the variable definition
 */
ReASItem* ReMFParser::parseLocalVar() {
   ReASItem* rc = parseVarDefinition(ReASNamedValue::A_NONE);
   return rc;
}

/**
 * @brief Parses the body.
 *
 * A body is a module, a class region or a method body.
 *
 * @param keywordStop   the first possible keyword which finishes the stm. list
 * @param keywordStop2  the 2nd possible keyword which finishes the statements
 * @param builtinVars   number of variables valid only in this body
 * @return              the first element of the statement list
 */
ReASItem* ReMFParser::parseBody(Keyword keywordStop, Keyword keywordStop2,
                                int builtinVars) {
   ReToken* token = m_lexer.nextNonSpaceToken();
   ReASItem* item = NULL;
   ReASItem* body = NULL;
   ReASNode1* lastStatement = NULL;
   ReASScope scope;
   m_tree.currentSpace()->startScope(scope);
   scope.m_builtInVars = builtinVars;
   bool again = true;
   const ReSourcePosition* lastPos = NULL;
   do {
      token = m_lexer.currentToken();
      if (lastPos == m_lexer.currentPosition())
         syntaxError(L_PARSE_BODY_NO_START,
                     "no statement starts with this symbol");
      lastPos = m_lexer.currentPosition();
      // eat a superflous ';'
      if (token->isOperator(O_SEMICOLON))
         token = m_lexer.nextNonSpaceToken();
      try {
         switch (token->tokenType()) {
         case TOKEN_OPERATOR:
         case TOKEN_STRING:
         case TOKEN_NUMBER:
         case TOKEN_REAL:
            m_lexer.undoLastToken();
            item = parseExprStatement();
            break;
         case TOKEN_KEYWORD:
            switch (token->id()) {
            case K_IF:
               item = parseIf();
               break;
            case K_WHILE:
               item = parseWhile();
               break;
            case K_REPEAT:
               item = parseRepeat();
               break;
            case K_FOR:
               item = parseFor();
               break;
            case K_CLASS:
               parseClass();
               item = NULL;
               break;
            case K_FUNCTION:
            case K_GENERATOR:
               parseMethod();
               item = NULL;
               break;
            case K_IMPORT:
               parseImport();
               item = NULL;
               break;
            case K_CONST:
            case K_LAZY:
               item = parseLocalVar();
               break;
            default:
               if (token->isKeyword(keywordStop, keywordStop2))
                  again = false;
               break;
            }
            break;
         case TOKEN_ID: {
            if (token->isCapitalizedId()) {
               item = parseLocalVar();
            } else {
               m_lexer.undoLastToken();
               item = parseExprStatement();
            }
            break;
         }
         case TOKEN_END_OF_SOURCE:
            again = false;
            break;
         default:
            break;
         }
         if (again && item != NULL) {
            if (body == NULL) {
               body = item;
            } else {
               lastStatement->setChild(item);
            }
            lastStatement = dynamic_cast<ReASNode1*>(item);
            if (lastStatement == NULL)
               error(L_PARSE_BODY_WRONG_ITEM, "wrong item type: %d",
                     item == NULL ? 0 : item->nodeType());
            token = m_lexer.currentToken();
            if (keywordStop != K_UNDEF
                  && token->isKeyword(keywordStop, keywordStop2))
               again = false;
         }
      } catch (ReSyntaxError exc) {
         // we look for the end of the statement:
         token = m_lexer.currentToken();
         RplTokenType type;
         Operator op;
         Keyword key;
         while ((type = token->tokenType()) != TOKEN_END_OF_SOURCE)
            if (type == TOKEN_OPERATOR
                  && ((op = Operator(token->id())) == O_SEMICOLON
                      || op == O_SEMI_SEMICOLON))
               break;
            else if (type == TOKEN_KEYWORD) {
               key = Keyword(token->id());
               if (key == K_ENDC || key == K_ENDF) {
                  // we need the token again!
                  m_lexer.undoLastToken();
                  break;
               } else if (key == K_FI || key == K_OD) {
                  break;
               } else {
                  token = m_lexer.nextNonSpaceToken();
               }
            } else {
               token = m_lexer.nextNonSpaceToken();
            }
      }
   } while (again);
   if (keywordStop != K_ENDF && keywordStop != K_ENDC
         && keywordStop != K_UNDEF)
      m_tree.currentSpace()->finishScope(m_lexer.currentPosition()->lineNo(),
                                         scope);
   return body;
}

/**
 * @brief Parses a parameter list of a method/function.
 *
 * @pre     token behind '(' is read
 * @post    token behind ')' is read
 * @return
 */
ReASVarDefinition* ReMFParser::parseParameterList() {
   ReASVarDefinition* rc = NULL;
   ReASVarDefinition* last = NULL;
   const ReSourcePosition* startPos = m_lexer.currentPosition();
   ReASItem* definition = NULL;
   do {
      if (definition != NULL)
         m_lexer.nextNonSpaceToken();
      ReASVarDefinition* current = parseVarDefinition(
                                      ReASNamedValue::A_PARAM);
      if (rc == NULL) {
         rc = current;
      } else {
         last->setChild(current);
      }
      last = current;
   } while (m_lexer.currentToken()->isOperator(O_COMMA));
   if (!m_lexer.currentToken()->isOperator(O_RPARENTH))
      syntaxError(L_PARSE_PARAMLIST_NO_PARENTH, ") expected", ")", startPos);
   m_lexer.nextNonSpaceToken();
   return rc;
}

/**
 * @brief Parses a class definition.
 *
 * The method definition will be stored in the symbol space of the parent
 *
 * @pre             token "func" is read
 * @post            token behind "endf" is read
 * @return          NULL
 */
void ReMFParser::parseMethod() {
   ReASMethod* method = NULL;
   const ReSourcePosition* startPos = m_lexer.currentPosition();
   ReToken* token = m_lexer.nextNonSpaceToken();
   if (!token->isTokenType(TOKEN_ID))
      syntaxError(L_PARSE_METH_NO_CLASS, "type name expected");
   QByteArray type = token->toString();
   if (!isupper(type.at(0)))
      syntaxError(L_PARSE_METH_NO_CLASS2,
                  "type name expected (must start with an upper case character)");
   token = m_lexer.nextNonSpaceToken();
   if (!token->isTokenType(TOKEN_ID))
      syntaxError(L_PARSE_METH_NO_NAME, "method name expected");
   QByteArray name = token->toString();
   if (!isupper(type.at(0)))
      syntaxError(L_PARSE_METH_NO_CLASS2,
                  "method name expected (must start with an lower case character)");
   token = m_lexer.nextNonSpaceToken();
   if (!token->isOperator(O_LPARENTH, O_COLON))
      syntaxError(L_PARSE_METH_NO_LPARENTH, "'(' or ':' expected");
   ReASVarDefinition* parameterList = NULL;
   method = new ReASMethod(name, m_tree);
   method->setPosition(startPos);
   ReSymbolSpace* symbols = m_tree.currentSpace();
   symbols->addMethod(method);
   m_tree.startClassOrMethod(name, ReSymbolSpace::SST_METHOD);
   method->setSymbols();
   if (token->isOperator(O_LPARENTH)) {
      token = m_lexer.nextNonSpaceToken();
      if (token->isOperator(O_RPARENTH)) {
         token = m_lexer.nextNonSpaceToken();
      } else {
         parameterList = parseParameterList();
         method->setChild2(parameterList);
      }
   }
   if (!token->isOperator(O_COLON))
      syntaxError(L_PARSE_METH_NO_COLON, "':' expected");
   method->setChild(parseBody(K_ENDF));
   if (!m_lexer.currentToken()->isKeyword(K_ENDF))
      syntaxError(L_PARSE_METH_NO_END, "end of function not found", "endf",
                  startPos);
   m_lexer.nextNonSpaceToken();
   m_tree.finishClassOrMethod(name);
}

/**
 * @brief Parses a class definition.
 *
 * @pre     "class" is read
 * @post    token behind "endc" is read
 */
void ReMFParser::parseClass() {
   const ReSourcePosition* startPos = m_lexer.currentPosition();
   ReToken* token = m_lexer.nextNonSpaceToken();
   if (!token->isTokenType(TOKEN_ID))
      syntaxError(L_PARSE_CLASS_NO_NAME, "class name expected");
   if (!token->isCapitalizedId())
      syntaxError(L_PARSE_CLASS_LOWERCASE,
                  "class name must start with an uppercase character");
   QByteArray name = token->toString();
   ReASUserClass* clazz = new ReASUserClass(name, startPos, m_tree);
   ReSymbolSpace* parent = m_tree.currentSpace();
   ReASUserClass* alreadyDefined = parent->addClass(clazz);
   if (alreadyDefined != NULL) {
      error(L_PARSE_CLASS_ALREADY_DEFINED, alreadyDefined->position(),
            "class already defined", "previous defined class");
   }
   m_tree.startClassOrMethod(name, ReSymbolSpace::SST_CLASS);
   clazz->setSymbols();
   m_tree.finishClassOrMethod(name);
}

/**
 * @brief Parses a the import statement
 */
void ReMFParser::parseImport() {
}

/**
 * @brief Parses a module.
 *
 * @pre     the first char of the module is the next char to read.
 * @post    the total module is read
 *
 * @param name  the name of the module (without path)
 */
ReASItem* ReMFParser::parseModule(ReSourceUnitName name) {
   m_tree.startModule(name);
   // parse until EOF:
   ReASItem* body = parseBody(K_UNDEF);
   m_tree.finishModule(name);
   return body;
}
/**
 * @brief Parse the input given by the source.
 */
void ReMFParser::parse() {
   ReSource* source = m_lexer.source();
   ReSourceUnit* mainModule = source->currentReader()->currentSourceUnit();
   ReSourceUnitName mainModuleName = mainModule->name();
   try {
      ReASItem* body = parseModule(mainModuleName);
      ReSymbolSpace* module = m_tree.findmodule(mainModuleName);
      if (module != NULL)
         module->setBody(body);
   } catch (RplParserStop exc) {
      printf("compiling aborted: %s\n", exc.reason());
   }
}

/**
 * @brief Parses an argument list in a method call.
 *
 * @pre     the token '(' is read
 * @post    the token behind the ')' is read
 * @return  the first element of the argument list
 */
ReASExprStatement* ReMFParser::parseArguments() {
   ReASExprStatement* first = NULL;
   ReASExprStatement* last = NULL;
   bool again = false;
   do {
      ReASItem* expr = parseExpr(0);
      if (!m_lexer.currentToken()->isOperator(O_COMMA, O_RPARENTH))
         syntaxError(L_PARSE_ARGS_NO_COMMA_OR_PARENT, "',' or ')' expected");
      again = m_lexer.currentToken()->isOperator(O_COMMA);
      ReASExprStatement* current = new ReASExprStatement();
      current->setPosition(expr->position());
      current->setChild2(expr);
      if (first == NULL)
         first = last = current;
      else {
         last->setChild(current);
         last = current;
      }
   } while (again);
   // skip ')':
   m_lexer.nextNonSpaceToken();
   return first;
}

