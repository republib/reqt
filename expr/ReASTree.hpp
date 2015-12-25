/*
 * ReASTree.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef RPLASTREE_HPP
#define RPLASTREE_HPP

enum ReASItemType {
   AST_UNDEF,
   AST_CONSTANT,
   AST_LIST_CONSTANT,
   AST_LIST_ENTRY,
   AST_MAP_CONSTANT,
   AST_MAP_ENTRY,
   AST_NAMED_VALUE,
   AST_CONVERSION,
   AST_INDEXED_VALUE,
   AST_FIELD,
   AST_VAR_DEFINITION,
   AST_EXPR_STATEMENT,
   AST_METHOD,
   AST_ARGUMENT,
   AST_INTRINSIC_METHOD,
   AST_PRE_UNARY_OP,
   AST_POST_UNARY_OP,
   AST_BINARY_OP,
   AST_METHOD_CALL,
   AST_WHILE,
   AST_REPEAT,
   AST_IF,
   AST_CONDITION,
   AST_ITERATED_FOR,
   AST_COUNTED_FOR,
   AST_SWITCH,
   AST_LEAVE,
   AST_CONTINUE
};

class ReASException: public ReException {
public:
   ReASException();
   ReASException(const ReSourcePosition* position, const char* format, ...);
protected:
   void build(const ReSourcePosition* position, const char* format,
              va_list varList);
};

class ReASClass;
class ReASNamedValue;
class ReASItem;
class ReASCondition;

class ReASVariant {
   /* The VM uses some tricks (performance): Therefore this class
    * must not be virtual!
    */
public:
   enum VariantType {
      VT_UNDEF,
      VT_FLOAT,
      VT_INTEGER,
      VT_BOOL,
      VT_OBJECT
   };
   enum VariantFlags {
      VF_UNDEF,
      /// if DT_OBJECT: object is a copy, don't free at method end
      VF_IS_COPY = 1 << 1,
      /// debugger: action if changed
      VF_WATCH_POINT = 1 << 2
   };

   friend class ReASCondition;
public:
   ReASVariant();
   ~ReASVariant();
   ReASVariant(const ReASVariant& source);
   ReASVariant& operator=(const ReASVariant& source);
   qreal asFloat() const;
   int asInt() const;
   bool asBool() const;
   void* asObject(const ReASClass** clazz) const;
   const QByteArray* asString() const;
   void setFloat(qreal number);
   void setInt(int integer);
   void setBool(bool value);
   void setObject(void* object, const ReASClass* clazz);
   void setString(const QByteArray& string);
   QByteArray toString(int maxLength = 80) const;
   VariantType variantType() const;
   const char* nameOfType() const;
   const ReASClass* getClass() const;
   void copyValue(const ReASVariant& source);
   void destroyValue();
private:
   VariantType m_variantType :8;
   /// bitmap of VF_... flags:
   int m_flags :8;
   union {
      qreal m_float;
      int m_int;
      bool m_bool;
      void* m_object;
   } m_value;
   const ReASClass* m_class;
};

class ReASTree;
class ReParser;
class ReVMThread;

class ReASItem {
public:
   enum NodeFlags {
      NF_UNDEF,
      /// the node calculates a value:
      NF_CALCULABLE = 1 << 1,
      /// the node calculates a value:
      NF_STATEMENT = 1 << 2,
      /// the tree under this node is complete checked for data type correctness
      NF_TYPECHECK_COMPLETE = 1 << 3,
      /// debugger: this node is a breakpoint
      NF_BREAKPOINT = 1 << 5
   };

public:
   friend class ReASTree;
   ReASItem(ReASItemType type);
   virtual ~ReASItem();
public:
   virtual bool check(ReParser& parser) = 0;
public:
   bool checkAsCalculable(const char* description, ReASClass* expectedClass,
                          ReParser& parser);
   const ReSourcePosition* position() const;
   void setPosition(const ReSourcePosition* position);
   unsigned int id() const;
   char* positionStr(char buffer[], size_t bufferSize) const;
   void error(ReLogger* logger, int location, const char* format, ...);
public:
   /**
    * @brief Writes the content of the instance into an output medium.
    *
    * @param writer    writes to output media
    * @param indent    nesting level: so many tabs will be used as prefix
    */
   virtual void dump(ReWriter& writer, int indent) = 0;
public:
   static void reset();
   static bool checkStatementList(ReASItem* list, ReParser& parser);
   static int calcAsInteger(ReASItem* expr, ReVMThread& thread);
   static bool calcAsBoolean(ReASItem* expr, ReVMThread& thread);
public:
   ReASItemType nodeType() const;
   const char* nameOfItemType() const;

