/*
 * ReASClasses.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef RPLASCLASSES_HPP
#define RPLASCLASSES_HPP

class ReSymbolSpace;
class ReVariable {
public:
   ReVariable(const QByteArray& name);
public:
   void dump(ReWriter& writer, int indent);

   ReASClass* type() const;
   void setType(ReASClass* type);
   const QByteArray& name() const;

protected:
   QByteArray m_name;
   // NULL for "simple" variables (int, float, bool)
   ReSymbolSpace* m_namespace;
   ReASVariant m_value;
   ReASClass* m_type;
};

class ReASScope {
public:
   int m_builtInVars;
   int m_varNoAtStart;
};

class ReASUserClass;
class ReASTree;
class ReSymbolSpace {
public:
   enum SymbolSpaceType {
      SST_UNDEF,
      SST_GLOBAL,
      SST_MODULE,
      SST_CLASS,
      SST_METHOD
   };

public:
   typedef QMap<QByteArray, ReASVarDefinition*> VariableMap;
   typedef QMap<QByteArray, ReASClass*> ClassMap;
   typedef QMap<QByteArray, ReASMethod*> MethodMap;
   typedef QList<ReASVarDefinition*> VariableList;
private:
   ReSymbolSpace(ReASTree& tree);
public:
   ReSymbolSpace(SymbolSpaceType type, const QByteArray& name,
                 ReSymbolSpace* parent);
   virtual ~ReSymbolSpace();
public:
   void startScope(ReASScope& scope);
   void finishScope(int endOfScope, ReASScope& scope);
   ReASVarDefinition* findVariable(const QByteArray& name) const;
   ReASClass* findClass(const QByteArray& name) const;
   ReASMethod* findMethod(const QByteArray& name) const;
   void dump(ReWriter& writer, int indent, const char* header = NULL);
   const QByteArray& name() const;
   ReASItem* body() const;
   void setBody(ReASItem* body);
   ReASItem* addVariable(ReASVarDefinition* variable, int& varNo);
   ReASItem* addMethod(ReASMethod* method);
   ReASUserClass* addClass(ReASUserClass* clazz);
   ReSymbolSpace* parent() const;
   VariableList listOfVars() const;
public:
   static const char* spaceTypeName(SymbolSpaceType type);
   static ReSymbolSpace* createGlobal(ReASTree& tree);
private:
   SymbolSpaceType m_type;
   QByteArray m_name;
   VariableMap m_variables;
   ClassMap m_classes;
   MethodMap m_methods;
   ReSymbolSpace* m_parent;
   ReASItem* m_body;
   VariableList m_listOfVars;
   ReASTree& m_tree;
};

class ReASBoolean: public ReASClass {
public:
   ReASBoolean(ReASTree& tree);
public:
   void* newValueInstance(void* source = NULL) const;
   void destroyValueInstance(void* object) const;
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
public:
   static ReASBoolean* m_instance;
};

class ReASFloat: public ReASClass {
public:
   ReASFloat(ReASTree& tree);
   ReASFloat(const QByteArray& name, ReASTree& tree);
public:
   void* newValueInstance(void* source = NULL) const;
   void destroyValueInstance(void* object) const;
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
public:
   static ReASFloat* m_instance;
};

class ReASInteger: public ReASFloat {
public:
   ReASInteger(ReASTree& tree);
public:
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
public:
   static ReASInteger* m_instance;
};

class ReASString: public ReASClass {
public:
   ReASString(ReASTree& tree);
public:
   void* newValueInstance(void* source = NULL) const;
   void destroyValueInstance(void* object) const;
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
public:
   static ReASString* m_instance;
};

class ReASList: public ReASClass {
public:
   ReASList(ReASTree& tree);
public:
   void* newValueInstance(void* source = NULL) const;
   void destroyValueInstance(void* object) const;
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
public:
   static ReASList* m_instance;
};

class ReASMap: public ReASClass {
public:
   ReASMap(ReASTree& tree);
public:
   void* newValueInstance(void* source = NULL) const;
   void destroyValueInstance(void* object) const;
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
public:
   static ReASMap* m_instance;
};

class ReASVoid: public ReASClass {
public:
   ReASVoid(ReASTree& tree);
public:
   void* newValueInstance(void* source = NULL) const;
   void destroyValueInstance(void* object) const;
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
public:
   static ReASVoid* m_instance;
};

class ReASFormula: public ReASClass {
public:
   ReASFormula(ReASTree& tree);
public:
   void* newValueInstance(void* source = NULL) const;
   void destroyValueInstance(void* object) const;
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
public:
   static ReASFormula* m_instance;
};

class ReASUserClass: public ReASClass {
public:
   ReASUserClass(const QByteArray& name, const ReSourcePosition* position,
                 ReASTree& tree);
public:
   void* newValueInstance(void* source = NULL) const;
   void destroyValueInstance(void* object) const;
   virtual bool boolValueOf(void* object) const;
   virtual QByteArray toString(void* object, int maxLength = 80) const;
   const ReSourcePosition* position() const;

private:
   const ReSourcePosition* m_position;
};

class ReASUserObject {
public:
   ReASUserObject(ReASUserClass* clazz);
   ~ReASUserObject();
public:
   void callMember();
private:
   ReASUserClass* m_class;
   ReASVariant* m_fields;
};

#endif // RPLASCLASSES_HPP
