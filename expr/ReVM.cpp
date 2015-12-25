/*
 * ReVM.cpp
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
 * @brief Implements an interpreter of an abstract syntax tree.
 */

/** @file expr/ReVM.hpp
 *
 * @brief Definitions for an interpreter of an abstract syntax tree.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

enum {
   LOC_VAL_OF_VAR_1 = LOC_FIRST_OF(LOC_VM), // 11401
   LOC_UNOP_1,
   LOC_UNOP_2,
   LOC_UNOP_3,
   LOC_UNOP_4, // 10005
   LOC_BINOP_1,
   LOC_COUNT
};

int ReVMThread::m_nextId = 1;

/** @class ReVMException ReVM.hpp "expr/ReVM.hpp"
 *
 * @brief Implements an exception for the virtual machine.
 *
 */
/**
 * @brief Constructor
 * @param format    the message with placeholders
 * @param ...       the values for the placeholders
 */
ReVMException::ReVMException(const char* format, ...) :
   ReException("") {
   char buffer[16000];
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   m_message = buffer;
}

/** @class ReStackFrame ReVM.hpp "expr/ReVM.hpp"
 *
 * @brief Implements the storage for a symbol space.
 *
 * The owner of a symbol space can be "global", a module, a class, or a method.
 * Some symbol spaces have more than one stack frame, e.g. a recursive called
 * method.
 */

/**
 * @brief Constructor.
 *
 * @param caller	name of the caller (for debugging)
 * @param symbols   the symbol space belonging to the stack frame
 */

ReStackFrame::ReStackFrame(ReASItem* caller, ReSymbolSpace* symbols) :
   m_countVariables(symbols->listOfVars().size()),
   m_variables(NULL),
   m_symbols(symbols),
   m_caller(caller) {
   if (m_countVariables > 0)
      m_variables = new ReASVariant[m_countVariables];
}

/**
 * @brief Destructor.
 */
ReStackFrame::~ReStackFrame() {
   delete[] m_variables;
   m_variables = NULL;
}

/**
 * @brief Returns the storage of a variable given by the index.
 *
 * @param index the index of the variable
 *
 * @return      the storage of the variable
 */
ReASVariant& ReStackFrame::valueOfVariable(int index) {
   if (index < 0 || index >= m_countVariables)
      throw ReVMException("valueOfVariable(): invalid index: %d", index);
   return m_variables[index];
}
/**
 * @brief Returns the symbol space of the frame.
 *
 * @return  the symbol space
 */
ReSymbolSpace* ReStackFrame::symbols() const {
   return m_symbols;
}

/** @class ReVMThread ReVM.hpp "expr/ReVM.hpp"
 *
 * @brief Implements a thread of the virtual machine.
 *
 * The virtual machine can execute many threads at the same time.
 * Each thread has its own stack.
 */

/**
 * @brief Constructor.
 *
 * @param maxStack  the maximal number of nested stack frames
 * @param vm        the parent, the virtual machine
 */
ReVMThread::ReVMThread(int maxStack, ReVirtualMachine* vm) :
   m_id(m_nextId++),
   m_debugMode(false),
   m_singleStep(false),
   m_tracing(false),
   m_maxStack(maxStack),
   m_frameStack(),
   // the stack is never empty!
   m_topOfFrames(0),
   m_valueStack(),
   // the stack is never empty!
   m_topOfValues(0),
   m_vm(vm),
   m_logger(new ReLogger()) {
   QByteArray prefix = "vm_thread_" + QByteArray::number(m_id);
   m_logger->buildStandardAppender(prefix);
   m_frameStack.reserve(maxStack);
   // the stack is never empty!
   //@ToDo:
   //m_frameStack.append(new ReStackFrame(vm->tree().symbolSpaces()[0]));
   throw ReNotImplementedException("ReVMThread::ReVMThread");
   // the stack is never empty!
   m_valueStack.append(new ReASVariant);
}

/**
 * @brief Executes a statement list.
 *
 * @param statements    the first statement of a statement list chained by
 *                      <code>m_child</code>
 * @param space         the current symbol space
 */
