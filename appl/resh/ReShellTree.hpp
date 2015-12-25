/*
 * ReShellTree.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef RESHELLTREE_HPP
#define RESHELLTREE_HPP

class ReShellStatement {
public:
   enum StatementFlags {
      SF_UNDEF,
      SF_INTERNAL_CMD = 1,
      SF_VAR_EXPANSION = 2,
      SF_INTERPOLATION = 4,
      SF_INP_REDIRECT = 8,
      SF_OUT_REDIRECT = 16,
      SF_SHELL_SCRIPT = 32,
      SF_OUT_TO_STRING = 64,
   };
public:
   ReSource* source;
   ReShellStatement* m_next;
   int m_flags;
};

class ReShellIf : public ReShellStatement {
public:
   ReShellStatement* m_condition;
   ReShellStatement* m_then;
   ReShellStatement* m_else;
};

class ReShellWhile : public ReShellStatement {
public:
   ReShellStatement* m_condition;
   ReShellStatement* m_body;
};

class ReCaseAlternative {
public:
   QByteArray m_value;
   ReShellStatement* m_statement;
   ReCaseAlternative* m_next;
};


class ReShellCase : public ReShellStatement {
protected:
   ReCaseAlternative* m_condition;
   ReShellStatement* m_body;
};

class ReShellFunction {
public:
   QByteArray m_name;
   ReShellStatement* m_body;
};


class ReShellTree {
public:
   ReShellTree();
public:
   QList<ReShellFunction> m_functions;
   ReShellStatement m_body;
};

#endif // RESHELLTREE_HPP
