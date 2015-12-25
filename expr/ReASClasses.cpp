/*
 * ReASClasses.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Predefined classes of the virtual machine, e.g ReASInteger.
 */
/** @file expr/ReASClasses.hpp
 *
 * @brief Definitions for predefined classes of the virtual machine.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

ReASList* ReASList::m_instance = NULL;
ReASMap* ReASMap::m_instance = NULL;
ReASFloat* ReASFloat::m_instance = NULL;
ReASInteger* ReASInteger::m_instance = NULL;
ReASString* ReASString::m_instance = NULL;
ReASBoolean* ReASBoolean::m_instance = NULL;
ReASVoid* ReASVoid::m_instance = NULL;
ReASFormula* ReASFormula::m_instance = NULL;

/** @class ReSymbolSpace ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements a symbol space for the parser.
 *
 * A symbol space is a container of the classes and variables which can be used
 * at a given moment while compiling.
 *
 * A symbol space could have a parent which is a symbol space too and extends
 * the visible classes / variables.
 *
 * If an entry exists more than one time in a symbol space chain only the first
 * can be seen.
 *
 * The "last" parent is named "global" and exists always. It contains the
 * core classes like String, List and Map and methods like print.
 *
 * Each module hit by the compiler builds its own symbol space with global as parent.
 *
 * Each class defines a symbol space and takes the symbol space of the superclass
 * as its parent. If there is no superclass the module's symbol space will be taken.
 *
 * Each method defines its own symbol space. The parent may be the symbol space
 * of the module or of the class.
 */
/**
 * @brief Constructor, only for the global symbol space.
 *
 * @param name      the symbol space's name
 * @param parent    the parent of the symbol space
 */
ReSymbolSpace::ReSymbolSpace(ReASTree& tree) :
   m_type(SST_GLOBAL),
   m_name("$global"),
   m_variables(),
   m_classes(),
   m_parent(NULL),
   m_body(NULL),
   m_listOfVars(),
   m_tree(tree) {
}
/**
 * @brief Constructor.
 *
 * @param type      the type of the symbol space: SST_MODULE, ...
 * @param name      the symbol space's name
 * @param parent    the parent of the symbol space
 */
ReSymbolSpace::ReSymbolSpace(ReSymbolSpace::SymbolSpaceType type,
                             const QByteArray& name, ReSymbolSpace* parent) :
   m_type(type),
   m_name(name),
   m_variables(),
   m_classes(),
   m_parent(parent),
   m_body(NULL),
   m_listOfVars(),
   m_tree(parent->m_tree) {
}

/**
 * @brief Destructor.
 */
ReSymbolSpace::~ReSymbolSpace() {
   ClassMap::iterator it;
   for (it = m_classes.begin(); it != m_classes.end(); it++) {
      delete it.value();
   }
   MethodMap::iterator it2;
   for (it2 = m_methods.begin(); it2 != m_methods.end(); it2++) {
      delete it2.value();
   }
}

/**
 * @brief Starts a scope.
 *
 * A scope is an "area" where variables can be defined.
 * A variable "lives" in a scope.
 *
 * Saves the status to restore it in <code>finishScope()</code>.
 *
 * @param scope     OUT: status info
 */
void ReSymbolSpace::startScope(ReASScope& scope) {
   scope.m_varNoAtStart = m_listOfVars.size();
}

/**
 * @brief Finishes a scope.
 *
 * Finishes the "live" of the variables created in the ending scope.
 *
 * @param endOfScope    line (inside the current source unit) which finishes the
 *                      scope
 * @param scope         the status of the scope at start.
 */
void ReSymbolSpace::finishScope(int endOfScope, ReASScope& scope) {
   // in methods/classes not needed:
   int ix = scope.m_varNoAtStart - scope.m_builtInVars;
   int last = m_listOfVars.size();
   for (; ix < last; ix++) {
      ReASVarDefinition* var = m_listOfVars[ix];
      var->setEndOfScope(endOfScope);
      const QByteArray& name = var->name();
      if (m_variables.contains(name))
         m_variables.remove(name);
   }
}

/**
 * @brief Search a variable in the symbol space.
 *
 * @param name  variable to find
 *
 * @return      NULL: not found<br>
 *              otherwise: the variable
 */
