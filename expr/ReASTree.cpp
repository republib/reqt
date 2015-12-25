/*
 * ReASTree.cpp
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
 * @brief Implementation of an Abstract Syntax Tree.
 *
 */
/** @file expr/ReASTree.hpp
 *
 * @brief Definitions for an Abstract Syntax Tree.
 *
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

enum {
   LOC_VARDEF_EXEC_1 = LOC_FIRST_OF(LOC_ASTREE), // 11001
   LOC_UNOP_CALC_1,
   LOC_UNARY_CHECK_1,
   LOC_UNARY_CHECK_2,
   LOC_UNARY_CHECK_3, // 11005
   LOC_BINOP_1,
   LOC_BINOP_CALC_1,
   LOC_BINOP_CALC_2,
   LOC_BINOP_CALC_3,
   LOC_BINOP_CALC_4, // 11010
   LOC_BINOP_CALC_5,
   LOC_BINOP_CALC_6,
   LOC_BINOP_CALC_7,
   LOC_BINOP_CALC_8,
   LOC_BINOP_CALC_9,   // 11015
   LOC_BINOP_CALC_10,
   LOC_BINOP_CALC_11,
   LOC_BINOP_CALC_12,
   LOC_VARDEF_CHECK_1,
   LOC_VARDEF_CHECK_2, // 11020
   LOC_ITEM_STATEM_LIST_1,
   LOC_CONV_CHECK_1,
   LOC_CONV_TRY_1,
   LOC_ITEM_FORCE_ERROR_1,
   LOC_UNARY_CHECK_4,  // 11025
   LOC_IF_CHECK_1,
   LOC_IF_CHECK_2,
   LOC_FORC_CHECK_1,
   LOC_FORC_CHECK_2,
   LOC_FORC_CHECK_3, // 11030
   LOC_ITEM_AS_INT_1,
   LOC_ITEM_AS_INT_2,
   LOC_METHOD_CALL_CHECK_1,
   LOC_MEHTOD_CALL_CHECK_2,
   LOC_MEHTOD_CALL_CHECK_3,    // 11035
   LOC_MEHTOD_CALL_CHECK_4,
   LOC_COUNT
};

unsigned int ReASItem::m_nextId = 1;

#define DEFINE_TABS(indent)  \
	char tabs[32]; \
	memset(tabs, '\t', sizeof tabs); \
	tabs[(unsigned) indent < sizeof tabs ? indent : sizeof tabs - 1] = '\0'

/**
 * @brief Writes a map into a file.
 *
 * The output is sorted by key.
 *
 * @param writer        writes to output
 * @param map           map to dump
 * @param withEndOfLine true: '\n' will be written at the end
 */
void dumpMap(ReWriter& writer, ReASMapOfVariants& map, bool withEndOfLine) {
   QList < QByteArray > sorted;
   sorted.reserve(map.size());
   ReASMapOfVariants::iterator it;
   for (it = map.begin(); it != map.end(); it++) {
      sorted.append(it.key());
   }
   qSort(sorted.begin(), sorted.end(), qLess<QByteArray>());
   QList<QByteArray>::iterator it2;
   bool first = true;
   for (it2 = sorted.begin(); it2 != sorted.end(); it2++) {
      ReASVariant* value = map[*it2];
      writer.format("%c'%s':%s", first ? '{' : ',', (*it2).constData(),
                    value->toString().constData());
      first = false;
   }
   if (first)
      writer.write("{");
   writer.write("}");
   if (withEndOfLine)
      writer.writeLine();
}

/** @class ReASException ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a specific exception for the Abstract Syntax Tree.
 */

/**
 * @brief Builds the message.
 *
 * @param position  describes the position of the error/warning
 * @param format    the reason of the exception
 * @param varList   the values for the placeholders in the format.
 */
void ReASException::build(const ReSourcePosition* position, const char* format,
                          va_list varList) {
   char buffer[64000];
   if (position != NULL) {
      m_message = I18N::s2b(position->toString());
      m_message += ": ";
   }
   qvsnprintf(buffer, sizeof buffer, format, varList);
   m_message += buffer;
}

/**
 * @brief Constructor.
 *
 * @param position  describes the position of the error/warning
 * @param format    the reason of the exception
 * @param ...       the values for the placeholders in the format.
 */
ReASException::ReASException(const ReSourcePosition* position,
                             const char* format, ...) :
   ReException("") {
   va_list ap;
   va_start(ap, format);
   build(position, format, ap);
   va_end(ap);
}

/**
 * @brief Constructor.
 */
ReASException::ReASException() :
   ReException("") {
}

/** @class ReASVariant ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a class which can hold the value of any type.
 *
 * The VM uses some tricks (performance): Therefore this class
 * must not be virtual!
 */
/**
 * @brief Constructor.
 */
ReASVariant::ReASVariant() :
   m_variantType(VT_UNDEF),
   m_flags(VF_UNDEF),
   // m_value(),
   m_class(NULL) {
}
/**
 * @brief Destructor.
 */
ReASVariant::~ReASVariant() {
   destroyValue();
   m_variantType = VT_UNDEF;
}

/**
 * @brief Copy constructor.
 * @param source    the source to copy
 */
ReASVariant::ReASVariant(const ReASVariant& source) :
   m_variantType(source.m_variantType),
   m_flags(source.m_flags),
   // m_value
   m_class(source.m_class) {
   copyValue(source);
}

/**
 * @brief Assignment operator.
 *
 * @param source    the source to copy
 * @return          the instance itself
 */
ReASVariant& ReASVariant::operator=(const ReASVariant& source) {
   destroyValue();
   m_variantType = source.m_variantType;
   m_flags = source.m_flags;
   m_class = source.m_class;
   copyValue(source);
   return *this;
}

/**
 * @brief Copies the value.
 * @param source    the source to copy
 */
void ReASVariant::copyValue(const ReASVariant& source) {
   destroyValue();
   m_variantType = source.m_variantType;
   m_class = source.m_class;
   switch (source.m_variantType) {
   case VT_BOOL:
      m_value.m_bool = source.m_value.m_bool;
      break;
   case VT_FLOAT:
      m_value.m_float = source.m_value.m_float;
      break;
   case VT_INTEGER:
      m_value.m_int = source.m_value.m_int;
      break;
   case VT_UNDEF:
      break;
   default:
      m_value.m_object = m_class->newValueInstance(source.m_value.m_object);
      break;
   }
   m_flags = source.m_flags;
}

/**
 * @brief Frees the resources of the instance.
 */
void ReASVariant::destroyValue() {
   switch (m_variantType) {
   case VT_BOOL:
   case VT_FLOAT:
   case VT_INTEGER:
   case VT_UNDEF:
      break;
   default:
      if ((m_flags & VF_IS_COPY) == 0)
         m_class->destroyValueInstance(m_value.m_object);
      m_value.m_object = NULL;
      break;
   }
   m_variantType = VT_UNDEF;
}
/**
 * @brief Returns the variantType of the instance.
 *
 * @return  the variant type
 */
ReASVariant::VariantType ReASVariant::variantType() const {
   return m_variantType;
}

/**
 * @brief Return the name of the variant type.
 *
 * @return  the type as string
 */
const char* ReASVariant::nameOfType() const {
   const char* rc = "?";
   switch (m_variantType) {
   case VT_UNDEF:
      rc = "<undef>";
      break;
   case VT_FLOAT:
      rc = "Float";
      break;
   case VT_INTEGER:
      rc = "Int";
      break;
   case VT_BOOL:
      rc = "Bool";
      break;
   case VT_OBJECT:
      rc = "Obj";
      break;
   default:
      break;
   }
   return rc;
}

/**
 * @brief Returns the class (data type) of the instance.
 *
 * @return  the variant type
 */
const ReASClass* ReASVariant::getClass() const {
   return m_class;
}

/**
 * @brief Returns the numeric value.
 *
 * @return              the numeric value
 * @throw RplException  the instance is not a numberic value
 *
 */
qreal ReASVariant::asFloat() const {
   if (m_variantType != VT_FLOAT)
      throw ReException("ReASVariant::asNumber: not a number: %d",
                        m_variantType);
   return m_value.m_float;
}
/**
 * @brief Returns the numeric value.
 *
 * @return              the numeric value
 * @throw RplException  the instance is not a numberic value
 *
 */
int ReASVariant::asInt() const {
   if (m_variantType != VT_INTEGER)
      throw ReException("ReASVariant::asInt: not an integer: %d",
                        m_variantType);
   return m_value.m_int;
}

/**
 * @brief Returns the boolean value.
 *
 * @return              the boolean value
 * @throw RplException  the instance is not a boolean value
 *
 */
bool ReASVariant::asBool() const {
   if (m_variantType != VT_BOOL)
      throw ReException("ReASVariant::asBool: not a boolean: %d",
                        m_variantType);
   return m_value.m_bool;
}

/**
 * @brief Returns the class specific value.
 *
 * @param clazz         OUT: the class of the instance. May be NULL
 * @return              the class specific value
 * @throw RplException  the instance is not a boolean value
 *
 */
void* ReASVariant::asObject(const ReASClass** clazz) const {
   if (m_variantType != VT_OBJECT)
      throw ReException("ReASVariant::asObject: not an object: %d",
                        m_variantType);
   if (clazz != NULL)
      *clazz = m_class;
   return m_value.m_object;
}

/**
 * @brief Returns the value as string.
 *
 * @return the value as string
 * @throw RplException  the instance is not a string value
 */
const QByteArray* ReASVariant::asString() const {
   const ReASClass* clazz;
   const QByteArray* rc = static_cast<const QByteArray*>(asObject(&clazz));
   if (clazz != ReASString::m_instance) {
      const QByteArray& name = clazz->name();
      throw ReException("ReASVariant::asString: not a string: %s",
                        name.constData());
   }
   return rc;
}

/**
 * @brief Make the instance to a numeric value.
 *
 * @param number    the numeric value.
 */
void ReASVariant::setFloat(qreal number) {
   destroyValue();
   m_variantType = VT_FLOAT;
   m_value.m_float = number;
   m_class = ReASFloat::m_instance;
}

/**
 * @brief Make the instance to an integer value.
 *
 * @param integer    the numeric value.
 */
void ReASVariant::setInt(int integer) {
   destroyValue();
   m_variantType = VT_INTEGER;
   m_value.m_int = integer;
   m_class = ReASInteger::m_instance;
}

/**
 * @brief Make the instance to a boolean value.
 *
 * @param value    the boolean value.
 */
void ReASVariant::setBool(bool value) {
   destroyValue();
   m_variantType = VT_BOOL;
   m_value.m_bool = value;
   m_class = ReASBoolean::m_instance;
}

/**
 * @brief Make the instance to a boolean value.
 *
 * @param string    the string value.
 */
void ReASVariant::setString(const QByteArray& string) {
   // deletion in ReASVariant::destroyValue():
   setObject(new QByteArray(string), ReASString::m_instance);
}

/**
 * @brief Builds a string.
 *
 * @param maxLength     the maximum length of the result
 * @return  the value as string
 */