   int flags() const;
   void setFlags(int flags);

   bool typeCheck(ReASClass* clazz1, ReASClass* clazz2);
   bool error(int location, ReParser& parser, const char* format, ...);
   bool ensureError(ReParser& parser, const char* info);
protected:
   unsigned int m_id :16;
   ReASItemType m_nodeType :8;
   int m_flags :5;
   int m_dataType :3;
   const ReSourcePosition* m_position;
private:
   static unsigned int m_nextId;
};

class ReASNode1;
class ReASCalculable {
public:
   ReASCalculable();
public:
   virtual void calc(ReVMThread& thread) = 0;
public:
   ReASClass* clazz() const;
   void setClass(ReASClass* clazz);
protected:
   ReASClass* m_class;
};

class ReStackFrame;
class ReASStorable: public ReASCalculable {
};
class ReVMThread;
class ReASConstant: public ReASItem, public ReASCalculable {
public:
   ReASConstant();
public:
   virtual void calc(ReVMThread& thread);
   virtual bool check(ReParser& parser);
public:
   virtual void dump(ReWriter& writer, int indent);
   ReASVariant& value();
private:
   ReASVariant m_value;
};

class ReASNode1: public ReASItem {
public:
   ReASNode1(ReASItemType type);
   virtual ~ReASNode1();
public:
   ReASItem* child() const;
   void setChild(ReASItem* child);
public:
   static void dumpStatements(ReWriter& writer, int indent,
                              ReASItem* statements);
protected:
   ReASItem* m_child;
};

class ReASNode2: public ReASNode1 {
public:
   ReASNode2(ReASItemType type);
   virtual ~ReASNode2();
public:
   ReASItem* child2() const;
   void setChild2(ReASItem* child2);

protected:
   ReASItem* m_child2;
};

class ReASNode3: public ReASNode2 {
public:
   ReASNode3(ReASItemType type);
   virtual ~ReASNode3();
public:
   ReASItem* child3() const;
   void setChild3(ReASItem* child3);

protected:
   ReASItem* m_child3;
};

class ReASNode4: public ReASNode3 {
public:
   ReASNode4(ReASItemType type);
   virtual ~ReASNode4();
public:
   ReASItem* child4() const;
   void setChild4(ReASItem* child4);

protected:
   ReASItem* m_child4;
};

class ReASNode5: public ReASNode4 {
public:
   ReASNode5(ReASItemType type);
   virtual ~ReASNode5();
public:
   ReASItem* child5() const;
   void setChild5(ReASItem* child5);
protected:
   ReASItem* m_child5;
};
class ReASNode6: public ReASNode5 {
public:
   ReASNode6(ReASItemType type);
   virtual ~ReASNode6();
public:
   ReASItem* child6() const;
   void setChild6(ReASItem* child5);
protected:
   ReASItem* m_child6;
};

typedef QList<ReASVariant*> ReASListOfVariants;
typedef QMap<QByteArray, ReASVariant*> ReASMapOfVariants;

class ReASListConstant: public ReASNode1, public ReASCalculable {
public:
   ReASListConstant();
public:
   virtual void calc(ReVMThread& thread);
   virtual bool check(ReParser& parser);

public:
   virtual void dump(ReWriter& writer, int indent);
   ReASVariant& value();
   ReASListOfVariants* list();
private:
   ReASVariant m_value;
};
class ReASMapConstant: public ReASNode1, public ReASCalculable {
public:
   ReASMapConstant();
public:
   virtual void calc(ReVMThread& thread);
   virtual bool check(ReParser& parser);
public:
   virtual void dump(ReWriter& writer, int indent);
   ReASVariant& value();
   ReASMapOfVariants* map();
private:
   ReASVariant m_value;
};