ReASVarDefinition* ReSymbolSpace::findVariable(const QByteArray& name) const {
   ReASVarDefinition* rc = NULL;
   if (m_variables.contains(name))
      rc = m_variables[name];
   else if (m_parent != NULL)
      rc = m_parent->findVariable(name);
   return rc;
}

/**
 * @brief Search the class in the symbol space hierarchy.
 *
 * @param name  Name of the class
 * @return      NULL: not found<br>
 *              otherwise: the class
 */
ReASClass* ReSymbolSpace::findClass(const QByteArray& name) const {
   ReASClass* rc = NULL;
   if (m_classes.contains(name))
      rc = m_classes[name];
   else if (m_parent != NULL)
      rc = m_parent->findClass(name);
   return rc;
}

/**
 * @brief Find a method in the instance.
 *
 * @param name  the method's name
 * @return      NULL: method not found
 *              otherwise: the method description
 */
ReASMethod* ReSymbolSpace::findMethod(const QByteArray& name) const {
   ReASMethod* rc = NULL;
   if (m_methods.contains(name))
      rc = m_methods[name];
   return rc;
}

/**
 * @brief Writes the content of the instance into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level: so many tabs will be used as prefix
 * @param header    NULL or the headline
 */
void ReSymbolSpace::dump(ReWriter& writer, int indent, const char* header) {
   if (header != NULL)
      writer.writeLine(header);
   writer.formatIndented(indent, "= %s (%s) parent: %s", m_name.constData(),
                         spaceTypeName(m_type),
                         m_parent == NULL ? "<none>" : m_parent->name().constData());
   QList < QByteArray > sorted;
   if (m_classes.size() > 0) {
      writer.writeIndented(indent, "== Classes:");
      sorted.reserve(m_classes.size());
      ClassMap::iterator it;
      for (it = m_classes.begin(); it != m_classes.end(); it++) {
         sorted.append(it.key());
      }
      qSort(sorted.begin(), sorted.end(), qLess<QByteArray>());
      QList<QByteArray>::iterator it2;
      for (it2 = sorted.begin(); it2 != sorted.end(); it2++) {
         ReASClass* clazz = m_classes[*it2];
         clazz->dump(writer, indent);
      }
   }
   if (m_methods.size() > 0) {
      writer.writeIndented(indent, "== Methods:");
      sorted.clear();
      sorted.reserve(m_variables.size());
      MethodMap::iterator it3;
      for (it3 = m_methods.begin(); it3 != m_methods.end(); it3++) {
         sorted.append(it3.key());
      }
      qSort(sorted.begin(), sorted.end(), qLess<QByteArray>());
      QList<QByteArray>::iterator it4;
      for (it4 = sorted.begin(); it4 != sorted.end(); it4++) {
         ReASMethod* method = m_methods[*it4];
         do {
            method->dump(writer, indent);
            method = method->sibling();
         } while (method != NULL);
      }
   }
   if (m_listOfVars.size() > 0) {
      writer.writeIndented(indent, "== Variables:");
      QList<QByteArray>::iterator it6;
      for (int ix = 0; ix < m_listOfVars.size(); ix++) {
         ReASVarDefinition* var = m_listOfVars[ix];
         var->dump(writer, indent);
      }
   }
   if (m_body != NULL) {
      writer.writeIndented(indent, "== Body:");
      ReASNode1::dumpStatements(writer, indent, m_body);
   }
}

/**
 * @brief Returns the name of a space type.
 *
 * @param type  type to inspect
 * @return      the name of the type
 */
const char* ReSymbolSpace::spaceTypeName(ReSymbolSpace::SymbolSpaceType type) {
   const char* rc = NULL;
   switch (type) {
   case SST_UNDEF:
      rc = "undef";
      break;
   case SST_GLOBAL:
      rc = "global";
      break;
   case SST_MODULE:
      rc = "module";
      break;
   case SST_CLASS:
      rc = "class";
      break;
   case SST_METHOD:
      rc = "method";
      break;
   default:
      rc = "?";
      break;
   }
   return rc;
}

/**
 * @brief Initilizes the global symbol space.
 *
 * @param tree  the abstract syntax tree
 * @return      the global symbol space
 */