void ReVMThread::execute(ReASNode1* statements, ReSymbolSpace* space) {
   RE_UNUSED(space);
   bool debugMode = m_debugMode;
   ReASNode1* next;
   while (statements != NULL) {
      if (debugMode
            && (m_singleStep
                || (statements->flags() & ReASItem::NF_BREAKPOINT) != 0))
         debug(statements);
      ReASStatement* statement = dynamic_cast<ReASStatement*>(statements);
      if (statement != NULL)
         statement->execute(*this);
      statements = dynamic_cast<ReASNode1*>(statements->child());
   }
}

/**
 * @brief Handles a debugger break.
 *
 * @param statement the current statement (not yet executed)
 */
void ReVMThread::debug(ReASNode1* statement) {
}

/**
 * @brief Returns the logger of the instance.
 *
 * @return  the logger
 */
ReLogger* ReVMThread::logger() const {
   return m_logger;
}
/**
 * @brief Reserves a value in the thread's value stack.
 *
 * @return  the reserved value
 */
ReASVariant& ReVMThread::reserveValue() {
   ReASVariant* rc;
   if (++m_topOfValues < m_valueStack.size()) {
      rc = m_valueStack[m_topOfValues];
      rc->destroyValue();
   } else {
      rc = new ReASVariant();
      m_valueStack.append(rc);
   }
   return *rc;
}

/**
 * @brief Returns the top of the value stack.
 *
 * @return  the top of the value stack
 */
ReASVariant& ReVMThread::topOfValues() {
   ReASVariant& rc = *m_valueStack[m_topOfValues];
   return rc;
}

/**
 * @brief Returns the entry under the top of the value stack.
 *
 * @return  the 2nd value the value stack
 */
ReASVariant& ReVMThread::top2OfValues() {
   ReASVariant& rc = *m_valueStack[m_topOfValues - 1];
   return rc;
}

/**
 * @brief Returns the top of stack and removes it.
 *
 * @return the old top of stack
 */
ReASVariant& ReVMThread::popValue() {
   ReASVariant& rc = *m_valueStack[m_topOfValues];
   if (m_topOfValues > 0)
      m_topOfValues--;
   return rc;
}

/**
 * @brief Copies a variable value to the top of the stack.
 *
 * @param symbolSpace   the symbol space of the variable
 * @param variableNo    the current no of the variable in the symbol space
 */
void ReVMThread::valueToTop(ReSymbolSpace* symbolSpace, int variableNo) {
   //@ToDo
}

/**
 * @brief Returns the "left value" (of an assignment).
 * @param item
 * @return
 */
ReASVariant& ReVMThread::lValue(ReASItem* item) {
   ReASVariant* rc;
   switch (item->nodeType()) {
   case AST_NAMED_VALUE: {
      ReASNamedValue* var = dynamic_cast<ReASNamedValue*>(item);
      rc = &valueOfVariable(var->symbolSpace(), var->variableNo());
      break;
   }
   default:
      break;
   }
   return *rc;
}

/**
 * @brief Returns the reference of the value of a variable.
 *
 * @param symbolSpace   the symbol space
 * @param variableNo    the current number in the symbol space
 * @return
 */
ReASVariant& ReVMThread::valueOfVariable(ReSymbolSpace* symbolSpace,
      int variableNo) {
   ReASVariant& rc = *m_valueStack[0];
   int ix = m_topOfFrames;
   ReStackFrame* frame = NULL;
   for (int ix = m_topOfFrames; ix >= 0; ix--) {
      frame = m_frameStack[ix];
      if (frame->symbols() == symbolSpace) {
         rc = frame->valueOfVariable(variableNo);
         break;
      }
   }
   if (frame == NULL)
      m_logger->logv(LOG_ERROR, LOC_VAL_OF_VAR_1,
                     "no frame has symbolspace %s", symbolSpace->name().constData());
   return rc;
}
/**
 * @brief Returns whether each execution step should be dumped.
 * @return  true: tracing is on<br>
 *          false: otherwise
 */
bool ReVMThread::tracing() const {
   return m_tracing;
}

/**
 * @brief Sets the tracing flag: if set each execution step is dumped.
 * @param tracing   true: tracing will be done<br>
 *                  false: tracing will not be done
 */
void ReVMThread::setTracing(bool tracing) {
   m_tracing = tracing;
}

/**
 * @brief Returns the parent, a virtual machine.
 *
 * @return  the virtual machine
 */
ReVirtualMachine* ReVMThread::vm() const {
   return m_vm;
}