class ReSymbolSpace;
class ReASNamedValue: public ReASItem, public ReASStorable {
   friend class ReASVarDefinition;
public:
   enum Attributes {
      A_NONE,
      /// the value cannot be changed.
      A_CONST = 1 << 1,
      /// the variable/constant is found in the global namespace, not in a method
      A_GLOBAL = 1 << 2,
      /// the variable/constant is found in the module namespace, not in a method
      A_MODULE_STATIC = 1 << 3,
      /// the evaluation should be lazy
      A_LAZY = 1 << 4,
      /// parameter of a method
      A_PARAM = 1 << 5,
      /// a automatic variable in counted for loops:
      A_LOOP = 1 << 6
   };

public:
   ReASNamedValue(ReASClass* clazz, ReSymbolSpace* space,
                  const QByteArray& name, int attributes);
public:
   virtual void calc(ReVMThread& thread);
   virtual bool check(ReParser& parser);
public:
   const QByteArray& name() const;
   void setSymbolSpace(ReSymbolSpace* space, int variableNo);
   void dump(ReWriter& writer, int indent);
   ReSymbolSpace* symbolSpace() const;
   int variableNo() const;
   void setVariableNo(int variableNo);
protected:
   QByteArray m_name;
   int m_attributes;
   ReSymbolSpace* m_symbolSpace;
   int m_variableNo;
};
class ReASConversion: public ReASNode1, public ReASCalculable {
public:
   enum Conversion {
      C_UNDEF,
      C_INT_TO_FLOAT,
      C_FLOAT_TO_INT,
      C_BOOL_TO_INT,
      C_BOOL_TO_FLOAT
   };

public:
   ReASConversion(ReASItem* expression);
public:
   virtual void calc(ReVMThread& thread);
   virtual bool check(ReParser& parser);
   virtual void dump(ReWriter& writer, int indent);
public:
   static ReASConversion* tryConversion(ReASClass* expected, ReASItem* expr,
                                        ReParser& parser, bool& isCorrect);
   static Conversion findConversion(ReASClass* from, ReASClass* to);
private:
   Conversion m_conversion;
};

class ReASIndexedValue: public ReASNode2, public ReASCalculable {
public:
   ReASIndexedValue();
public:
   virtual void calc(ReVMThread& thread);
   virtual bool check(ReParser& parser);
public:
   void dump(ReWriter& writer, int indent);
};

class ReASStatement {
public:
   ReASStatement();
public:
   virtual int execute(ReVMThread& thread) = 0;
public:
   static int executeStatementList(ReASItem* list, ReVMThread& thread);
};

class ReASVarDefinition: public ReASNode3, public ReASStatement {
public:
   ReASVarDefinition();
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
public:
   void dump(ReWriter& writer, int indent);
   const QByteArray& name() const;
   int endOfScope() const;
   void setEndOfScope(int endOfScope);
   ReASClass* clazz() const;
private:
   /// the column of the blockend containing the definition.
   /// if 0: end is end of method or end of class
   /// Note: the source unit is stored in <code>ReASItem::m_sourcePosition</code>
   int m_endOfScope;
};

class ReASExprStatement: public ReASNode2, public ReASStatement {
public:
   ReASExprStatement();
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
public:
   void dump(ReWriter& writer, int indent);
};