ReSymbolSpace* ReSymbolSpace::createGlobal(ReASTree& tree) {
   ReSymbolSpace* rc = new ReSymbolSpace(tree);
   rc->m_tree = tree;
   ReASInteger::m_instance = new ReASInteger(tree);
   rc->m_classes[ReASInteger::m_instance->name()] = ReASInteger::m_instance;
   ReASBoolean::m_instance = new ReASBoolean(tree);
   rc->m_classes[ReASBoolean::m_instance->name()] = ReASBoolean::m_instance;
   ReASFloat::m_instance = new ReASFloat(tree);
   rc->m_classes[ReASFloat::m_instance->name()] = ReASFloat::m_instance;
   ReASString::m_instance = new ReASString(tree);
   rc->m_classes[ReASString::m_instance->name()] = ReASString::m_instance;
   ReASList::m_instance = new ReASList(tree);
   rc->m_classes[ReASList::m_instance->name()] = ReASList::m_instance;
   ReASMap::m_instance = new ReASMap(tree);
   rc->m_classes[ReASMap::m_instance->name()] = ReASMap::m_instance;
   ReASVoid::m_instance = new ReASVoid(tree);
   rc->m_classes[ReASVoid::m_instance->name()] = ReASVoid::m_instance;
   ReASFormula::m_instance = new ReASFormula(tree);
   rc->m_classes[ReASFormula::m_instance->name()] = ReASFormula::m_instance;
   return rc;
}
/**
 * @brief Returns the list of the variables.
 *
 * @return the list of the variables
 */
ReSymbolSpace::VariableList ReSymbolSpace::listOfVars() const {
   return m_listOfVars;
}

/**
 * @brief Returns the parent of the symbol space.
 *
 * @return  the symbolspace of the object (module, method, class..) containing
 *          the object belonging to the instance
 */
ReSymbolSpace* ReSymbolSpace::parent() const {
   return m_parent;
}

/**
 * @brief Returns the body (an abstract syntax tree) of the symbol space.
 *
 * @return  NULL: no body available<br>
 *          othewise: the body of the instance
 */
ReASItem* ReSymbolSpace::body() const {
   return m_body;
}

/**
 * @brief Sets the body (an abstract syntax tree) of the symbol space.
 *
 * @param body  the new body
 */
void ReSymbolSpace::setBody(ReASItem* body) {
   m_body = body;
}

/**
 * @brief Adds a variable to the symbol space.
 *
 * @param variable  the variable to add
 * @param varNo     OUT: variable number, current number in the symbol space
 * @return          NULL: success<br>
 *                  otherwise: the already defined variable/method
 */
ReASItem* ReSymbolSpace::addVariable(ReASVarDefinition* variable, int& varNo) {
   ReASItem* rc = NULL;
   const QByteArray& name = variable->name();
   if (m_variables.contains(name))
      rc = m_variables[name];
   else if (m_methods.contains(name))
      rc = m_methods[name];
   else {
      m_variables[name] = variable;
      varNo = m_listOfVars.size();
      m_listOfVars.append(variable);
   }
   return rc;
}

/**
 * @brief Adds a method to the symbol space.
 *
 * @param method    the method to add
 * @return          NULL: success<br>
 *                  otherwise: the already defined variable/method
 */
ReASItem* ReSymbolSpace::addMethod(ReASMethod* method) {
   ReASItem* rc = NULL;
   const QByteArray& name = method->name();
   if (m_variables.contains(name))
      rc = m_variables[name];
   else if (!m_methods.contains(name)) {
      m_methods[name] = method;
   } else {
      ReASMethod* first = m_methods[name];
      ReASMethod* oldMethod = first;
      do {
         if (oldMethod->equalSignature(*method))
            rc = oldMethod;
         else
            oldMethod = oldMethod->sibling();
      } while (rc == NULL && oldMethod != NULL);
      if (rc == NULL) {
         method->setChild(first);
         m_methods[name] = method;
      }
   }
   return rc;
}
/**
 * @brief Adds a class to the instance.
 *
 * @param clazz     the class to add
 * @return          NULL: success<br>
 *                  otherwise: the already defined class
 */
ReASUserClass* ReSymbolSpace::addClass(ReASUserClass* clazz) {
   ReASUserClass* rc = NULL;
   const QByteArray& name = clazz->name();
   if (m_classes.contains(name)) {
      rc = dynamic_cast<ReASUserClass*>(m_classes[name]);
   } else {
      m_classes[name] = clazz;
   }
   return rc;
}

