/*
 * ReVM.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef ReVM_HPP
#define ReVM_HPP

class ReVMException: public ReException {
public:
   ReVMException(const char* message, ...);
};
class ReStackFrame {
public:
   ReStackFrame(ReASItem* caller, ReSymbolSpace* symbols);
   ~ReStackFrame();
public:
   ReASVariant& valueOfVariable(int index);
   ReSymbolSpace* symbols() const;

private:
   int m_countVariables;
   ReASVariant* m_variables;
   ReSymbolSpace* m_symbols;
   ReASItem* m_caller;
};

class ReVirtualMachine;
class ReVMThread {
   friend class ReASItem;
   friend class ReASStatement;
   friend class ReASCalculable;
   friend class ReASCondition;
public:
   typedef QList<ReStackFrame*> StackFrameList;
public:
   ReVMThread(int maxStack, ReVirtualMachine* vm);
public:
   void execute(ReASNode1* statements, ReSymbolSpace* space);
   virtual void debug(ReASNode1* statement);
   ReWriter* errorWriter() const;
   void setErrorWriter(ReWriter* errorWriter);
   ReLogger* logger() const;
   ReASVariant& reserveValue();
   ReASVariant& topOfValues();
   ReASVariant& top2OfValues();
   ReASVariant& popValue();
   void valueToTop(ReSymbolSpace* symbolSpace, int variableNo);
   ReASVariant& lValue(ReASItem* item);
   ReASVariant& valueOfVariable(ReSymbolSpace* symbolSpace, int variableNo);
   bool tracing() const;
   void setTracing(bool tracing);
   ReVirtualMachine* vm() const;
   void pushFrame(ReStackFrame* frame);
   void popFrame();

protected:
   int m_id;
   bool m_debugMode;
   bool m_singleStep;
   bool m_tracing;
   int m_maxStack;
   StackFrameList m_frameStack;
   int m_topOfFrames;
   QList<ReASVariant*> m_valueStack;
   int m_topOfValues;
   ReVirtualMachine* m_vm;
   ReLogger* m_logger;
private:
   static int m_nextId;
};

class ReVirtualMachine {
public:
   enum VMFlag {
      VF_UNDEF,
      VF_TRACE_STATEMENTS = 1 << 1,
      VF_TRACE_LOCALS = 1 << 2,
      VF_TRACE_AUTO_VARIABLES = 1 << 3

   };
   typedef QList<const char*> LineList;
public:
   ReVirtualMachine(ReASTree& tree, ReSource& source, int maxStack = 1024);
public:
   void executeModule(const char* module);
   void addThread(ReASItem* initialization, ReSymbolSpace* spaceInitialization,
                  ReASItem* statements, ReSymbolSpace* space, int maxStack = 0);
   bool hasFlag(VMFlag flag) const;
   void setFlag(VMFlag flag);
   void clearFlag(VMFlag flag);
   ReWriter* traceWriter() const;
   void setTraceWriter(ReWriter* traceWriter);
   ReASTree& tree() const;

private:
   int m_maxStack;
   QList<ReVMThread*> m_threads;
   int m_flags;
   ReSource& m_source;
   ReASTree& m_tree;
   LineList m_trace;
   ReWriter* m_traceWriter;
};

#endif // ReVM_HPP