class ReASUnaryOp: public ReASNode1, ReASCalculable {
public:
   enum UnaryOp {
      UOP_UNDEF,
      UOP_PLUS,
      UOP_MINUS_INT,
      UOP_MINUS_FLOAT,
      UOP_NOT_BOOL,
      UOP_NOT_INT,
      UOP_INC,
      UOP_DEC
   };
public:
   ReASUnaryOp(UnaryOp op, ReASItemType type);
public:
   virtual void calc(ReVMThread& thread);
   virtual bool check(ReParser& parser);
public:
   int getOperator() const;
   void dump(ReWriter& writer, int indent);
public:
   static const char* nameOfOp(UnaryOp op);
private:
   UnaryOp m_operator;
};
class ReASBinaryOp: public ReASNode2, public ReASCalculable {
public:
   enum BinOperator {
      BOP_UNDEF,
      BOP_ASSIGN,
      BOP_PLUS_ASSIGN,
      BOP_MINUS_ASSIGN,
      BOP_TIMES_ASSIGN,
      BOP_DIV_ASSIGN,
      BOP_MOD_ASSIGN,
      BOP_POWER_ASSIGN,
      BOP_LOG_OR_ASSIGN,
      BOP_LOG_AND_ASSIGN,
      BOP_LOG_XOR_ASSIGN,
      BOP_BIT_OR_ASSIGN,
      BOP_BIT_AND_ASSIGN,
      BOP_BIT_XOR_ASSIGN,
      BOP_LSHIFT_ASSIGN,
      BOP_LOG_RSHIFT_ASSIGN,
      BOP_ARTITH_RSHIFT_ASSIGN,
      BOP_PLUS,
      BOP_MINUS,
      BOP_TIMES,
      BOP_DIV,
      BOP_MOD,
      BOP_POWER,
      BOP_LOG_OR,
      BOP_LOG_AND,
      BOP_LOG_XOR,
      BOP_BIT_OR,
      BOP_BIT_AND,
      BOP_BIT_XOR,
      BOP_LSHIFT,
      BOP_LOG_RSHIFT,
      BOP_ARTITH_RSHIFT,
      BOP_EQ,
      BOP_NE,
      BOP_LE,
      BOP_LT,
      BOP_GE,
      BOP_GT,
      BOB_COUNT
   };
private:
   inline bool isAssignment() const {
      return m_operator >= BOP_ASSIGN
             && m_operator <= BOP_ARTITH_RSHIFT_ASSIGN;
   }
public:
   ReASBinaryOp();
public:
   virtual void calc(ReVMThread& thread);
   virtual bool check(ReParser& parser);
public:
   BinOperator getOperator() const;
   void setOperator(BinOperator op);
   void dump(ReWriter& writer, int indent);
private:
   void assign(ReVMThread& thread);
public:
   static const char* nameOfOp(BinOperator op);
private:
   BinOperator m_operator;
};

class ReASIf: public ReASNode4, public ReASStatement {
public:
   ReASIf();
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
   virtual void dump(ReWriter& writer, int indent);
};

class ReASForIterated: public ReASNode4, public ReASStatement {
public:
   ReASForIterated(ReASVarDefinition* variable);
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
   virtual void dump(ReWriter& writer, int indent);
};

class ReASForCounted: public ReASNode6, public ReASStatement {
public:
   ReASForCounted(ReASVarDefinition* variable);
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
   virtual void dump(ReWriter& writer, int indent);
};

class ReASWhile: public ReASNode3, public ReASStatement {
public:
   ReASWhile();
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
   virtual void dump(ReWriter& writer, int indent);
};

class ReASRepeat: public ReASNode3, public ReASStatement {
public:
   ReASRepeat();
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
   virtual void dump(ReWriter& writer, int indent);
};

class ReASMethod;
class ReASMethodCall: public ReASNode3, public ReASStatement {
public:
   ReASMethodCall(const QByteArray& name, ReASItem* parent);
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
public:
   void dump(ReWriter& writer, int indent);

public:
   ReASMethod* method() const;
   void setMethod(ReASMethod* method);

   ReASExprStatement* arg1() const;
private:
   QByteArray m_name;
   ReASMethod* m_method;
};

class RplParameter: ReASItem {
public:
   RplParameter();
   virtual ~RplParameter();
private:
   QByteArray m_name;
   ReASNamedValue* m_default;
};

class ReASField: public ReASNode1 {
public:
   ReASField(const QByteArray& name);
public:
   virtual bool check(ReParser& parser);
public:
   void dump(ReWriter& writer, int indent);
private:
   QByteArray m_name;
};