/**
 * @brief Returns the name of the symbol space.
 *
 * @return the name
 */
const QByteArray& ReSymbolSpace::name() const {
   return m_name;
}

/** @class ReASBoolean ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements the class of a Boolean.
 *
 * A Boolean is a real value.
 */
/**
 * @brief Constructor.
 */
ReASBoolean::ReASBoolean(ReASTree& tree) :
   ReASClass("Bool", tree) {
}
/**
 * @brief Creates a value object (used in ReASVariant).
 *
 * For Booleans nothing is to do!
 *
 * @param source    NULL or a source to copy
 * @return          NULL
 */
void* ReASBoolean::newValueInstance(void*) const {
   return NULL;
}

/**
 * @brief Destroys the given object.
 *
 * For Booleans nothing is to do!
 *
 * @param object    object to destroy
 */
void ReASBoolean::destroyValueInstance(void*) const {
}

/**
 * @brief Calculates the boolean value of an class specific object.
 *
 * This method should never be called.
 *
 * @param object    the object to test (with type QList*)
 * @return          false
 */
bool ReASBoolean::boolValueOf(void*) const {
   return false;
}

/**
 * @brief Returns a string representation of an instance.
 *
 * @param object    the object to convert
 * @param maxLength not used
 * @return          a string describing the <code>object</code>
 */
QByteArray ReASBoolean::toString(void* object, int) const {
   return ((ReASVariant*) object)->asBool() ? "True" : "False";
}

/** @class ReASNumber ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements the class of a Boolean.
 *
 * A Boolean is one of the values true and false.
 */
/**
 * @brief Constructor.
 */
ReASFloat::ReASFloat(ReASTree& tree) :
   ReASClass("Float", tree) {
}

ReASFloat::ReASFloat(const QByteArray& name, ReASTree& tree) :
   ReASClass(name, tree) {
   m_superClass = ReASFloat::m_instance;
}
/**
 * @brief Creates a value object (used in ReASVariant).
 *
 * For Booleans nothing is to do!
 *
 * @param source    NULL or a source to copy
 * @return          NULL
 */
void* ReASFloat::newValueInstance(void*) const {
   return NULL;
}

/**
 * @brief Destroys the given object.
 *
 * For Booleans nothing is to do!
 *
 * @param object    object to destroy
 */
void ReASFloat::destroyValueInstance(void*) const {
}

/**
 * @brief Calculates the boolean value of an class specific object.
 *
 * This method should never be called.
 *
 * @param object    the object to test
 * @return          false
 */
bool ReASFloat::boolValueOf(void*) const {
   return false;
}

/**
 * @brief Returns a string representation of an instance.
 *
 * @param object    the object to convert
 * @param maxLength not used
 * @return          a string describing the <code>object</code>
 */
QByteArray ReASFloat::toString(void* object, int) const {
   char buffer[256];
   qsnprintf(buffer, sizeof buffer, "%f", ((ReASVariant*) object)->asFloat());
   return buffer;
}

/** @class ReASInteger ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements the class of a Boolean.
 *
 * A Boolean is a one of the values true and false.
 */
/**
 * @brief Constructor.
 */
ReASInteger::ReASInteger(ReASTree& tree) :
   ReASFloat("Int", tree) {
}

/**
 * @brief Calculates the boolean value of an class specific object.
 *
 * This method should never be called.
 *
 * @param object    the object to test
 * @return          false
 */
bool ReASInteger::boolValueOf(void*) const {
   return false;
}

/**
 * @brief Returns a string representation of an instance.
 *
 * @param object    the object to convert
 * @param maxLength the maximum length of the result
 * @return          a string describing the <code>object</code>
 */
QByteArray ReASInteger::toString(void* object, int maxLength) const {
   char buffer[64];
   qsnprintf(buffer, sizeof buffer, "%.*d", maxLength,
             ((ReASVariant*) object)->asInt());
   return buffer;
}

/** @class ReASString ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements the class of a string.
 *
 * A string is a mutable character sequence.
 */
/**
 * @brief Constructor.
 */
ReASString::ReASString(ReASTree& tree) :
   ReASClass("Str", tree) {
}
/**
 * @brief Creates a value object (used in ReASVariant).
 *
 * @param source    NULL or a source to copy
 * @return          a new value object (specific for the class)
 */