/**
 * @brief Adds a frame to the frame stack.
 *
 * @param frame     frame to add
 */
void ReVMThread::pushFrame(ReStackFrame* frame) {
   if (m_frameStack.size() >= m_maxStack)
      throw ReASException(NULL, "too deep recursion: %d", m_maxStack);
   m_frameStack.push_back(frame);
}

/**
 * @brief Removes the top of the frames from the stack.
 */
void ReVMThread::popFrame() {
   if (m_frameStack.size() <= 0)
      throw ReASException(NULL, "frame stack is empty");
   m_frameStack.pop_back();
}

/** @class ReVirtualMachine ReVM.hpp "expr/ReVM.hpp"
 *
 * @brief Implements a virtual machine.
 *
 * This is an execution unit which interprets an abstract syntax tree.
 */
ReVirtualMachine::ReVirtualMachine(ReASTree& tree, ReSource& source,
                                   int maxStack) :
   m_maxStack(maxStack),
   m_threads(),
   m_flags(VF_UNDEF),
   m_source(source),
   m_tree(tree),
   m_trace() {
   m_threads.reserve(8);
   m_trace.reserve(1024);
}

/**
 * @brief Executes the program in a module.
 *
 * @param module    the module's name
 */
void ReVirtualMachine::executeModule(const char* module) {
   ReSymbolSpace* space = m_tree.findmodule(module);
   if (space == NULL)
      throw ReVMException("module not found: %s", module);
   //@ToDo:
   //ReStackFrame frame(space);
   throw ReNotImplementedException("ReVirtualMachine::executeModule");
   ReSymbolSpace* mainSpace = NULL;
   ReASItem* mainStatements = NULL;
   ReASMethod* method = space->findMethod("main");
   if (method != NULL) {
      mainStatements = method->child();
      mainSpace = method->symbols();
   }
   addThread(space->body(), space, mainStatements, mainSpace);
}

/**
 * @brief Adds a thread to the instance.
 *
 * @param initialization        the statements for initialization
 * @param spaceInitialization   the symbol space of the initialization
 * @param statements            the statement list to execute. This is normally
 *                              the body of the main program
 * @param space                 the symbol space of the statements, normally
 *                              the symbol space of the main program
 * @param maxStack  the maximal number of nested stack frames.
 *                  <= 0: use the default
 */
void ReVirtualMachine::addThread(ReASItem* initialization,
                                 ReSymbolSpace* spaceInitialization, ReASItem* statements,
                                 ReSymbolSpace* space, int maxStack) {
   ReVMThread* thread = new ReVMThread(maxStack <= 0 ? m_maxStack : maxStack,
                                       this);
   m_threads.append(thread);
   if (initialization != NULL) {
      thread->execute(dynamic_cast<ReASNode1*>(initialization),
                      spaceInitialization);
   }
   if (statements != NULL)
      thread->execute(dynamic_cast<ReASNode1*>(statements), space);
}
/**
 * @brief Tests whether a given flag is set.
 *
 * @param flag      flag to test
 * @return          true: the flag is set<br>
 *                  false: otherwise
 */
bool ReVirtualMachine::hasFlag(ReVirtualMachine::VMFlag flag) const {
   bool rc = (m_flags & flag) != 0;
   return rc;
}
/**
 * @brief Adds a flag to the current flags.
 *
 * @param flag
 * @return
 */
void ReVirtualMachine::setFlag(ReVirtualMachine::VMFlag flag) {
   m_flags |= flag;
}

/**
 * @brief Adds a flag to the current flags.
 *
 * @param flag
 * @return
 */
void ReVirtualMachine::clearFlag(ReVirtualMachine::VMFlag flag) {
   m_flags &= ~flag;
}

/**
 * @brief Returns the trace writer.
 * @return  the trace writer
 */
ReWriter* ReVirtualMachine::traceWriter() const {
   return m_traceWriter;
}

/**
 * @brief Sets the trace writer.
 *
 * @param traceWriter   the new writer
 */
void ReVirtualMachine::setTraceWriter(ReWriter* traceWriter) {
   m_traceWriter = traceWriter;
}
/**
 * @brief Returns the abstract symbol tree.
 *
 * @return  the abstract symbol tree
 */
ReASTree& ReVirtualMachine::tree() const {
   return m_tree;
}