QByteArray ReASVariant::toString(int maxLength) const {
   QByteArray rc;
   char buffer[256];
   switch (m_variantType) {
   case VT_BOOL:
      rc = m_value.m_bool ? "True" : "False";
      break;
   case VT_FLOAT:
      qsnprintf(buffer, sizeof buffer, "%f", m_value.m_float);
      rc = buffer;
      break;
   case VT_INTEGER:
      qsnprintf(buffer, sizeof buffer, "%lld", m_value.m_int);
      rc = buffer;
      break;
   case VT_OBJECT:
      rc = m_class->toString(m_value.m_object, maxLength);
      break;
   default:
   case VT_UNDEF:
      rc = "None";
      break;
   }
   return rc;
}

/**
 * @brief Make the instance to an object.
 *
 * @param object    the class specific value object.
 * @param clazz     the data type of the object
 */
void ReASVariant::setObject(void* object, const ReASClass* clazz) {
   destroyValue();
   m_variantType = VT_OBJECT;
   m_value.m_object = object;
   m_class = clazz;
}

/** @class ReASItem ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements the abstract base class of all entries of an AST.
 *
 */

/**
 * @brief Constructor.
 *
 * @param type  the type of the instance
 */
ReASItem::ReASItem(ReASItemType type) :
   m_id(m_nextId++),
   m_nodeType(type),
   m_flags(0),
   m_position(NULL) {
}
/**
 * @brief Destructor.
 */

ReASItem::~ReASItem() {
}

/**
 * @brief Checks a calculable node for correctness.
 *
 * @param description   description of the meaning, e.g. "start value"
 * @param expectedClass the node must have this type
 * @param parser        for error processing
 * @return              <code>true</code>: instance and children are correct<br>
 *                      <code>false</code>: otherwise
 */
bool ReASItem::checkAsCalculable(const char* description,
                                 ReASClass* expectedClass, ReParser& parser) {
   RE_UNUSED(expectedClass);
   bool rc = true;
   if (!check(parser))
      rc = false;
   if (rc) {
      ReASCalculable* expr = dynamic_cast<ReASCalculable*>(this);
      if (expr == NULL)
         rc = error(LOC_ITEM_AS_INT_1, parser, "%s not calculable: %s",
                    description, nameOfItemType());
      else if (expr->clazz() != ReASInteger::m_instance)
         rc = error(LOC_ITEM_AS_INT_2, parser,
                    "%s: wrong type %s instead of integer", description,
                    expr->clazz()->name().constData());
   }
   return rc;
}

/**
 * @brief Returns the position of the item in the source code.
 *
 * @return the position of the item
 */
const ReSourcePosition* ReASItem::position() const {
   return m_position;
}

/**
 * @brief Stores the position in the source code.
 *
 * @param position  the position to store
 */
void ReASItem::setPosition(const ReSourcePosition* position) {
   m_position = position;
}

/**
 * @brief Returns the id of the instance.
 *
 * @return  the id
 */
unsigned int ReASItem::id() const {
   return m_id;
}

/**
 * @brief Returns the position as a string.
 *
 * @param buffer        OUT: the target buffer
 * @param bufferSize    size of the target buffer
 * @return              <code>buffer</code>
 */
char* ReASItem::positionStr(char buffer[], size_t bufferSize) const {
   char* rc = (char*) "";
   if (m_position != NULL)
      rc = m_position->utf8(buffer, bufferSize);
   return rc;
}

/**
 * @brief Logs an internal error.
 *
 * @param logger    can write to the output medium
 * @param location  identifies the error location
 * @param format    string with placeholders (optional) like <code>sprintf()</code>
 * @param ...       values for the placeholders
 */
void ReASItem::error(ReLogger* logger, int location, const char* format, ...) {
   char buffer[1024];
   int halfBufferSize = (sizeof buffer) / 2;
   qsnprintf(buffer, halfBufferSize, "id: %d [%s]:", m_id,
             positionStr(buffer + halfBufferSize, halfBufferSize));
   int length = strlen(buffer);
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer + length, (sizeof buffer) - length, format, ap);
   va_end(ap);
   logger->log(LOG_ERROR, location, buffer);
}

/**
 * @brief Resets the static id counter.
 */
void ReASItem::reset() {
   m_nextId = 1;
}
/**
 * @brief Calculates an integer value.
 *
 * @param expr      a calculable node
 * @param thread    the execution unit
 * @return          the value described by the node <code>expr</code>
 */
int ReASItem::calcAsInteger(ReASItem* expr, ReVMThread& thread) {
   ReASCalculable* expr2 = dynamic_cast<ReASCalculable*>(expr);
   expr2->calc(thread);
   ReASVariant& value = thread.popValue();
   int rc = value.asInt();
   return rc;
}

/**
 * @brief Calculates an boolean value.
 *
 * @param expr      a calculable node
 * @param thread    the execution unit
 * @return          the value described by the node <code>expr</code>
 */
bool ReASItem::calcAsBoolean(ReASItem* expr, ReVMThread& thread) {
   ReASCalculable* expr2 = dynamic_cast<ReASCalculable*>(expr);
   expr2->calc(thread);
   ReASVariant& value = thread.popValue();
   bool rc = value.asBool();
   return rc;
}
/**
 * @brief Checks the correctness of a statement list.
 *
 * @param list      statement list to check
 * @param parser    for error processing
 * @return          <code>true</code>: all statements are correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASItem::checkStatementList(ReASItem* list, ReParser& parser) {
   bool rc = true;
   while (list != NULL) {
      if (!list->check(parser))
         rc = false;
      if (dynamic_cast<ReASStatement*>(list) == NULL)
         rc = list->error(LOC_ITEM_STATEM_LIST_1, parser,
                          "not a statement: %s", list->nameOfItemType());
      ReASNode1* node = dynamic_cast<ReASNode1*>(list);
      if (node == NULL) {
         list->error(LOC_ITEM_STATEM_LIST_1, parser, "not a node: %s",
                     list->nameOfItemType());
         list = NULL;
      } else {
         list = node->child();
      }
   }
   return rc;
}
/**
 * @brief Returns the node type.
 *
 * @return  the node type
 */
ReASItemType ReASItem::nodeType() const {
   return m_nodeType;
}

/**
 * @brief Returns the node type as a string.
 *
 * @return  the node type as string
 */
const char* ReASItem::nameOfItemType() const {
   const char* rc = "?";
   switch (m_nodeType) {
   case AST_CONSTANT:
      rc = "constant";
      break;
   case AST_LIST_CONSTANT:
      rc = "list";
      break;
   case AST_LIST_ENTRY:
      rc = "listEntry";
      break;
   case AST_MAP_CONSTANT:
      rc = "map";
      break;
   case AST_MAP_ENTRY:
      rc = "mapEntry";
      break;
   case AST_NAMED_VALUE:
      rc = "namedValue";
      break;
   case AST_INDEXED_VALUE:
      rc = "indexedValue";
      break;
   case AST_FIELD:
      rc = "field";
      break;
   case AST_VAR_DEFINITION:
      rc = "varDef";
      break;
   case AST_EXPR_STATEMENT:
      rc = "exprStatement";
      break;
   case AST_METHOD:
      rc = "method";
      break;
   case AST_ARGUMENT:
      rc = "arg";
      break;
   case AST_INTRINSIC_METHOD:
      rc = "intrinsicMethod";
      break;
   case AST_PRE_UNARY_OP:
      rc = "preUnary";
      break;
   case AST_POST_UNARY_OP:
      rc = "postUnary";
      break;
   case AST_BINARY_OP:
      rc = "binOp";
      break;
   case AST_METHOD_CALL:
      rc = "methodCall";
      break;
   case AST_WHILE:
      rc = "while";
      break;
   case AST_REPEAT:
      rc = "repeat";
      break;
   case AST_IF:
      rc = "if";
      break;
   case AST_CONDITION:
      rc = "condition";
      break;
   case AST_ITERATED_FOR:
      rc = "iFor";
      break;
   case AST_COUNTED_FOR:
      rc = "cFor";
      break;
   case AST_SWITCH:
      rc = "switch";
      break;
   case AST_LEAVE:
      rc = "leave";
      break;
   case AST_CONTINUE:
      rc = "continue";
      break;
   default:
      break;
   }
   return rc;
}
/**
 * @brief Returns the flags of the node.
 *
 * @return  the bitmask with the flags
 */
int ReASItem::flags() const {
   return m_flags;
}
/**
 * @brief Sets the flags of the node.
 *
 * @param flags     the new value of the bitmask
 */
void ReASItem::setFlags(int flags) {
   m_flags = flags;
}

/**
 * @brief Tests the compatibility of 2 data types.
 * @param class1    1st class to inspect
 * @param class2    2nd class to inspect
 * @return          true: classes are compatible<br>
 *                  false: otherwise
 */
bool ReASItem::typeCheck(ReASClass* class1, ReASClass* class2) {
   bool rc;
   if (class1 == NULL || class2 == NULL)
      rc = false;
   else
      //@ToDo: subclasses
      rc = class1 == class2;
   return rc;
}

/**
 * @brief Issues an error message.
 *
 * @param location  an error specific id
 * @param parser    for error processing
 * @param format    the error message with placeholders (like <code>printf</code>)
 * @param ...       the values for the placeholders
 * @return          false (for chaining)
 */
bool ReASItem::error(int location, ReParser& parser, const char* format, ...) {
   va_list varList;
   va_start(varList, format);
   parser.addMessage(ReParser::LT_ERROR, location, m_position, format,
                     varList);
   va_end(varList);
   return false;
}

/**
 * @brief Ensures the occurrence of an error.
 *
 * When called a previous error should be happend. If not an internal error
 * will be issued.
 *
 * @param parser    for error processing
 * @param info      additional info
 * @return          <code>false</code> (for chaining)
 */
bool ReASItem::ensureError(ReParser& parser, const char* info) {
   if (parser.errors() == 0)
      error(LOC_ITEM_FORCE_ERROR_1, parser, "lost error (internal error): %s",
            info);
   return false;
}

/** @class ReASCalculable ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief An abstract base class for items which calculates a value.
 *
 */
/**
 * @brief Constructor.
 */
ReASCalculable::ReASCalculable() :
   m_class(NULL) {
}
/**
 * @brief Returns the class of the node
 * @return  the class
 */

ReASClass* ReASCalculable::clazz() const {
   return m_class;
}
/**
 * @brief Sets the class of the node.
 * @param clazz     the new class
 */
void ReASCalculable::setClass(ReASClass* clazz) {
   m_class = clazz;
}

/** @class ReASStorable ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements the abstract base class of value containing items.
 *
 */

/** @class ReASConstant ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a constant for the Abstract Syntax Tree.
 *
 */

/**
 * @brief Constructor.
 *
 */
ReASConstant::ReASConstant() :
   ReASItem(AST_CONSTANT),
   m_value() {
}

/**
 * @brief Copies the const value to the top of value stack.
 *
 * @param thread    IN/OUT: the execution unit, a VM thread
 */
void ReASConstant::calc(ReVMThread& thread) {
   ReASVariant& value = thread.reserveValue();
   value.copyValue(m_value);
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: a constant is always correct
 */
bool ReASConstant::check(ReParser& parser) {
   RE_UNUSED(&parser);
   return true;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASConstant::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent, "const id: %d value: %s %s", m_id,
                         m_value.toString().constData(), positionStr(buffer, sizeof buffer));
}