void* ReASString::newValueInstance(void* source) const {
   QByteArray* rc =
      source == NULL ?
      new QByteArray() : new QByteArray(*(QByteArray*) source);
   return (void*) rc;
}

/**
 * @brief Destroys the given object.
 *
 * The object must be created by <code>newValueInstance()</code>.
 *
 * @param object    object to destroy
 */
void ReASString::destroyValueInstance(void* object) const {
   delete (QByteArray*) object;
}

/**
 * @brief Calculates the boolean value of an class specific object.
 *
 * This method should never be called.
 *
 * @param object    the object to test (a QByteArray* instance)
 * @return          false: the string is empty
 *                  true: otherwise
 */
bool ReASString::boolValueOf(void* object) const {
   bool rc = false;
   if (object != NULL) {
      QByteArray* string = static_cast<QByteArray*>(object);
      if (string == NULL)
         throw ReException("ReASString.boolValueOf(): not a string");
      rc = !string->isEmpty();
   }
   return rc;
}

/**
 * @brief Returns a string representation of an instance.
 *
 * @param object    the object to convert
 * @param maxLength the maximum length of the result
 * @return          a string describing the <code>object</code>
 */
QByteArray ReASString::toString(void* object, int maxLength) const {
   QByteArray rc;
   QByteArray* string = reinterpret_cast<QByteArray*>(object);
   int length = string->size();
   if (length + 2 > maxLength)
      length = maxLength - 2;
   rc.reserve(length);
   rc += "'";
   if (string->size() < maxLength - 2) {
      rc += *string;
   } else {
      rc += string->mid(0, maxLength - 2 - 3);
      rc += "...";
   }
   rc += "'";
   return rc;
}

/** @class ReASList ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements the class of a list.
 *
 * A list is a container of any values.  Values can be selected by the index.
 */
/**
 * @brief Constructor.
 */
ReASList::ReASList(ReASTree& tree) :
   ReASClass("List", tree) {
}

/**
 * @brief Creates a value object (used in ReASVariant).
 *
 * @param source    NULL or a source to copy
 * @return          a new value object (specific for the class)
 */
void* ReASList::newValueInstance(void* source) const {
   ReASListOfVariants* rc = new ReASListOfVariants();
   if (source != NULL) {
      ReASListOfVariants* source2 = (ReASListOfVariants*) source;
      rc->reserve(source2->size());
      ReASListOfVariants::iterator it;
      for (it = source2->begin(); it != source2->end(); it++) {
         // deleting in destroyValue():
         rc->append(new ReASVariant(*(*it)));
      }
   }
   return (void*) rc;
}

/**
 * @brief Destroys the given object.
 *
 * The object must be created by <code>newValueInstance()</code>.
 *
 * @param object    object to destroy
 */
void ReASList::destroyValueInstance(void* object) const {
   delete static_cast<ReASListOfVariants*>(object);
}

/**
 * @brief Calculates the boolean value of an class specific object.
 *
 * @param object    the object to test (with type QList*)
 * @return          false: the list is empty<br>
 *                  true: otherwise
 */
bool ReASList::boolValueOf(void* object) const {
   bool rc = false;
   if (object != NULL) {
      ReASListOfVariants* list = static_cast<ReASListOfVariants*>(object);
      if (list == NULL)
         throw ReException("ReASList.boolValueOf(): not a list");
      rc = !list->empty();
   }
   return rc;
}

/**
 * @brief Returns a string representation of an instance.
 *
 * @param object    the object to convert
 * @param maxLength unused
 * @return          a string describing the <code>object</code>
 */
QByteArray ReASList::toString(void* object, int maxLength) const {
   QByteArray rc;
   rc.reserve(maxLength);
   rc += "[";
   ReASListOfVariants* list = reinterpret_cast<ReASListOfVariants*>(object);
   ReASListOfVariants::iterator it;
   bool first = true;
   for (it = list->begin(); it != list->end(); it++) {
      if (first)
         first = false;
      else
         rc += ",";
      QByteArray part = (*it)->toString(maxLength - rc.size() - 5);
      if (maxLength - rc.size() - 5 - part.size() <= 0) {
         rc += "...";
         break;
      } else {
         rc += part;
      }
   }
   rc += "]";
   return rc;
}