class ReASClass;
class ReSymbolSpace;
class ReASMethod: public ReASNode2 {
public:
   ReASMethod(const QByteArray& name, ReASTree& tree);
public:
   virtual bool check(ReParser& parser);
   virtual int execute(ReVMThread& thread);
public:
   void dump(ReWriter& writer, int indent);
   ReSymbolSpace* symbols() const;
   void setSymbols();
   const QByteArray& name() const;
   bool equalSignature(ReASMethod& other) const;
   ReASMethod* sibling() const;
   void setSibling(ReASMethod* sibling);
   int getFirstParamWithDefault() const;
   void setFirstParamWithDefault(int value);

private:
   QByteArray m_name;
   ReASClass* m_resultType;
   ReSymbolSpace* m_symbols;
   // chain over all overloaded methods (same name, other signature):
   ReASMethod* m_sibling;
   ReASTree& m_tree;
   // -1: no parameter with default value. >= 0: index of the first
   int firstParamWithDefault;
};

class ReASClass {
public:
   typedef QMap<QByteArray, ReASMethod*> MethodMap;
public:
   ReASClass(const QByteArray& name, ReASTree& m_tree);
   virtual ~ReASClass();
public:
   /**
    * @brief Creates a value object (used in ReASVariant).
    *
    * @param source    NULL or a source to copy
    * @return          a new value object (specific for the class)
    */
   virtual void* newValueInstance(void* source = NULL) const = 0;
   /**
    * @brief Destroys the given object.
    *
    * The object must be created by <code>newValueInstance()</code>.
    *
    * @param object    object to destroy
    */
   virtual void destroyValueInstance(void* object) const = 0;
   /**
    * @brief Returns the boolean value of a class specific value.
    *
    * Example: the boolean value of an the empty string is false
    *
    * @param object    object to test
    * @return          false: the object represents the false value<br>
    *                  true: otherwise
    */
   virtual bool boolValueOf(void* object) const = 0;
   /**
    * @brief Returns a string representation of an instance.
    *
    * @param object    the object to convert
    * @param maxLength the maximum length of the result (string)
    * @return          a string describing the <code>object</code>
    */
   virtual QByteArray toString(void* object, int maxLength = 80) const = 0;
public:
   const QByteArray& name() const;
   virtual void dump(ReWriter& writer, int indent);
   void setSymbols();
protected:
   QByteArray m_name;
   ReSymbolSpace* m_symbols;
   const ReASClass* m_superClass;
   ReASTree& m_tree;
};

#include "expr/ReASClasses.hpp"

#include "ReParser.hpp"
class ReSymbolSpace;
class ReASTree {
public:
   enum {
      DMP_NONE,
      DMP_GLOBALS = 1 << 1,
      DMP_MODULES = 1 << 2,
      DMP_SPACE_STACK = 1 << 3,
      DMP_SPACE_HEAP = 1 << 4,
      DMP_ALL = DMP_GLOBALS | DMP_MODULES | DMP_SPACE_STACK | DMP_SPACE_HEAP,
      DMP_NO_GLOBALS = DMP_MODULES | DMP_SPACE_STACK | DMP_SPACE_HEAP
   };
   typedef QMap<QByteArray, ReSymbolSpace*> SymbolSpaceMap;
   typedef QList<ReSymbolSpace*> SymbolSpaceStack;
public:
   ReASTree();
   ~ReASTree();
public:
   bool startModule(ReSourceUnitName name);
   void finishModule(ReSourceUnitName name);
   ReSymbolSpace* startClassOrMethod(const QByteArray& name,
                                     ReSymbolSpace::SymbolSpaceType type);
   void finishClassOrMethod(const QByteArray& name);
   SymbolSpaceStack& symbolSpaces();
   ReSymbolSpace* currentSpace() const;
   ReASClass* findClass(const QByteArray& name);
   void clear();
   void dump(const char* filename, int flags = DMP_ALL, const char* header =
                NULL);
   ReSymbolSpace* findmodule(const QByteArray& name);
   ReSourcePosition* copyPosition();
   ReByteStorage& store();

protected:
   void init();
   void destroy();
private:
   // the mother of all symbol spaces.
   ReSymbolSpace* m_global;
   // contains all hit modules
   SymbolSpaceMap m_modules;
   // nested modules (import), classes and methods build this stack:
   SymbolSpaceStack m_symbolSpaces;
   // top of the stack:
   ReSymbolSpace* m_currentSpace;
   // contain all ever built symbol spaces:
   SymbolSpaceMap m_symbolSpaceHeap;
   ReByteStorage m_store;
};

#endif // RPLASTREE_HPP