/**
 * @brief Returns the value of the constant.
 *
 * This method will be used to set the value of the constant:
 * <pre><code>ReASConstant constant;
 * constant.value().setString("Jonny");
 *</code></pre>
 *
 * @return the internal value
 */
ReASVariant& ReASConstant::value() {
   return m_value;
}

/** @class ReASListConstant ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a container for constant list entries.
 *
 */

/**
 * @brief Constructor.
 */
ReASListConstant::ReASListConstant() :
   ReASNode1(AST_LIST_CONSTANT),
   ReASCalculable() {
   m_value.setObject(ReASList::m_instance->newValueInstance(),
                     ReASList::m_instance);
}
/**
 * @brief Returns the list.
 *
 * @return  the list
 */
ReASListOfVariants* ReASListConstant::list() {
   ReASListOfVariants* rc = static_cast<ReASListOfVariants*>(m_value.asObject(
                               NULL));
   return rc;
}

/**
 * @brief Copies the list constant to the top of value stack.
 *
 * @param thread    IN/OUT: the execution unit, a VM thread
 */
void ReASListConstant::calc(ReVMThread& thread) {
   ReASVariant& value = thread.reserveValue();
   value.copyValue(m_value);
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: a constant is always correct
 */
bool ReASListConstant::check(ReParser& parser) {
   RE_UNUSED(&parser);
   return true;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASListConstant::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent, "listConst id: %d %s", m_id,
                         positionStr(buffer, sizeof buffer));
   QByteArray sValue = m_value.toString(8092);
   writer.writeIndented(indent + 1, sValue.constData());
}

/**
 * @brief Returns the value of the constant.
 *
 * This method will be used to set the value of the constant:
 * <pre><code>ReASConstant constant;
 * constant.value().setString("Jonny");
 *</code></pre>
 *
 * @return the internal value
 */
ReASVariant& ReASListConstant::value() {
   return m_value;
}

/** @class ReASMapConstant ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a hash map for constant list entries.
 *
 */
/**
 * @brief ReASMapConstant::ReASMapConstant
 */
ReASMapConstant::ReASMapConstant() :
   ReASNode1(AST_MAP_CONSTANT),
   ReASCalculable(),
   m_value() {
   m_value.setObject(new ReASMapOfVariants, ReASMap::m_instance);
}

/**
 * @brief Copies the map constant to the top of value stack.
 *
 * @param thread    IN/OUT: the execution unit, a VM thread
 */
void ReASMapConstant::calc(ReVMThread& thread) {
   ReASVariant& value = thread.reserveValue();
   value.copyValue(m_value);
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: a constant is always correct
 */
bool ReASMapConstant::check(ReParser& parser) {
   RE_UNUSED(&parser);
   return true;
}
/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASMapConstant::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent, "mapConst id: %d %s", m_id,
                         positionStr(buffer, sizeof buffer));
   writer.indent(indent);
   dumpMap(writer, *map(), true);
}

/**
 * @brief Returns the value of the constant, containing a map.
 *
 * @return  the variant value
 */
ReASVariant& ReASMapConstant::value() {
   return m_value;
}

/**
 * @brief Returns the (low level) map of the constant.
 *
 * @return  the map of the constant
 */
ReASMapOfVariants* ReASMapConstant::map() {
   ReASMapOfVariants* rc = static_cast<ReASMapOfVariants*>(m_value.asObject(
                              NULL));
   return rc;
}

/** @class ReASNamedValue ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a named values, a constant or a variable
 */

/**
 * @brief Constructor.
 *
 * @param clazz         the data type (class)
 * @param space         the current symbol space
 * @param name          the name of the variable
 * @param attributes    the attributes of the variable
 */
ReASNamedValue::ReASNamedValue(ReASClass* clazz, ReSymbolSpace* space,
                               const QByteArray& name, int attributes) :
   ReASItem(AST_NAMED_VALUE),
   m_name(name),
   m_attributes(attributes),
   m_symbolSpace(space),
   m_variableNo(-1) {
   m_class = clazz;
}

/**
 * @brief Returns the name.
 *
 * @return the name
 */
const QByteArray& ReASNamedValue::name() const {
   return m_name;
}

/**
 * @brief Sets the symbol space.
 * @param space
 * @param variableNo
 */
void ReASNamedValue::setSymbolSpace(ReSymbolSpace* space, int variableNo) {
   m_symbolSpace = space;
   m_variableNo = variableNo;
}
/**
 * @brief Copies the value of the variable to the top of value stack.
 *
 * @param thread    IN/OUT: the execution unit, a VM thread
 */
void ReASNamedValue::calc(ReVMThread& thread) {
   thread.valueToTop(m_symbolSpace, m_variableNo);
   if (thread.tracing())
      thread.vm()->traceWriter()->format("nVal %s=%.80s", m_name.constData(),
                                         thread.topOfValues().toString().constData());
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASNamedValue::check(ReParser& parser) {
   RE_UNUSED(&parser);
   return true;
}
/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASNamedValue::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent, "namedValue %s id: %d attr: 0x%x %s",
                         m_name.constData(), m_id, m_attributes,
                         positionStr(buffer, sizeof buffer));
}

/**
 * @brief Returns the symbol space of the variable.
 *
 * @return  the symbol space
 */
ReSymbolSpace* ReASNamedValue::symbolSpace() const {
   return m_symbolSpace;
}

/**
 * @brief Sets the variable no in the instance.
 *
 * @param variableNo    the variable number
 */
void ReASNamedValue::setVariableNo(int variableNo) {
   m_variableNo = variableNo;
}

/**
 * @brief Returns the variable no of the variable.
 *
 * @return  the current number of the variable in the symbol space
 */
int ReASNamedValue::variableNo() const {
   return m_variableNo;
}

/** @class ReASConversion ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a data type conversion.
 *
 * <code>m_child</code>: the expression which will be converted
 */
/**
 * @brief Constructor.
 * @param expression    the expression to convert
 */
ReASConversion::ReASConversion(ReASItem* expression) :
   ReASNode1(AST_CONVERSION),
   m_conversion(C_UNDEF) {
   m_child = expression;
   m_position = expression->position();
}

/**
 * @brief Convert an expression to another data type.
 *
 * Possible conversions: @see <code>ReASConversion::Conversion</code>
 *
 * @param thread    execution value
 */
void ReASConversion::calc(ReVMThread& thread) {
   ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child);
   expr->calc(thread);
   ReASVariant& value = thread.topOfValues();
   switch (m_conversion) {
   case C_INT_TO_FLOAT:
      value.setFloat((qreal) value.asInt());
      break;
   case C_FLOAT_TO_INT:
      value.setInt((int) value.asFloat());
      break;
   case C_BOOL_TO_INT:
      value.setInt((int) value.asBool());
      break;
   case C_BOOL_TO_FLOAT:
      value.setFloat((qreal) value.asBool());
      break;
   default:
      break;
   }
   if (thread.tracing())
      thread.vm()->traceWriter()->format("(%s): %s",
                                         m_class->name().constData(), value.toString().constData());
}

/**
 * @brief Returns the conversion type of two classes.
 *
 * @param from  class to convert
 * @param to    result class of the conversion
 *
 * @return      <code>C_UNDEF</code>: not convertable<br>
 *              otherwise: the conversion type
 */
ReASConversion::Conversion ReASConversion::findConversion(ReASClass* from,
      ReASClass* to) {
   Conversion rc = C_UNDEF;
   if (from == ReASFloat::m_instance) {
      if (to == ReASInteger::m_instance)
         rc = C_FLOAT_TO_INT;
   } else if (from == ReASInteger::m_instance) {
      if (to == ReASFloat::m_instance)
         rc = C_INT_TO_FLOAT;
   } else if (from == ReASBoolean::m_instance) {
      if (to == ReASInteger::m_instance)
         rc = C_BOOL_TO_INT;
      else if (to == ReASInteger::m_instance)
         rc = C_BOOL_TO_INT;
      else if (to == ReASFloat::m_instance)
         rc = C_BOOL_TO_FLOAT;
   }
   return rc;
}

/**
 * @brief Checks the node.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASConversion::check(ReParser& parser) {
   bool rc = m_child != NULL && m_child->check(parser);
   ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child);
   if (!rc || expr == NULL)
      ensureError(parser, "ReASConversion::check");
   else {
      ReASClass* from = expr->clazz();
      m_conversion = findConversion(from, m_class);
      if (m_conversion != C_UNDEF)
         rc = true;
      else
         parser.error(LOC_CONV_CHECK_1,
                      "invalid data type conversion: %s -> %s",
                      from->name().constData(), m_class->name().constData());
   }
   return rc;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASConversion::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent, "conversion %s id: %d expr: %d %s",
                         m_class->name().constData(), m_id, m_child->id(),
                         positionStr(buffer, sizeof buffer));
}

/**
 * @brief Tries to find a conversion to a given type.
 *
 * Checks if an expression has a given type. If not it will be tried to find
 * a conversion. If this is not possible an error occurres. Otherwise the
 * converter will be returned.
 *
 * @param expected  the expected data type
 * @param expr      the expression to convert
 * @param parser    for error processing
 * @param isCorrect OUT: false: error has been detected<br>
 *                  No change if no error
 *
 * @return          NULL: no conversion necessary<br>
 *                  otherwise: a converter to the given type
 */
ReASConversion* ReASConversion::tryConversion(ReASClass* expected,
      ReASItem* expr, ReParser& parser, bool& isCorrect) {
   ReASConversion* rc = NULL;
   if (!expr->check(parser))
      isCorrect = false;
   else {
      ReASCalculable* expr2 = dynamic_cast<ReASCalculable*>(expr);
      if (expr2 != NULL) {
         Conversion type = findConversion(expr2->clazz(), expected);
         if (type == C_UNDEF) {
            isCorrect = parser.error(LOC_CONV_TRY_1,
                                     "invalid data type conversion: %s -> %s",
                                     expr2->clazz()->name().constData(),
                                     expected->name().constData());
         } else if (expr2->clazz() != expected) {
            rc = new ReASConversion(expr);
            rc->m_conversion = type;
            rc->setClass(expected);
         }
      }
   }
   return rc;
}

/** @class ReASIndexedValue ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an indexed values (member of a list)
 *
 * <code>m_child</code>: the parent: a list/map expression
 * <code>m_child2</code>: the index expression
 */
ReASIndexedValue::ReASIndexedValue() :
   ReASNode2(AST_INDEXED_VALUE) {
}

/**
 * @brief Calculates an indexed expression.
 *
 * Possible: list index or map index
 *
 * @param thread    execution value
 */