/** @class ReASMap ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements the class of a map.
 *
 * A map is a container of (key, value) pairs.
 * Values can be selected by the key.
 */
/**
 * @brief Constructor.
 */
ReASMap::ReASMap(ReASTree& tree) :
   ReASClass("Map", tree) {
}
/**
 * @brief Creates a value object (used in ReASVariant).
 *
 * @param source    NULL or a source to copy
 * @return          a new value object (specific for the class)
 */
void* ReASMap::newValueInstance(void* source) const {
   ReASMapOfVariants* rc = new ReASMapOfVariants();
   if (source != NULL) {
      ReASMapOfVariants* source2 = static_cast<ReASMapOfVariants*>(source);
      // rc->reserve(source2->size());
      ReASMapOfVariants::iterator it;
      for (it = source2->begin(); it != source2->end(); it++) {
         // deleting in destroyValue():
         const QByteArray& key = it.key();
         ReASVariant* value = new ReASVariant(*it.value());
         (*rc)[key] = value;
      }
   }
   return (void*) rc;
}

/**
 * @brief Destroys the given object.
 *
 * The object must be created by <code>newValueInstance()</code>.
 *
 * @param object    object to destroy
 */
void ReASMap::destroyValueInstance(void* object) const {
   delete (ReASMapOfVariants*) object;
}

/**
 * @brief Calculates the boolean value of an class specific object.
 *
 * @param object    the object to test (with type QMap*)
 * @return
 */
bool ReASMap::boolValueOf(void* object) const {
   bool rc = false;
   if (object != NULL) {
      ReASMapOfVariants* map = reinterpret_cast<ReASMapOfVariants*>(object);
      if (map == NULL)
         throw ReException("ReASMap.boolValueOf(): not a map");
      rc = map->empty() > 0;
   }
   return rc;
}

/**
 * @brief Returns a string representation of an instance.
 *
 * @param object    the object to convert
 * @param maxLength maximal length of the result
 * @return          a string describing the <code>object</code>
 */
QByteArray ReASMap::toString(void* object, int maxLength) const {
   QByteArray rc;
   rc.reserve(maxLength);
   rc += "[";
   ReASMapOfVariants* map = reinterpret_cast<ReASMapOfVariants*>(object);
   ReASMapOfVariants::iterator it;
   bool first = true;
   for (it = map->begin(); it != map->end(); it++) {
      if (first)
         first = false;
      else
         rc += ",";
      if (maxLength - rc.size() - 5 - 2 - it.key().size() <= 0) {
         rc += "...";
         break;
      } else {
         rc += "'";
         rc += it.key();
         rc += "':";
      }
      QByteArray part = it.value()->toString(maxLength - rc.size() - 5);
      if (maxLength - rc.size() - 5 - part.size() <= 0) {
         rc += "...";
         break;
      } else {
         rc += part;
      }
   }
   rc += "}";
   return rc;
}

/** @class ReVariable ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements a variable of a symbol space.
 */

/**
 * @brief Constructor.
 */
ReVariable::ReVariable(const QByteArray& name) :
   m_name(name),
   m_namespace(NULL),
   m_value(),
   m_type(NULL) {
}

/**
 * @brief Writes the content of the instance into an output media.
 *
 * @param writer    writes to output
 * @param indent    nesting level: so many tabs will be used as prefix
 */
void ReVariable::dump(ReWriter& writer, int indent) {
   const char* name1 =
      m_type == NULL ? "NoneType" : m_type->name().constData();
   QByteArray val = m_value.toString();
   writer.formatIndented(indent, "%s %s: value: %s", name1, m_name.constData(),
                         val.constData());
}
/**
 * @brief Returns the data type of the variable.
 *
 * @return  the class of the variable
 */
ReASClass* ReVariable::type() const {
   return m_type;
}

/**
 * @brief Sets the data type.
 * @param type  the class of the variable
 */
void ReVariable::setType(ReASClass* type) {
   m_type = type;
}
/**
 * @brief Returns the name of the variable.
 *
 * @return  the name
 */
const QByteArray& ReVariable::name() const {
   return m_name;
}

/** @class ReVariable ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements a data type representing a none type.
 */
