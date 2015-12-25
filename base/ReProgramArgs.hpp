/*
 * ReProgramArgs.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REPROGRAMARGS_H_
#define REPROGRAMARGS_H_

class ReProgramArgs;
/** All errors will reported by this exception.
 */
class ReOptionException: public ReException {
public:
   ReOptionException(ReProgramArgs* caller, const char* format, ...);
   ReOptionException(ReProgramArgs* caller, const QString& message);
};

class ReProgOption;
typedef QMap<QByteArray, ReProgOption*> ReProgOptionMap;

/**
 * This class analyses the program arguments and give an interface for retrieving them.
 *
 * Program arguments contains the program name, possibly options and "true" arguments.
 * <p>Options are short name options or long name options.</p>
 * <p>A short name option is preceded by a single dash ('-'), a long name option starts with two dashes: (--).</p>
 * <p>There are three types of options: boolean, integer and string.</p>
 * <p>Every option must have an default value.</p>
 * <p>A boolean option has normally a default value <code>false</code>. If it appears in the arguments
 * it will be have the value <code>true</code>.</p>
 * <p>An integer or string option can be followed by an integer or string value:<br>
 * Short name option: -x value or -xvalue<br>
 * Long name option: -xxx=value</p>
 * <p>The program must contain a definition of the options: <code>addInt(), addBool() and/or addString()</code>,
 * the analyse (<code>init()</code>) and the retrieval (<code>getInt(), getBool() and/or getString()</code>.</p>
 * <p>The connection between definition and retrieval are names.</p>
 */
class ReProgramArgs {
public:
   static const char* PREFIX_LINE_OPTION;
   static QByteArray UNDEFINED_STRING;
public:
   enum DataType {
      DT_UNDEF,
      DT_INT,
      DT_BOOL,
      DT_STRING,
      DT_STRING_EMPTY
   };
   enum {
      UNDEF_SHORT_NAME = 0
   };
public:
   ReProgramArgs(const char* usageList[], const char* examples[] = NULL);
   ReProgramArgs(const char* usageString, const char* examples = NULL);
   ReProgramArgs(const QString& usageString,
                 QString examples = ReQStringUtils::m_empty);
   virtual ~ReProgramArgs();
public:
   void addBool(const char* name, const char* description, char shortOpt,
                const char* longOpt, bool defaultVal);
   void addInt(const char* name, const char* description, char shortOpt,
               const char* longOpt, int defaultVal);
   void addString(const char* name, const char* description, char shortOpt,
                  const char* longOpt, bool mayBeEmpty, const char* defaultVal);
   int argCount() const;
   const char* arg(size_t index) const;
   const char** args() const;
   bool getBool(const char* name);
   int getInt(const char* name);
   const char* getString(const char* name, QByteArray& buffer);
   void help(const char* message, bool issueLastError,
             QByteArrayList& lines) const;
   void help(const char* message, bool issueLastError = false, FILE* stream = stdout) const;
   void init(int argc, const char* argv[], bool hasProgram);
   inline void init(int argc, char* argv[]) {
      return init(argc, (const char**) argv, true);
   }
   const char* programName() const;
   void setLastError(const char* message);
   void setLastError(const QByteArray& message);
   void setUsage(const char* usage[]);
   void setProgramName(const QByteArray& program);
   QByteArray shift();
   const char* typeToString(DataType dataType);
private:
   void addProperties(const char* name, const char* description, char shortOpt,
                      const char* longOpt, DataType dataType, const char* defaultValue);
   void analyseLong(const char* opt);
   bool analyseShort(const char* opt, const char* nextArg);
   ReProgOption* search(char shortName, const char* longName);
   void setValue(const char* name, const char* value, DataType dataType);
protected:
   QByteArrayList m_usage;
   QByteArrayList m_examples;
   ReProgOptionMap  m_options;
   const char** m_args;
   int m_argCount;
   const char* m_program;
   QByteArray m_lastError;
};

class ReProgOption {
public:
   ReProgramArgs::DataType m_type;
   QByteArray m_name;
   QByteArray m_longName;
   char m_shortName;
   QByteArray m_value;
   QByteArray m_defaultValue;
   QByteArray m_description;
};

#endif /* REPROGRAMARGS_H_ */