void ReASIndexedValue::calc(ReVMThread& thread) {
   ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child2);
   expr->calc(thread);
   ReASVariant& ixValue = thread.popValue();
   int ix = ixValue.asInt();
   ReASCalculable* list = dynamic_cast<ReASCalculable*>(m_child);
   list->calc(thread);
   ReASVariant& listValue = thread.popValue();
   //@ToDo: access to the lists element: assignment or to stack
   if (thread.tracing())
      thread.vm()->traceWriter()->format("[%d]: %.80s", ix,
                                         thread.topOfValues().toString().constData());
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASIndexedValue::check(ReParser& parser) {
   ReASCalculable* list = dynamic_cast<ReASCalculable*>(m_child);
   bool rc = m_child != NULL && m_child->check(parser);
   if (!rc || list == NULL)
      ensureError(parser, "ReASIndexedValue::check");
   else {
      // index value:
      // tryConversion() calls m_child2->check()!
      ReASConversion* converter = ReASConversion::tryConversion(
                                     ReASInteger::m_instance, m_child2, parser, rc);
      if (rc && converter != NULL)
         m_child = converter;
      if (rc) {
         //@ToDo: dynamic subclass of list / map
         m_class = ReASString::m_instance;
         rc = m_class != NULL && m_class == ReASInteger::m_instance;
      }
   }
   return rc;
}

/**
 * @brief Writes the internals into an output media.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASIndexedValue::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent, "indexedValue id: %d index: %d parent: %d %s",
                         m_id, m_child2->id(), m_child->id(),
                         positionStr(buffer, sizeof buffer));
   m_child2->dump(writer, indent + 1);
   m_child->dump(writer, indent + 1);
}

/** @class RplVarDefinition ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements variable definition for the Abstract Syntax Tree.
 *
 * <code>m_child</code>: next statement<br>
 * <code>m_child2</code>: named value (name + default value expression)
 * <code>m_child3</code>: initial value or NULL
 */

/**
 * @brief Constructor.
 */
ReASVarDefinition::ReASVarDefinition() :
   ReASNode3(AST_VAR_DEFINITION),
   ReASStatement(),
   m_endOfScope(0) {
   m_flags |= NF_STATEMENT;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASVarDefinition::dump(ReWriter& writer, int indent) {
   ReASNamedValue* namedValue = dynamic_cast<ReASNamedValue*>(m_child2);
   QByteArray name = namedValue->name();
   char endOfScope[32];
   endOfScope[0] = '\0';
   if (m_endOfScope > 0)
      qsnprintf(endOfScope, sizeof endOfScope, "-%d:0", m_endOfScope);
   char buffer[256];
   writer.formatIndented(indent,
                         "varDef %s %s id: %d namedValue: %d value: %d succ: %d %s%s",
                         clazz() == NULL ? "?" : clazz()->name().constData(), name.constData(),
                         m_id, m_child2 == NULL ? 0 : m_child2->id(),
                         m_child3 == NULL ? 0 : m_child3->id(),
                         m_child == NULL ? 0 : m_child->id(), positionStr(buffer, sizeof buffer),
                         endOfScope);
   if (m_child2 != NULL)
      m_child2->dump(writer, indent + 1);
   if (m_child3 != NULL)
      m_child3->dump(writer, indent + 1);
}

/**
 * @brief Returns the name of the variable.
 *
 * @return  the name
 */
const QByteArray& ReASVarDefinition::name() const {
   ReASNamedValue* namedValue = dynamic_cast<ReASNamedValue*>(m_child2);
   const QByteArray& rc = namedValue->name();
   return rc;
}

/**
 * @brief Returns the data type (class) of the variable.
 *
 * @return  the data type
 */
ReASClass* ReASVarDefinition::clazz() const {
   ReASNamedValue* namedValue = dynamic_cast<ReASNamedValue*>(m_child2);
   ReASClass* rc = namedValue == NULL ? NULL : namedValue->clazz();
   return rc;
}
/**
 * @brief Returns the column of the scope end.
 *
 * 0 means end of method or end of class
 *
 * @return 0 or the column of the scope end
 */
int ReASVarDefinition::endOfScope() const {
   return m_endOfScope;
}

/**
 * @brief Sets the column of the scope end.
 *
 * @param endOfScope    the column of the scope end
 */
void ReASVarDefinition::setEndOfScope(int endOfScope) {
   m_endOfScope = endOfScope;
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASVarDefinition::check(ReParser& parser) {
   ReASNamedValue* var = dynamic_cast<ReASNamedValue*>(m_child2);
   bool rc = var != NULL && (m_child3 == NULL || m_child3->check(parser));
   if (!rc)
      ensureError(parser, "ReASVarDefinition::check");
   else {
      if (m_child3 != NULL) {
         // with initialization:
         ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child3);
         if (expr == NULL)
            rc = error(LOC_VARDEF_CHECK_1, parser,
                       "Not a calculable expression: %s",
                       m_child3->nameOfItemType());
         else if (!typeCheck(var->clazz(), expr->clazz()))
            rc = error(LOC_VARDEF_CHECK_2, parser,
                       "data types are not compatible: %s/%s",
                       var->clazz()->name().constData(),
                       expr->clazz() == NULL ?
                       "?" : expr->clazz()->name().constData());
      }
   }
   return rc;
}

/**
 * @brief Executes the statement.
 *
 * @return  0: continue the current statement list<br>
 */
int ReASVarDefinition::execute(ReVMThread& thread) {
   if (m_child3 != NULL) {
      // has an initialization:
      ReASNamedValue* var = dynamic_cast<ReASNamedValue*>(m_child2);
      ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child3);
      expr->calc(thread);
      ReASVariant& value = thread.popValue();
      ReASVariant& destination = thread.valueOfVariable(var->m_symbolSpace,
                                 var->m_variableNo);
      if (thread.tracing())
         thread.vm()->traceWriter()->format("%s = %.80s [%.80s]",
                                            var->m_name.constData(), value.toString().constData(),
                                            destination.toString().constData());
      destination.copyValue(value);
   }
   return 0;
}

/** @class ReASExprStatement ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an statement consisting of an expression.
 *
 * <code>m_child</code>: next statement<br>
 * <code>m_child2</code>: expression
 */

/**
 * @brief Constructor.
 */
ReASExprStatement::ReASExprStatement() :
   ReASNode2(AST_EXPR_STATEMENT),
   ReASStatement() {
   m_flags |= NF_STATEMENT;
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASExprStatement::check(ReParser& parser) {
   bool rc = m_child2->check(parser);
   if (rc) {
      ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child2);
      if (expr == NULL)
         rc = ensureError(parser, "ReASExprStatement::check");
   }
   return rc;
}
/**
 * @brief Executes the statement.
 *
 * @return  0: continue the current statement list<br>
 */
int ReASExprStatement::execute(ReVMThread& thread) {
   ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child2);
   expr->calc(thread);
   ReASVariant& value = thread.popValue();
   if (thread.tracing())
      thread.vm()->traceWriter()->format("expr: %s",
                                         value.toString().constData());
   value.destroyValue();
   return 0;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */

void ReASExprStatement::dump(ReWriter& writer, int indent) {
   char buffer[256];
   if (m_id == 40)
      m_id = 40;
   writer.formatIndented(indent, "Expr id: %d expr: %d succ: %d %s", m_id,
                         m_child2 == NULL ? 0 : m_child2->id(),
                         m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   if (m_child2 != NULL)
      m_child2->dump(writer, indent + 1);
}

/** @class ReASNode1 ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an inner node of the abstract syntax tree with one child.
 *
 * This class is an abstract class.
 */

/**
 * @brief ReASNode1::ReASNode1
 * @param type
 */
ReASNode1::ReASNode1(ReASItemType type) :
   ReASItem(type),
   m_child(NULL) {
}

/**
 * @brief Destructor.
 */
ReASNode1::~ReASNode1() {
   delete m_child;
   m_child = NULL;
}
/**
 * @brief Returns the child.
 *
 * @return  the child of the instance
 */
ReASItem* ReASNode1::child() const {
   return m_child;
}
/**
 * @brief Sets the child.
 */
void ReASNode1::setChild(ReASItem* child) {
   m_child = child;
}

/**
 * @brief Writes the internals of a statement list into a file.
 *
 * @param writer        writes to output media
 * @param indent        the indent level of the statement list
 * @param statements    the chain of statements to dump
 */
void ReASNode1::dumpStatements(ReWriter& writer, int indent,
                               ReASItem* statements) {
   ReASNode1* chain = dynamic_cast<ReASNode1*>(statements);
   while (chain != NULL) {
      chain->dump(writer, indent);
      chain = dynamic_cast<ReASNode1*>(chain->m_child);
   }
}

/** @class ReASNode2 ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an inner node of the abstract syntax tree with two childs.
 *
 * This class is an abstract class.
 */

/**
 * @brief ReASNode2::ReASNode2
 * @param type
 */
ReASNode2::ReASNode2(ReASItemType type) :
   ReASNode1(type),
   m_child2(NULL) {
}

/**
 * @brief Destructor.
 */
ReASNode2::~ReASNode2() {
   delete m_child2;
   m_child2 = NULL;
}
ReASItem* ReASNode2::child2() const {
   return m_child2;
}

void ReASNode2::setChild2(ReASItem* child2) {
   m_child2 = child2;
}

/** @class ReASNode3 ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an inner node of the abstract syntax tree with 3 childs.
 *
 * This class is an abstract class.
 */

/**
 * @brief ReASNode3::ReASNode3
 * @param type
 */
ReASNode3::ReASNode3(ReASItemType type) :
   ReASNode2(type),
   m_child3(NULL) {
}

/**
 * @brief Destructor.
 */
ReASNode3::~ReASNode3() {
   delete m_child3;
   m_child3 = NULL;
}
/**
 * @brief Returns the child3.
 *
 * @return  the child 3
 */
ReASItem* ReASNode3::child3() const {
   return m_child3;
}

/**
 * @brief Sets the child3.
 *
 * @param child3    the new child3
 */
void ReASNode3::setChild3(ReASItem* child3) {
   m_child3 = child3;
}

/** @class ReASNode4 ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an inner node of the abstract syntax tree with 3 childs.
 *
 * This class is an abstract class.
 */

/**
 * @brief ReASNode4::ReASNode4
 * @param type
 */
ReASNode4::ReASNode4(ReASItemType type) :
   ReASNode3(type),
   m_child4(NULL) {
}

/**
 * @brief Destructor.
 */
ReASNode4::~ReASNode4() {
   delete m_child4;
   m_child4 = NULL;
}

/**
 * @brief Returns the child4.
 *
 * @return  the child 4
 */
ReASItem* ReASNode4::child4() const {
   return m_child4;
}

/**
 * @brief Sets the child4.
 *
 * @param child4    the new child3
 */
void ReASNode4::setChild4(ReASItem* child4) {
   m_child4 = child4;
}

/** @class ReASNode5 ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an inner node of the abstract syntax tree with 4 childs.
 *
 * This class is an abstract class.
 */

/**
 * @brief ReASNode5::ReASNode5
 * @param type
 */
ReASNode5::ReASNode5(ReASItemType type) :
   ReASNode4(type),
   m_child5(NULL) {
}

/**
 * @brief Destructor.
 */
ReASNode5::~ReASNode5() {
   delete m_child5;
   m_child5 = NULL;
}

/**
 * @brief Returns the child5.
 *
 * @return  the child 5
 */
ReASItem* ReASNode5::child5() const {
   return m_child5;
}

/**
 * @brief Sets the child5.
 *
 * @param child5    the new child3
 */
void ReASNode5::setChild5(ReASItem* child5) {
   m_child5 = child5;
}

/** @class ReASNode6 ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an inner node of the abstract syntax tree with 4 childs.
 *
 * This class is an abstract class.
 */

/**
 * @brief ReASNode6::ReASNode6
 * @param type
 */
ReASNode6::ReASNode6(ReASItemType type) :
   ReASNode5(type),
   m_child6(NULL) {
}

/**
 * @brief Destructor.
 */
ReASNode6::~ReASNode6() {
   delete m_child6;
   m_child6 = NULL;
}

/**
 * @brief Returns the child5.
 *
 * @return  the child 5
 */
ReASItem* ReASNode6::child6() const {
   return m_child6;
}

/**
 * @brief Sets the child6.
 *
 * @param child6    the new child6
 */
void ReASNode6::setChild6(ReASItem* child6) {
   m_child6 = child6;
}

/** @class ReASUnaryOp ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an unary operation.
 *
 * This is an operation with one operand, e.g. the boolean 'not' operation.
 *
 * <code>m_child</code>: operand
 */

/**
 * @brief Constructor.
 *
 * @param op    the operator id
 * @param type  the node type
 */
ReASUnaryOp::ReASUnaryOp(UnaryOp op, ReASItemType type) :
   ReASNode1(type),
   m_operator(op) {
}

/**
 * @brief Calculates the value of the unary operator.
 *
 * @param thread    IN/OUT: the execution unit, a VM thread
 */
void ReASUnaryOp::calc(ReVMThread& thread) {
   ReASVariant& value = thread.topOfValues();
   switch (m_operator) {
   case UOP_PLUS:
      break;
   case UOP_MINUS_INT:
      value.setInt(-value.asInt());
      break;
   case UOP_MINUS_FLOAT:
      value.setFloat(-value.asFloat());
      break;
   case UOP_NOT_BOOL:
      value.setBool(!value.asBool());
      break;
   case UOP_NOT_INT:
      value.setInt(~value.asInt());
      break;
   case UOP_DEC:
   case UOP_INC:
   default:
      error(thread.logger(), LOC_UNOP_CALC_1, "unknown operator: %d",
            m_operator);
      break;
   }
   if (thread.tracing())
      thread.vm()->traceWriter()->format("unary %s: %s", nameOfOp(m_operator),
                                         value.toString().constData());
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASUnaryOp::check(ReParser& parser) {
   bool rc = m_child->check(parser);
   if (rc) {
      ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child);
      ReASClass* clazz = expr == NULL ? NULL : expr->clazz();
      if (clazz == NULL) {
         rc = ensureError(parser, "ReASUnaryOp::check");
      } else {
         switch (m_operator) {
         case UOP_PLUS:
            if (clazz != ReASInteger::m_instance
                  && clazz != ReASFloat::m_instance)
               rc = error(LOC_UNARY_CHECK_1, parser,
                          "wrong data type for unary operator '+': %s",
                          clazz->name().constData());
            break;
         case UOP_MINUS_INT:
            if (clazz != ReASFloat::m_instance)
               m_operator = UOP_MINUS_FLOAT;
            else if (clazz != ReASInteger::m_instance)
               rc = error(LOC_UNARY_CHECK_2, parser,
                          "wrong data type for unary operator '-': %s",
                          clazz->name().constData());
            break;
         case UOP_NOT_BOOL:
            if (clazz != ReASBoolean::m_instance)
               rc = error(LOC_UNARY_CHECK_3, parser,
                          "wrong data type for unary operator '!': %s",
                          clazz->name().constData());
            break;
         case UOP_NOT_INT:
            if (clazz != ReASInteger::m_instance)
               rc = error(LOC_UNARY_CHECK_4, parser,
                          "wrong data type for unary operator '!': %s",
                          clazz->name().constData());
            break;
         case UOP_DEC:
            break;
         case UOP_INC:
            break;
         default:
            throw ReASException(position(), "unknown operator: %d",
                                m_operator);
            break;
         }
      }
   }
   return rc;
}

/**
 * @brief Returns the operator of the unary operation.
 *
 * @return the operator
 */
int ReASUnaryOp::getOperator() const {
   return m_operator;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASUnaryOp::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent, "Unary %d op: %s (%d) expr: %d %s", m_id,
                         nameOfOp(m_operator), m_operator, m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   if (m_child != NULL)
      m_child->dump(writer, indent + 1);
}
/**
 * @brief Returns the name (a string) of an unary operator.
 *
 * @param op    the operand to convert
 * @return      the name of the operator
 */
const char* ReASUnaryOp::nameOfOp(ReASUnaryOp::UnaryOp op) {
   const char* rc;
   switch (op) {
   case UOP_PLUS:
      rc = "+";
      break;
   case UOP_MINUS_INT:
   case UOP_MINUS_FLOAT:
      rc = "-";
      break;
   case UOP_NOT_BOOL:
      rc = "!";
      break;
   case UOP_NOT_INT:
      rc = "~";
      break;
   case UOP_INC:
      rc = "++";
      break;
   case UOP_DEC:
      rc = "--";
      break;
   default:
      throw ReException("unknown unary operator: %d", (int) op);
      break;
   }
   return rc;
}

/** @class ReASStatement ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a base class for all statements.
 *
 * @note statements are always <code>ReASNode1</code> and m_child is used
 * for the successors (next statement).
 */

/**
 * @brief Constructor.
 */
ReASStatement::ReASStatement() {
}
/**
 * @brief Executes the statements of a statement list.
 *
 * @param list      statement list
 * @param thread    execution unit
 * @return  0: continue the current statement list<br>
 *          n > 0: stop the n most inner statement lists (initialized by leave)
 *          n < 0: stop the -n most inner statement lists (initialized by continue)
 */
int ReASStatement::executeStatementList(ReASItem* list, ReVMThread& thread) {
   int rc = 0;
   while (rc == 0 && list != NULL) {
      ReASStatement* statement = dynamic_cast<ReASStatement*>(list);
      rc = statement->execute(thread);
   }
   return rc;
}

/** @class ReASIf ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an if statement.
 *
 * The if statement has a condition, a then-part and an optional else-part.
 * If the condition is evaluated to true, the then-part will be executed.
 * Otherwise the else-part if it exists.
 *
 * <code>m_child</code>: next statement<br>
 * <code>m_child2</code>: condition<br>
 * <code>m_child3</code>: then part<br>
 * <code>m_child4</code>: else part or NULL<br>
 */

ReASIf::ReASIf() :
   ReASNode4(AST_IF) {
   m_flags |= NF_STATEMENT;
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASIf::check(ReParser& parser) {
   bool rc = true;
   if (m_child2 == NULL)
      rc = ensureError(parser, "'if' misses condition");
   else if (m_child2->checkAsCalculable("condition", ReASBoolean::m_instance,
                                        parser))
      rc = false;
   if (m_child3 != NULL && !checkStatementList(m_child3, parser))
      rc = false;
   if (m_child4 != NULL && !checkStatementList(m_child4, parser))
      rc = false;
   return rc;
}

/**
 * @brief Executes the statement.
 *
 * @return  0: continue the current statement list<br>
 *          n > 0: stop the n most inner statement lists (initialized by leave)
 *          n < 0: stop the -n most inner statement lists (initialized by continue)
 */
int ReASIf::execute(ReVMThread& thread) {
   int rc = 0;
   bool condition = calcAsBoolean(m_child2, thread);
   if (thread.tracing())
      thread.vm()->traceWriter()->format("if %s",
                                         condition ? "true" : "false");
   ReASItem* list = condition ? m_child3 : m_child4;
   if (list != NULL) {
      if ((rc = executeStatementList(list, thread)) != 0) {
         if (rc < 0)
            rc--;
         else if (rc > 0)
            rc++;
      }
   }
   return rc;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASIf::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent,
                         "If id: %d condition: %d then: %d else: %d succ: %d%s", m_id,
                         m_child2 == NULL ? 0 : m_child2->id(),
                         m_child3 == NULL ? 0 : m_child3->id(),
                         m_child4 == NULL ? 0 : m_child4->id(),
                         m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   m_child2->dump(writer, indent + 1);
   if (m_child3 != NULL)
      m_child3->dump(writer, indent + 1);
   if (m_child4 != NULL)
      m_child4->dump(writer, indent + 1);
}

/** @class ReASFor ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a for statement.
 *
 * The for statement has an initialization, a condition, a forwarding
 * statement and a body.
 * The initialization will be called first.
 * Then the condition will be tested. If true the body will be executed
 * and then the forwarding statement.
 *
 * <code>m_child</code>: next statement<br>
 * <code>m_child2</code>: body<br>
 * <code>m_child3</code>: iterator variable<br>
 * <code>m_child4</code>: container variable<br>
 */

/**
 * @brief Constructor.
 *
 * @param variable      NULL or the iterator variable
 */
ReASForIterated::ReASForIterated(ReASVarDefinition* variable) :
   ReASNode4(AST_ITERATED_FOR),
   ReASStatement() {
   m_flags |= NF_STATEMENT;
   m_child2 = variable;
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASForIterated::check(ReParser& parser) {
   RE_UNUSED(&parser);
   return false;
}

/**
 * @brief Executes the statement.
 *
 * @return  0: continue the current statement list<br>
 *          n > 0: stop the n most inner statement lists (initialized by leave)
 *          n < 0: stop the -n most inner statement lists (initialized by continue)
 */
int ReASForIterated::execute(ReVMThread& thread) {
   RE_UNUSED(&thread);
   return 0;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASForIterated::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent,
                         "forIt id: %d var: %d set: %d body: %d succ: %d %s", m_id,
                         m_child3 == NULL ? 0 : m_child3->id(),
                         m_child4 == NULL ? 0 : m_child4->id(),
                         m_child2 == NULL ? 0 : m_child2->id(),
                         m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   if (m_child3 != NULL)
      m_child3->dump(writer, indent + 1);
   if (m_child4 != NULL)
      m_child4->dump(writer, indent + 1);
   dumpStatements(writer, indent + 1, m_child2);
}

/** @class ReASForCounted ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a for statement.
 *
 * The for statement has an optional variable, an optional start value,
 * an end value, an optional step and a body.
 *
 * The start and end value will be calculated.
 * The body will be executed so many times given by the start and end value.
 *
 * <code>m_child</code>: next statement<br>
 * <code>m_child2</code>: body<br>
 * <code>m_child3</code>: variable or NULL<br>
 * <code>m_child4</code>: start value or NULL<br>
 * <code>m_child5</code>: end value<br>
 * <code>m_child6</code>: step value or NULL
 *
 */

/**
 * @brief Constructor.
 *
 * @param variable      NULL or the counter variable
 */
ReASForCounted::ReASForCounted(ReASVarDefinition* variable) :
   ReASNode6(AST_ITERATED_FOR),
   ReASStatement() {
   m_flags |= NF_STATEMENT;
   m_child3 = variable;
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASForCounted::check(ReParser& parser) {
   bool rc = true;
   ReASNamedValue* var = NULL;
   if (m_child3 != NULL) {
      var = dynamic_cast<ReASNamedValue*>(m_child3);
      if (!m_child3->check(parser))
         rc = false;
      if (var == NULL)
         rc = error(LOC_FORC_CHECK_1, parser, "not a variable: %s",
                    m_child3->nameOfItemType());
   }
   if (m_child4 != NULL
         && !m_child4->checkAsCalculable("start value", ReASInteger::m_instance,
                                         parser))
      rc = false;
   if (m_child5 != NULL
         && !m_child5->checkAsCalculable("end value", ReASInteger::m_instance,
                                         parser))
      rc = false;
   if (m_child6 != NULL
         && !m_child6->checkAsCalculable("step value", ReASInteger::m_instance,
                                         parser))
      rc = false;
   if (m_child2 != NULL && !checkStatementList(m_child2, parser))
      rc = false;
   return rc;
}

/**
 * @brief Executes the statement.
 *
 * @return  0: continue the current statement list<br>
 *          n > 0: stop the n most inner statement lists (initialized by leave)
 *          n < 0: stop the -n most inner statement lists (initialized by continue)
 */
int ReASForCounted::execute(ReVMThread& thread) {
   int rc = 0;
   ReASStatement* body = dynamic_cast<ReASStatement*>(m_child2);
   if (body == NULL)
      throw ReASException(
         m_child2 == NULL ? m_position : m_child2->position(),
         "forc statement: body is not a statement");
   int start = m_child4 == NULL ? 1 : calcAsInteger(m_child4, thread);
   int end = m_child5 == NULL ? 0 : calcAsInteger(m_child5, thread);
   int step = m_child6 == NULL ? 1 : calcAsInteger(m_child6, thread);
   ReASNamedValue* var =
      m_child3 == NULL ? NULL : dynamic_cast<ReASNamedValue*>(m_child3);
   if (thread.tracing())
      thread.vm()->traceWriter()->format("for %s from %d to %d step %d",
                                         var == NULL ? "?" : var->name().constData(), start, end, step);
   for (int ii = start; ii <= end; ii += step) {
      //@ToDo: assign to the variable
      int rc2 = body->execute(thread);
      if (rc2 != 0) {
         if (rc2 > 0) {
            // rc comes from "break";
            rc = rc2 - 1;
         } else {
            // rc comes from "continue";
            if (rc2 == -1)
               continue;
            else
               rc = rc2 + 1;
         }
         break;
      }
   }
   return rc;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASForCounted::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent,
                         "forC id: %d var: %d from: %d to: %d step: %d body: %d succ: %d %s",
                         m_id, m_child3 == NULL ? 0 : m_child3->id(),
                         m_child4 == NULL ? 0 : m_child4->id(),
                         m_child5 == NULL ? 0 : m_child5->id(),
                         m_child6 == NULL ? 0 : m_child6->id(),
                         m_child2 == NULL ? 0 : m_child2->id(),
                         m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   if (m_child3 != NULL)
      m_child3->dump(writer, indent + 1);
   if (m_child4 != NULL)
      m_child4->dump(writer, indent + 1);
   if (m_child5 != NULL)
      m_child5->dump(writer, indent + 1);
   if (m_child6 != NULL)
      m_child6->dump(writer, indent + 1);
   dumpStatements(writer, indent + 1, m_child2);
}

/** @class ReASWhile ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a while statement.
 *
 * The while statement has an a condition and a body.
 * The body will be executed while the condition returns true.
 *
 * <code>m_child</code>: next statement<br>
 * <code>m_child2</code>: condition<br>
 * <code>m_child3</code>: body<br>
 */

ReASWhile::ReASWhile() :
   ReASNode3(AST_WHILE),
   ReASStatement() {
   m_flags |= NF_STATEMENT;
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASWhile::check(ReParser& parser) {
   bool rc = true;
   if (m_child2 == NULL)
      ensureError(parser, "missing condition for 'while''");
   else
      rc = m_child2->checkAsCalculable("condition", ReASBoolean::m_instance,
                                       parser);
   if (m_child3 != NULL && !checkStatementList(m_child3, parser))
      rc = false;
   return rc;
}

/**
 * @brief Executes the statement.
 *
 * @return  0: continue the current statement list<br>
 *          n > 0: stop the n most inner statement lists (initialized by leave)
 *          n < 0: stop the -n most inner statement lists (initialized by continue)
 */
int ReASWhile::execute(ReVMThread& thread) {
   int rc = 0;
   ReASStatement* body = dynamic_cast<ReASStatement*>(m_child3);
   if (thread.tracing())
      thread.vm()->traceWriter()->write("while");
   while (calcAsBoolean(m_child2, thread)) {
      int rc2 = body->execute(thread);
      if (rc2 != 0) {
         if (rc2 > 0) {
            // rc comes from "break";
            rc = rc2 - 1;
         } else {
            // rc comes from "continue";
            if (rc2 == -1)
               continue;
            else
               rc = rc2 + 1;
         }
         break;
      }
   }
   return rc;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASWhile::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent,
                         "while id: %d condition: %d body: %d succ: %d %s", m_id,
                         m_child2 == NULL ? 0 : m_child2->id(),
                         m_child3 == NULL ? 0 : m_child3->id(),
                         m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   if (m_child2 != NULL)
      m_child2->dump(writer, indent + 1);
   dumpStatements(writer, indent + 1, m_child3);
}

/** @class ReASRepeat ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a while statement.
 *
 * The while statement has an a condition and a body.
 * The body will be executed while the condition returns true.
 *
 * <code>m_child</code>: next statement<br>
 * <code>m_child2</code>: condition<br>
 * <code>m_child3</code>: body<br>
 */

ReASRepeat::ReASRepeat() :
   ReASNode3(AST_REPEAT),
   ReASStatement() {
   m_flags |= NF_STATEMENT;
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASRepeat::check(ReParser& parser) {
   bool rc = true;
   if (m_child3 != NULL && !checkStatementList(m_child3, parser))
      rc = false;
   if (m_child2 == NULL)
      ensureError(parser, "missing condition for 'repeat''");
   else if (!m_child2->checkAsCalculable("condition", ReASBoolean::m_instance,
                                         parser))
      rc = false;
   return rc;
}

/**
 * @brief Executes the statement.
 *
 * Meaning of the childs:
 * m_child: body
 * m_child2: condition
 */
int ReASRepeat::execute(ReVMThread& thread) {
   int rc = 0;
   ReASStatement* body = dynamic_cast<ReASStatement*>(m_child3);
   if (thread.tracing())
      thread.vm()->traceWriter()->write("repeat");
   do {
      int rc2 = body->execute(thread);
      if (rc2 != 0) {
         if (rc2 > 0) {
            // rc comes from "break";
            rc = rc2 - 1;
         } else {
            // rc comes from "continue";
            if (rc2 == -1)
               continue;
            else
               rc = rc2 + 1;
         }
         break;
      }
   } while (!calcAsBoolean(m_child2, thread));
   return rc;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASRepeat::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent,
                         "repeat id: %d condition: %d body: %d succ: %d %s", m_id,
                         m_child2 == NULL ? 0 : m_child2->id(),
                         m_child3 == NULL ? 0 : m_child3->id(),
                         m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   if (m_child2 != NULL)
      m_child2->dump(writer, indent + 1);
   dumpStatements(writer, indent + 1, m_child3);
}

/** @class ReASClass ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements the base class of an Abstract Syntax Tree class.
 *
 * This class is abstract.
 */
/**
 * @brief Constructor.
 */
ReASClass::ReASClass(const QByteArray& name, ReASTree& tree) :
   m_name(name),
   m_symbols(NULL),
   m_superClass(NULL),
   m_tree(tree) {
}

/**
 * @brief Destructor.
 *
 * Does nothing but forces a virtual destructor of all derived classes.
 *
 */
ReASClass::~ReASClass() {
}

/**
 * @brief Return the class name.
 *
 * @return the class name
 */
const QByteArray& ReASClass::name() const {
   return m_name;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASClass::dump(ReWriter& writer, int indent) {
   writer.formatIndented(indent, "class %s super: %s", m_name.constData(),
                         m_superClass == NULL ? "<none>" : m_superClass->name().constData());
   m_symbols->dump(writer, indent);
}

/**
 * @brief Sets the symbol space from the current in the tree.
 */
void ReASClass::setSymbols() {
   m_symbols = m_tree.currentSpace();
}

/** @class ReASTree ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a manager for all parts of an Abstract Syntax Tree.
 *
 * It contains the global symbol space and maintainance a list of used modules.
 */

/**
 * @brief Constructor.
 */
ReASTree::ReASTree() :
   m_global(NULL),
   m_modules(),
   m_symbolSpaces(),
   m_currentSpace(NULL),
   m_store(128 * 1024) {
   init();
}

/**
 * @brief Destructor.
 */
ReASTree::~ReASTree() {
   destroy();
}

/**
 * @brief Initializes the instance.
 *
 * Used in the constructor and in clear.
 */
void ReASTree::init() {
   m_global = ReSymbolSpace::createGlobal(*this);
   m_symbolSpaces.append(m_global);
   m_currentSpace = m_global;
}

/**
 * @brief Frees the resources of the instance.
 */
void ReASTree::destroy() {
   SymbolSpaceMap::iterator it;
   for (it = m_symbolSpaceHeap.begin(); it != m_symbolSpaceHeap.end(); it++) {
      delete it.value();
   }
   m_symbolSpaceHeap.clear();
}
/**
 * @brief Returns the string storage of the instance.
 *
 * @return  the efficient allocator for C strings
 */
ReByteStorage& ReASTree::store() {
   return m_store;
}

/**
 * @brief Handles the start of a new module.
 *
 * @param name  the module's name
 * @return      true: the module is new<br>
 *              false: the module is yet known
 */
bool ReASTree::startModule(ReSourceUnitName name) {
   bool rc = m_modules.contains(name);
   if (!rc) {
      // freed in ~ReASTree()
      ReSymbolSpace* space = new ReSymbolSpace(ReSymbolSpace::SST_MODULE,
            name, m_global);
      m_symbolSpaceHeap[name] = space;
      m_modules[name] = space;
      m_symbolSpaces.append(space);
      m_currentSpace = space;
   }
   return rc;
}
/**
 * @brief Search for the symbol space of a given module.
 *
 * @param name  the module's name
 * @return      NULL: not found<br>
 *              otherwise: the symbol space of the module
 */
ReSymbolSpace* ReASTree::findmodule(const QByteArray& name) {
   ReSymbolSpace* rc = m_modules.contains(name) ? m_modules[name] : NULL;
   return rc;
}

/**
 * @brief Handles the end of a module.
 * @param name  the module's name
 */
void ReASTree::finishModule(ReSourceUnitName name) {
   ReSymbolSpace* top = m_symbolSpaces.at(m_symbolSpaces.size() - 1);
   if (top->name() != name)
      throw ReException("ReASTree::finishModule(): module is not top: %s",
                        name);
   else {
      m_symbolSpaces.removeLast();
      // "global" is always the bottom:
      m_currentSpace = m_symbolSpaces.at(m_symbolSpaces.size() - 1);
   }
}

/**
 * @brief Handles the start of a new class definition.
 * @param name      name of the class/method
 * @param type      the symbol space type
 * @return          the new symbol space
 */
ReSymbolSpace* ReASTree::startClassOrMethod(const QByteArray& name,
      ReSymbolSpace::SymbolSpaceType type) {
   // the stack m_modules is never empty because of "global" and modules.
   ReSymbolSpace* parent = m_symbolSpaces[m_symbolSpaces.size() - 1];
   QByteArray fullName = parent->name() + "." + name;
   // freed in ~ReASTree()
   ReSymbolSpace* space = new ReSymbolSpace(type, fullName, parent);
   m_symbolSpaceHeap[fullName] = space;
   m_symbolSpaces.append(space);
   m_currentSpace = space;
   return space;
}

/**
 * @brief Handles the end of a class definition.
 *
 * @param name  the name of the class (short form)
 */
void ReASTree::finishClassOrMethod(const QByteArray& name) {
   ReSymbolSpace* top = m_symbolSpaces.at(m_symbolSpaces.size() - 1);
   if (!top->name().endsWith("." + name))
      throw ReException("ReASTree::finishModule(): class is not top: %s",
                        name.constData());
   else {
      m_symbolSpaces.removeLast();
      // "global" is the bottom always!
      m_currentSpace = m_symbolSpaces.at(m_symbolSpaces.size() - 1);
   }
}

/**
 * @brief Returns the stack of the symbol spaces.
 *
 * @return the stack with the active symbol spaces
 */
ReASTree::SymbolSpaceStack& ReASTree::symbolSpaces() {
   return m_symbolSpaces;
}
/**
 * @brief Returns the current symbol space (top of the stack).
 *
 * @return the current symbol space
 */
ReSymbolSpace* ReASTree::currentSpace() const {
   return m_currentSpace;
}

/**
 * @brief Removes all content from the abstract syntax tree.
 */
void ReASTree::clear() {
   destroy();
   //m_global->clear();
   m_modules.clear();
   m_symbolSpaces.clear();
   init();
}

/**
 * @brief Writes the internals into a file.
 *
 * @param filename      filename
 * @param flags         what to dump: sum of DMP_... flags
 * @param header        NULL or a text put on the top
 */
void ReASTree::dump(const char* filename, int flags, const char* header) {
   ReFileWriter writer(filename);
   if (header != NULL)
      writer.writeLine(header);
   if (flags & DMP_GLOBALS) {
      m_global->dump(writer, 0, "=== Globals:");
   }
   if (flags & DMP_MODULES) {
      QList < QByteArray > sorted;
      sorted.reserve(m_modules.size());
      SymbolSpaceMap::iterator it;
      for (it = m_modules.begin(); it != m_modules.end(); it++) {
         sorted.append(it.key());
      }
      qSort(sorted.begin(), sorted.end(), qLess<QByteArray>());
      QList<QByteArray>::iterator it2;
      for (it2 = sorted.begin(); it2 != sorted.end(); it2++) {
         ReSymbolSpace* space = m_modules[*it2];
         space->dump(writer, 0);
      }
   }
   writer.close();
}

/** @class ReASMethodCall ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a method or function call for the Abstract Syntax Tree.
 *
 * <code>m_child</code>: next statement<br>
 * <code>m_child2</code>: argument list<br>
 * <code>m_child3</code>: parent (variable, field ...)
 */
/**
 * @brief Constructor.
 *
 * @param name      name of the method/function
 * @param parent    NULL: it is a function<br>
 *                  otherwise: the parent (variable, field ...)
 */

ReASMethodCall::ReASMethodCall(const QByteArray& name, ReASItem* parent) :
   ReASNode3(AST_METHOD_CALL),
   ReASStatement(),
   m_name(name),
   m_method(NULL) {
   m_flags |= NF_STATEMENT;
   m_child3 = parent;
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASMethodCall::check(ReParser& parser) {
   bool rc = true;
   ReASExprStatement* args = dynamic_cast<ReASExprStatement*>(m_child2);
   int argCount = 0;
   ReASMethod* method = m_method;
   ReASVarDefinition* params =
      dynamic_cast<ReASVarDefinition*>(method->child2());
   while (args != NULL && params != NULL) {
      argCount++;
      ReASCalculable* argExpr = dynamic_cast<ReASCalculable*>(args->child2());
      if (argExpr == NULL)
         rc = error(LOC_METHOD_CALL_CHECK_1, parser,
                    "argument %d misses expr", argCount);
      else {
         ReASNamedValue* var;
         ReASItem* param = params->child2();
         if (param == NULL
               || (var = dynamic_cast<ReASNamedValue*>(param)) == NULL)
            rc = error(LOC_MEHTOD_CALL_CHECK_2, parser,
                       "parameter %d misses named value: %s", argCount,
                       param == NULL ? "<null>" : param->nameOfItemType());
         else {
            // tryConversion() calls args->args->child2()->check()!
            ReASConversion* converter = ReASConversion::tryConversion(
                                           var->clazz(), args->child2(), parser, rc);
            if (rc && converter != NULL)
               args->setChild2(converter);
         }
      }
      args = dynamic_cast<ReASExprStatement*>(args->child());
      params = dynamic_cast<ReASVarDefinition*>(params->child());
   }
   if (args != NULL && params == NULL)
      rc = error(LOC_MEHTOD_CALL_CHECK_3, parser,
                 "too many arguments: %d are enough", argCount);
   else if (args == NULL && params != NULL && params->child3() != NULL)
      rc = error(LOC_MEHTOD_CALL_CHECK_4, parser,
                 "too few arguments: %d are not enough", argCount);
   return rc;
}

/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASMethodCall::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent,
                         "call %s Id: %d args: %d parent: %d succ: %d %s", m_name.constData(),
                         m_id, m_child2 == NULL ? 0 : m_child2->id(),
                         m_child3 == NULL ? 0 : m_child3->id(),
                         m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   if (m_child2 != NULL)
      m_child2->dump(writer, indent + 1);
   if (m_child3 != NULL)
      m_child3->dump(writer, indent + 1);
}

/**
 * @brief Executes the method call.
 *
 * @return  0: continue the current statement list
 */
int ReASMethodCall::execute(ReVMThread& thread) {
   int rc = 0;
   ReStackFrame frame(this, m_method->symbols());
   thread.pushFrame(&frame);
   ReASExprStatement* args = dynamic_cast<ReASExprStatement*>(m_child2);
   int ixArg = -1;
   while (args != NULL) {
      ixArg++;
      ReASCalculable* argExpr = dynamic_cast<ReASCalculable*>(args->child2());
      argExpr->calc(thread);
      ReASVariant& value = thread.popValue();
      ReASVariant& varValue = frame.valueOfVariable(ixArg);
      varValue.copyValue(value);
   }
   thread.popFrame();
   return rc;
}

ReASMethod* ReASMethodCall::method() const {
   return m_method;
}

/**
 * @brief Sets the method.
 * @param method    method to set
 */
void ReASMethodCall::setMethod(ReASMethod* method) {
   m_method = method;
}

/**
 * @brief Returns the argument list.
 *
 * @return  the first element of an argument list
 */
ReASExprStatement* ReASMethodCall::arg1() const {
   return dynamic_cast<ReASExprStatement*>(m_child2);
}

/** @class ReASException ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a call of a method or function.
 *
 * <code>m_child</code>: body
 * <code>m_child2</code>: argument list (or NULL)
 */

/** @class ReASBinaryOp ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements binary operator for the Abstract Syntax Tree.
 *
 * <code>m_child</code>: left operand<br>
 * <code>m_child2</code>: right operand
 */
/**
 * @brief Constructor.
 */
ReASBinaryOp::ReASBinaryOp() :
   ReASNode2(AST_BINARY_OP),
   m_operator(BOP_UNDEF) {
}

/**
 * @brief Calculates the binary operation.
 *
 * @param thread    IN/OUT: the bool value of the condition
 */
void ReASBinaryOp::calc(ReVMThread& thread) {
   if (isAssignment())
      assign(thread);
   else {
      ReASCalculable* op1 = dynamic_cast<ReASCalculable*>(m_child);
      ReASCalculable* op2 = dynamic_cast<ReASCalculable*>(m_child2);
      if (op1 == NULL || op2 == NULL)
         error(thread.logger(), LOC_BINOP_CALC_1, "operand is null: %d / %d",
               m_child == NULL ? 0 : m_child->id(),
               m_child2 == NULL ? 0 : m_child2->id());
      else {
         op1->calc(thread);
         op2->calc(thread);
         ReASVariant& val1 = thread.top2OfValues();
         ReASVariant& val2 = thread.topOfValues();
         switch (m_operator) {
         case BOP_PLUS:
            switch (val1.variantType()) {
            case ReASVariant::VT_FLOAT:
               val1.setFloat(val1.asFloat() + val2.asFloat());
               break;
            case ReASVariant::VT_INTEGER:
               val1.setInt(val1.asInt() + val2.asInt());
               break;
            case ReASVariant::VT_OBJECT:
            //if (val1.getClass() == ReASString::m_instance)
            default:
               error(thread.logger(), LOC_BINOP_CALC_2,
                     "invalid type for '+': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_MINUS:
            switch (val1.variantType()) {
            case ReASVariant::VT_FLOAT:
               val1.setFloat(val1.asFloat() - val2.asFloat());
               break;
            case ReASVariant::VT_INTEGER:
               val1.setInt(val1.asInt() - val2.asInt());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_3,
                     "invalid type for '-': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_TIMES:
            switch (val1.variantType()) {
            case ReASVariant::VT_FLOAT:
               val1.setFloat(val1.asFloat() * val2.asFloat());
               break;
            case ReASVariant::VT_INTEGER:
               val1.setInt(val1.asInt() * val2.asInt());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_4,
                     "invalid type for '*': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_DIV:
            switch (val1.variantType()) {
            case ReASVariant::VT_FLOAT:
               val1.setFloat(val1.asFloat() / val2.asFloat());
               break;
            case ReASVariant::VT_INTEGER:
               val1.setInt(val1.asInt() / val2.asInt());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_5,
                     "invalid type for '/': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_MOD:
            switch (val1.variantType()) {
            case ReASVariant::VT_FLOAT:
               val1.setFloat(fmod(val1.asFloat(), val2.asFloat()));
               break;
            case ReASVariant::VT_INTEGER:
               val1.setInt(val1.asInt() % val2.asInt());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_6,
                     "invalid type for '%': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_POWER:
            switch (val1.variantType()) {
            case ReASVariant::VT_FLOAT:
               val1.setFloat(fmod(val1.asFloat(), val2.asFloat()));
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_7,
                     "invalid type for '**': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_LOG_OR:
            switch (val1.variantType()) {
            case ReASVariant::VT_BOOL:
               val1.setBool(val1.asBool() || val2.asBool());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_8,
                     "invalid type for '||': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_LOG_AND:
            switch (val1.variantType()) {
            case ReASVariant::VT_BOOL:
               val1.setBool(val1.asBool() && val2.asBool());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_9,
                     "invalid type for '&&': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_LOG_XOR:
            switch (val1.variantType()) {
            case ReASVariant::VT_BOOL:
               val1.setBool(val1.asBool() != val2.asBool());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_9,
                     "invalid type for '^^': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_BIT_OR:
            switch (val1.variantType()) {
            case ReASVariant::VT_INTEGER:
               val1.setInt(val1.asInt() | val2.asInt());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_10,
                     "invalid type for '|': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_BIT_AND:
            switch (val1.variantType()) {
            case ReASVariant::VT_INTEGER:
               val1.setInt(val1.asInt() & val2.asInt());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_11,
                     "invalid type for '&': %s", val1.nameOfType());
               break;
            }
            break;
         case BOP_BIT_XOR:
            switch (val1.variantType()) {
            case ReASVariant::VT_INTEGER:
               val1.setInt(val1.asInt() ^ val2.asInt());
               break;
            default:
               error(thread.logger(), LOC_BINOP_CALC_12,
                     "invalid type for '^': %s", val1.nameOfType());
               break;
            }
            break;
         default:
            break;
         }
         thread.popValue();
      }
   }
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASBinaryOp::check(ReParser& parser) {
   RE_UNUSED(&parser);
   return false;
}

/**
 * @brief Returns the operator.
 *
 * @return the operator
 */
ReASBinaryOp::BinOperator ReASBinaryOp::getOperator() const {
   return m_operator;
}

/**
 * @brief Sets the operator.
 *
 * @param op    the operator
 */
void ReASBinaryOp::setOperator(BinOperator op) {
   m_operator = op;
}
/**
 * @brief Writes the internals into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASBinaryOp::dump(ReWriter& writer, int indent) {
   const QByteArray& opName = nameOfOp(m_operator);
   char buffer[256];
   writer.formatIndented(indent,
                         "BinOp id: %d op: %s (%d) left: %d right: %d %s", m_id,
                         opName.constData(), m_operator, m_child == NULL ? 0 : m_child->id(),
                         m_child2 == NULL ? 0 : m_child2->id(),
                         positionStr(buffer, sizeof buffer));
   if (indent < 32 && m_child != NULL)
      m_child->dump(writer, indent + 1);
   if (indent < 32 && m_child2 != NULL)
      m_child2->dump(writer, indent + 1);
}

/**
 * @brief Does an assignment.
 *
 * @param thread
 */
void ReASBinaryOp::assign(ReVMThread& thread) {
   ReASVariant& rValue = thread.lValue(m_child);
   ReASCalculable* expr = dynamic_cast<ReASCalculable*>(m_child2);
   if (expr == NULL)
      error(thread.logger(), LOC_BINOP_1, "not a calculable: id: %d",
            m_child2 == NULL ? 0 : m_child2->id());
   else {
      ReASVariant& value = thread.popValue();
      switch (m_operator) {
      case BOP_ASSIGN:
         break;
      case BOP_PLUS_ASSIGN:
         //switch(value.variantType()){
         //}
         break;
      case BOP_MINUS_ASSIGN:
      case BOP_TIMES_ASSIGN:
      case BOP_DIV_ASSIGN:
      case BOP_MOD_ASSIGN:
      case BOP_POWER_ASSIGN:
      case BOP_LOG_OR_ASSIGN:
      case BOP_LOG_AND_ASSIGN:
      case BOP_LOG_XOR_ASSIGN:
      case BOP_BIT_OR_ASSIGN:
      case BOP_BIT_AND_ASSIGN:
      case BOP_BIT_XOR_ASSIGN:
         break;
      default:
         break;
      }
      rValue.copyValue(value);
   }
}
/**
 * @brief Returns the name (a string) of a binary operator.
 *
 * @param op    operator to convert
 *
 * @return      the name of the operator
 */
const char* ReASBinaryOp::nameOfOp(ReASBinaryOp::BinOperator op) {
   const char* rc;
   switch (op) {
   case BOP_ASSIGN:
      rc = "=";
      break;
   case BOP_PLUS_ASSIGN:
      rc = "+=";
      break;
   case BOP_MINUS_ASSIGN:
      rc = "-=";
      break;
   case BOP_TIMES_ASSIGN:
      rc = "*=";
      break;
   case BOP_DIV_ASSIGN:
      rc = "/=";
      break;
   case BOP_MOD_ASSIGN:
      rc = "%=";
      break;
   case BOP_POWER_ASSIGN:
      rc = "**=";
      break;
   case BOP_LOG_OR_ASSIGN:
      rc = "||=";
      break;
   case BOP_LOG_AND_ASSIGN:
      rc = "&&=";
      break;
   case BOP_LOG_XOR_ASSIGN:
      rc = "^^=";
      break;
   case BOP_BIT_OR_ASSIGN:
      rc = "|=";
      break;
   case BOP_BIT_AND_ASSIGN:
      rc = "&=";
      break;
   case BOP_BIT_XOR_ASSIGN:
      rc = "^=";
      break;
   case BOP_LSHIFT_ASSIGN:
      rc = "<<=";
      break;
   case BOP_LOG_RSHIFT_ASSIGN:
      rc = ">>=";
      break;
   case BOP_ARTITH_RSHIFT_ASSIGN:
      rc = ">>>=";
      break;
   case BOP_PLUS:
      rc = "+";
      break;
   case BOP_MINUS:
      rc = "-";
      break;
   case BOP_TIMES:
      rc = "*";
      break;
   case BOP_DIV:
      rc = "/";
      break;
   case BOP_MOD:
      rc = "%";
      break;
   case BOP_POWER:
      rc = "**";
      break;
   case BOP_LOG_OR:
      rc = "||";
      break;
   case BOP_LOG_AND:
      rc = "&&";
      break;
   case BOP_LOG_XOR:
      rc = "^^";
      break;
   case BOP_BIT_OR:
      rc = "|";
      break;
   case BOP_BIT_AND:
      rc = "&";
      break;
   case BOP_BIT_XOR:
      rc = "^";
      break;
   case BOP_LSHIFT:
      rc = "";
      break;
   case BOP_LOG_RSHIFT:
      rc = ">>";
      break;
   case BOP_ARTITH_RSHIFT:
      rc = ">>>";
      break;
   case BOP_EQ:
      rc = "==";
      break;
   case BOP_NE:
      rc = "!=";
      break;
   case BOP_LE:
      rc = "<=";
      break;
   case BOP_LT:
      rc = "<";
      break;
   case BOP_GE:
      rc = ">=";
      break;
   case BOP_GT:
      rc = ">";
      break;
   default:
      throw ReException("unknown binary op %d", (int) op);
   }
   return rc;
}

/** @class ReASMethod ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements a method definition for the Abstract Syntax Tree.
 *
 * The special case "function" (a method without class) is included.
 *
 * <code>m_child</code>: body<br>
 * <code>m_child2</code>: parameter list (or NULL)
 */
/**
 * @brief Constructor.
 *
 * @param name      the method name
 * @param tree      the abstract syntax tree
 */
ReASMethod::ReASMethod(const QByteArray& name, ReASTree& tree) :
   ReASNode2(AST_METHOD),
   m_name(name),
   m_resultType(NULL),
   m_symbols(NULL),
   m_sibling(NULL),
   m_tree(tree),
   firstParamWithDefault(-1) {
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASMethod::check(ReParser& parser) {
   RE_UNUSED(&parser);
   return false;
}

/**
 * @brief Executes the statement.
 *
 * This method will be never called. Must exit: Otherwise the class is abstract.
 */
int ReASMethod::execute(ReVMThread& thread) {
   RE_UNUSED(&thread);
   return 0;
}

/**
 * @brief Writes the internals of the instance into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASMethod::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.indent(indent);
   writer.format("Method %s %s(",
                 m_resultType == NULL ? "<NoneType>" : m_resultType->name().constData(),
                 m_name.constData());
   ReSymbolSpace* parent = m_symbols->parent();
   writer.formatLine(") id: %d parent: %s args: %d body: %d %s", m_id,
                     parent == NULL ? "" : parent->name().constData(),
                     m_child2 == NULL ? 0 : m_child2->id(), m_child->id(),
                     positionStr(buffer, sizeof buffer));
   if (m_child2 != NULL)
      m_child2->dump(writer, indent + 1);
   dumpStatements(writer, indent + 1, m_child);
   m_symbols->dump(writer, indent + 1);
}
/**
 * @brief Returns the symbol space of the instance.
 *
 * @return  the symbol space
 */
ReSymbolSpace* ReASMethod::symbols() const {
   return m_symbols;
}

/**
 * @brief Sets the symbol space from the current of the tree.
 */
void ReASMethod::setSymbols() {
   m_symbols = m_tree.currentSpace();
}
/**
 * @brief Returns the name of the method
 * @return  the name
 */

const QByteArray& ReASMethod::name() const {
   return m_name;
}

/**
 * @brief Tests whether an other method has the same signature (parameterlist).
 * @param other     the method for comparison
 * @return          true: same signature<br>
 *                  false: otherwise
 */
bool ReASMethod::equalSignature(ReASMethod& other) const {
   bool rc = true;
   ReASExprStatement* args = dynamic_cast<ReASExprStatement*>(m_child2);
   ReASExprStatement* otherArgs =
      dynamic_cast<ReASExprStatement*>(other.child2());
   while (rc && (args != NULL || otherArgs != NULL)) {
      if (args == NULL || otherArgs == NULL)
         rc = false;
      else {
         ReASVarDefinition* def = dynamic_cast<ReASVarDefinition*>(args
                                  ->child2());
         ReASVarDefinition* defOther =
            dynamic_cast<ReASVarDefinition*>(otherArgs->child2());
         if (def->clazz() != defOther->clazz())
            rc = false;
      }
   }
   return rc;
}
/**
 * @brief Returns the next overloaded method.
 *
 * @return  NULL: no other method available<br>
 *          otherwise: the next method with the same name but another signature
 */
ReASMethod* ReASMethod::sibling() const {
   return m_sibling;
}

/**
 * @brief Sets the next overloaded method.
 *
 * @param sibling   another method with the same name but another signature
 */
void ReASMethod::setSibling(ReASMethod* sibling) {
   m_sibling = sibling;
}
/**
 * @brief Returns the index of the first parameter with a default value.
 *
 * @return  -1: no parameter has a default value<br>
 *          otherwise: the index of the first parameter with a default
 */
int ReASMethod::getFirstParamWithDefault() const {
   return firstParamWithDefault;
}
/**
 * @brief Sets the index of the first parameter with default value
 * @param value
 */
void ReASMethod::setFirstParamWithDefault(int value) {
   firstParamWithDefault = value;
}

/** @class ReASField ReASTree.hpp "expr/ReASTree.hpp"
 *
 * @brief Implements an class field for the Abstract Syntax Tree.
 *
 * <code>m_child</code>: parent (variable, field, method)
 */

/**
 * @brief Constructor.
 *
 * @param name      name of the field
 */
ReASField::ReASField(const QByteArray& name) :
   ReASNode1(AST_FIELD),
   m_name(name) {
}

/**
 * @brief Checks the correctness of the instance.
 *
 * @param parser    for error processing
 * @return          <code>true</code>: node is correct<br>
 *                  <code>false</code>: otherwise
 */
bool ReASField::check(ReParser& parser) {
   RE_UNUSED(&parser);
   return false;
}

/**
 * @brief Writes the internals of the instance into a file.
 *
 * @param writer    writes to output
 * @param indent    nesting level
 */
void ReASField::dump(ReWriter& writer, int indent) {
   char buffer[256];
   writer.formatIndented(indent, "field %s id: %d parent: %d succ: %s",
                         m_name.constData(), m_id, m_child == NULL ? 0 : m_child->id(),
                         positionStr(buffer, sizeof buffer));
   m_child->dump(writer, indent + 1);
}