ReASVoid::ReASVoid(ReASTree& tree) :
   ReASClass("Void", tree) {
}

/**
 * @brief Instantiates a new object.
 *
 * In this case we do nothing.
 *
 * @param source    ignored
 * @return
 */
void* ReASVoid::newValueInstance(void*) const {
   return NULL;
}

/**
 * @brief Destroys an object created by newValueInstance.
 *
 * In this case we do nothing.
 *
 * @param object    not used
 */
void ReASVoid::destroyValueInstance(void*) const {
}

/**
 * @brief Returns the bool value of the given object
 * @param object    ignored
 * @return          false
 */
bool ReASVoid::boolValueOf(void*) const {
   return false;
}

/**
 * @brief Converts the object into a string.
 *
 * @param object    ignored
 * @param maxLength ignored
 * @return          the empty string
 */
QByteArray ReASVoid::toString(void*, int) const {
   return QByteArray("");
}

/** @class ReASFormula ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements a data type representing a calculated value.
 */

/**
 * @brief Constructor.
 *
 * @param tree  the abstract syntax tree
 */
ReASFormula::ReASFormula(ReASTree& tree) :
   ReASClass("Formula", tree) {
}

/**
 * @brief Instantiates a new object.
 *
 * In this case we do nothing.
 *
 * @param expr      the result
 * @return
 */
void* ReASFormula::newValueInstance(void* expr) const {
   return expr;
}

/**
 * @brief Destroys an object created by newValueInstance.
 *
 * In this case we do nothing.
 *
 * @param object    not used
 */
void ReASFormula::destroyValueInstance(void*) const {
}

/**
 * @brief Returns the bool value of the given object
 * @param object    ignored
 * @return          false
 */
bool ReASFormula::boolValueOf(void*) const {
   return false;
}

/**
 * @brief Converts the object into a string.
 *
 * @param object    ignored
 * @param maxLength ignored
 * @return          the empty string
 */
QByteArray ReASFormula::toString(void* object, int) const {
   ReASExprStatement* expr = static_cast<ReASExprStatement*>(object);
   char buffer[64];
   qsnprintf(buffer, sizeof buffer, "<formula %d>", expr->id());
   return buffer;
}

/** @class ReASUserClass ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements a data type representing an user defined class.
 */

/**
 * @brief Constructor.
 *
 * @param name      name of the user defined class
 * @param position  the position of the class definition
 * @param tree      the abstract syntax tree
 */
ReASUserClass::ReASUserClass(const QByteArray& name,
                             const ReSourcePosition* position, ReASTree& tree) :
   ReASClass(name, tree),
   m_position(position) {
}

/**
 * @brief Creates an instance of an user defined class.
 *
 * @param source    the type (user defined class) of the result
 * @return          an instance of an user defined class
 */
void* ReASUserClass::newValueInstance(void* source) const {
   ReASUserClass* clazz = static_cast<ReASUserClass*>(source);
   ReASUserObject* rc = new ReASUserObject(clazz);
   return static_cast<void*>(rc);
}

void ReASUserClass::destroyValueInstance(void* object) const {
   ReASUserObject* obj = static_cast<ReASUserObject*>(object);
   delete obj;
}

/**
 * @brief Returns the bool value of the object.
 *
 * @param object    object to test
 * @return          true: object != NULL<br>
 *                  false: object == NULL
 */
bool ReASUserClass::boolValueOf(void* object) const {
   return object != NULL;
}

/**
 * @brief Returns a string representation an instance of a user defined class.
 *
 * @param object        object to convert
 * @param maxLength     maximum length of the string
 * @return
 */
QByteArray ReASUserClass::toString(void*, int) const {
   return m_name;
}
/**
 * @brief Returns the source position of the instance.
 *
 * @return  the source position
 */
const ReSourcePosition* ReASUserClass::position() const {
   return m_position;
}

/** @class ReASUserObject ReASClasses.hpp "expr/ReASClasses.hpp"
 *
 * @brief Implements an instance of an user defined class.
 */
ReASUserObject::ReASUserObject(ReASUserClass* clazz) :
   m_class(clazz),
   m_fields(NULL) {
}

/**
 * @brief Destructor.
 */
ReASUserObject::~ReASUserObject() {
   delete[] m_fields;
   m_fields = NULL;
}
