/*
 * ReProgramArgs.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"

const char* ReProgramArgs::PREFIX_LINE_OPTION = "   ";
QByteArray ReProgramArgs::UNDEFINED_STRING("\01");

/** @brief Constructor.
 *
 * @param caller	the object which throw the exception.
 * @param format	the error message with placeholders like sprintf()
 * @param ...
 */
ReOptionException::ReOptionException(ReProgramArgs* caller, const char* format,
                                     ...) :
   ReException() {
   char buffer[64*1024];
   va_list ap;
   va_start(ap, format);
   qvsnprintf(buffer, sizeof buffer, format, ap);
   va_end(ap);
   m_message = buffer;
   if (caller != NULL)
      caller->setLastError(m_message);
}

/** @brief Constructor.
 *
 * @param caller	the object which throw the exception
 * @param message	the error message
*/
ReOptionException::ReOptionException(ReProgramArgs* caller, const QString& message) :
   ReException() {
   m_message = I18N::s2b(message);
   if (caller != NULL)
      caller->setLastError(m_message);
}

/** @brief Constructor.
 *
 * @param usageList	a string array with the description of the usage.
 * 					Every string will be issued in a separate line
 * @param examples	a string with one ore more calling examples.
 * 					Every string will be issued in a separate line
 */
ReProgramArgs::ReProgramArgs(const char* usageList[], const char* examples[]) :
   m_usage(),
   m_examples(),
   m_options(),
   m_args(NULL),
   m_argCount(0),
   m_program(NULL),
   m_lastError(ReStringUtils::m_empty) {
   QByteArray line;
   for (const char** argv = usageList; *argv != NULL; argv++) {
      line = *argv;
      ReStringUtils::chomp(line);
      m_usage.append(line.constData());
   }
   if (examples != NULL) {
      for (const char** argv = examples; *argv != NULL; argv++) {
         if (strncmp(*argv, "$0", 2) != 0)
            m_examples.append(*argv);
         else {
            QByteArray line;
            line.append(m_program, -1);
            m_examples.append(line.constData() + 2);
         }
      }
   }
}

/** @brief Constructor.
 *
 * @param usageString	a string with the description of the usage
 *						It may contain <code>'\\n'</code> for separate lines
 * @param examples		a string with one ore more calling examples.
 *						It may contain <code>'\\n'</code> for separate lines
 */
ReProgramArgs::ReProgramArgs(const char* usageString, const char* examples) :
   m_usage(),
   m_examples(),
   m_options(),
   m_args(NULL),
   m_argCount(0),
   m_program("?"),
   m_lastError() {
   m_usage = QByteArray(usageString).split('\n');
   if (examples != NULL) {
      if (strstr(examples, "$0") == NULL)
         m_examples = QByteArray(examples).split('\n');
      else {
         m_examples = QByteArray(examples).split('\n');
      }
   }
}

/** @brief Constructor.
 *
 * @param usageString	a string with the description of the usage
 *						It may contain <code>'\\n'</code> for separate lines
 * @param examples		a string with one ore more calling examples.
 *						It may contain <code>'\\n'</code> for separate lines
 */
ReProgramArgs::ReProgramArgs(const QString& usageString, QString examples) :
   m_usage(),
   m_examples(),
   m_options(),
   m_args(NULL),
   m_argCount(0),
   m_program("?"),
   m_lastError() {
   m_usage = I18N::s2b(usageString).split('\n');
   if (! examples.isEmpty()) {
      m_examples = I18N::s2b(examples).split('\n');
   }
}
/** @brief Destructor.
 */
ReProgramArgs::~ReProgramArgs() {
}

/**
 * Sets the usage message.
 *
 * @param usage     a vector of lines without '\n'
 */
void ReProgramArgs::setUsage(const char* usage[]) {
   m_usage.clear();
   for (int ix = 0; usage[ix] != NULL; ix++)
      m_usage.append(usage[ix]);
}

/**
 * Sets the program name.
 *
 * @param program	the program name
 */
void ReProgramArgs::setProgramName(const QByteArray& program) {
   m_program = program;
}

/**
 * Gets the first program argument and remove it from the other args.
 *
 * @return	"": no more arguments<br>
 *			the first argument
 */
QByteArray ReProgramArgs::shift() {
   QByteArray rc;
   if (m_argCount > 0) {
      rc = m_args[0];
      m_argCount--;
      m_args++;
   }
   return rc;
}

/**
 * Converts a type into a string.
 *
 * @param dataType	type to convert
 * @return			the type as string
 */
const char* ReProgramArgs::typeToString(ReProgramArgs::DataType dataType) {
   const char* rc;
   static QByteArray s_buffer;
   switch(dataType) {
   case DT_BOOL:
      rc = "bool";
      break;
   case DT_INT:
      rc = "int";
      break;
   case DT_STRING:
      rc = "string";
      break;
   case DT_STRING_EMPTY:
      rc = "string(empty)";
      break;
   default:
      s_buffer = "unknown type (" + QByteArray::number(dataType) + ")";
      rc = s_buffer.constData();
      break;
   }
   return rc;
}
/** @brief Puts the property infos into the property string.
 *
 * The <strong>property string</strong> is a string stored in the hashlist.
 * It contains all infos about the option but the current value.
 *
 * @param name			the name of the option. Used in the methods <code>getInt()</code>, ...
 * @param description	a short description of the option. Used in the user messages
 * @param shortOpt		the one character option identifier. Used in the arguments. Must be preceded by '-'
 * @param longOpt		the multi character option identifier.  Used in the arguments. Must be preceded by '--'
 * @param dataType		the data type of the option: DT_INT, DT_BOOL ...
 * @param defaultValue	the default value of the option
 * @param lengthValue	the length of <code>defaultValue</code>
 */
void ReProgramArgs::addProperties(const char* name, const char* description,
                                  char shortOpt, const char* longOpt, DataType dataType,
                                  const char* defaultValue) {
   ReProgOptionMap::const_iterator it;
   for (it = m_options.cbegin(); it != m_options.cend(); ++it) {
      if (name == it.key()) {
         throw ReOptionException(this, QObject::tr("name defined twice: %1").arg(name));
      } else {
         ReProgOption* opt = it.value();
         if (shortOpt != 0 && shortOpt == opt->m_shortName)
            throw ReOptionException(this, QObject::tr("short option defined twice: %1").arg(shortOpt));
         else if (opt->m_longName == longOpt)
            throw ReOptionException(this, QObject::tr("long option defined twice: %1").arg(longOpt));
      }
   }
   ReProgOption* opt = new ReProgOption;
   opt->m_name = name;
   opt->m_longName = longOpt;
   opt->m_defaultValue = defaultValue;
   opt->m_shortName = shortOpt;
   opt->m_type = dataType;
   opt->m_description = description;
   // Mark current value as default:
   opt->m_value = defaultValue;
   m_options[name] = opt;
}

/** @brief Adds an option with a boolean value.
 *
 * @param name			the name of the option. Used in the methods <code>getBool()</code>
 * @param description	a short description of the option. Used in the user messages
 * @param shortOpt		the one character option identifier. Used in the arguments. Must be preceded by '-'
 * @param longOpt		the multi character option identifier.  Used in the arguments. Must be preceded by '--'
 * @param defaultValue	the default value of the option.
 *
 * @see getBool()
 */
void ReProgramArgs::addBool(const char* name, const char* description,
                            char shortOpt, const char* longOpt, bool defaultValue) {
   addProperties(name, description, shortOpt, longOpt, DT_BOOL,
                 defaultValue ? "t" : "f");
}

/** @brief Adds an option with an integer value.
 *
 * @param name			the name of the option. Used in the methods <code>getInt()</code>
 * @param description	a short description of the option. Used in the user messages
 * @param shortOpt		the one character option identifier. Used in the arguments. Must be preceded by '-'
 * @param longOpt		the multi character option identifier.  Used in the arguments. Must be preceded by '--'
 * @param defaultValue	the default value of the option
 *
 * * @see getInt()
 */
void ReProgramArgs::addInt(const char* name, const char* description,
                           char shortOpt, const char* longOpt, int defaultValue) {
   QByteArray number = QByteArray::number(defaultValue);
   addProperties(name, description, shortOpt, longOpt, DT_INT, number.constData());
}

/** @brief Adds an option with a string value.
 *
 * @param name			the name of the option. Used in the methods <code>getString()</code>
 * @param description	A short description of the option. Used in the user messages
 * @param shortOpt		the one character option identifier. Used in the arguments. Must be preceded by '-'
 * @param longOpt		the multi character option identifier.  Used in the arguments. Must be preceded by '--'
 * @param mayBeEmpty	<code>false</code>: an empty string is an error
 * @param defaultValue	the default value of the option
 *
 * @see getString()
 */
void ReProgramArgs::addString(const char* name, const char* description,
                              char shortOpt, const char* longOpt, bool mayBeEmpty,
                              const char* defaultValue) {
   addProperties(name, description, shortOpt, longOpt,
                 mayBeEmpty ? DT_STRING_EMPTY : DT_STRING,
                 defaultValue == NULL ? UNDEFINED_STRING.constData() : defaultValue);
}

/** @brief Analyses a long name option.
 *
 * The syntax of an long name option is --name or --name=value
 *
 * @param opt		the option string without --
 *
 */
void ReProgramArgs::analyseLong(const char* opt) {
   QByteArray name;
   const char* value = strchr(opt, '=');
   if (value == NULL)
      name = opt;
   else {
      name = QByteArray(opt).mid(0, value - opt);
      value++;
   }
   ReProgOption* option = search(0, name);
   if (option == NULL)
      throw ReOptionException(this, QObject::tr("unknown option: %1")
                              .arg(name.constData()));
   switch (option->m_type) {
   case DT_INT:
      if (value == NULL)
         throw ReOptionException(this,
                                 QObject::tr("Option %1: parameter expected. Use --%1=number").arg(name.constData()));
      else
         option->m_value = value;
      break;
   case DT_STRING:
      if (value == NULL)
         throw ReOptionException(this,
                                 QObject::tr("Option %1: parameter expected. Use --%1=string").arg(name.constData()));
      else if (value[0] == '\0')
         throw ReOptionException(this,
                                 QObject::tr("Option %1: empty string is not allowed. Use --%1=string").arg(name.constData()));
      option->m_value = value;
      break;
   case DT_STRING_EMPTY:
      if (value == NULL)
         value = "";
      option->m_value = value;
      break;
   case DT_BOOL:
      if (value == NULL) {
         // Invert the default value:
         option->m_value = option->m_defaultValue == "t" ? "f" : "t";
      } else if (ReStringUtils::isInList(value, ";y;yes;t;true", true,
                                         ReStringUtils::AUTO_SEPARATOR))
         option->m_value = "t";
      else if (ReStringUtils::isInList(value, ";n;no;f;false",
                                       true, ReStringUtils::AUTO_SEPARATOR))
         option->m_value = "f";
      else
         throw ReOptionException(this,
                                 QObject::tr("Option %1: Not a boolean value: %2. Use true or false")
                                 .arg(name.constData()).arg(value));
      break;
   default:
      break;
   }
}

/** @brief Analyses one or more short name options.
 *
 * Multiple short name options can be written in one word:
 * <p>Example: -x -y -z can be noted as -xyz</p>
 * <p>On the other side an option with parameter can be written in two forms:</p>
 * <ul>
 * <li>-xABC</li>
 * <li>-x ABC</li>
 * </ul>
 *
 * @param opt		an option string
 * @param nextArg	the next argument behind the current option string.
 * 					May be NULL (no more arguments)
 *
 * @return 	true a second word has been used: It was a parameter of an string or integer option.<br>
 * 			false: The next argument has not been used
 */
bool ReProgramArgs::analyseShort(const char* opt, const char* nextArg) {
   bool rc = false;
   QByteArrayList properties;
   bool again;
   do {
      again = false;
      ReProgOption* option = search(opt[0], NULL);
      // Forget the option short name:
      opt++;
      switch (option->m_type) {
      case DT_INT:
      case DT_STRING:
      case DT_STRING_EMPTY:
         if (opt[0] != '\0') {
            setValue(option->m_name, opt, option->m_type);
         } else {
            if (nextArg == NULL || nextArg[0] == '-') {
               if (option->m_type == DT_STRING_EMPTY)
                  setValue(option->m_name, "", option->m_type);
               else
                  throw ReOptionException(this,
                                          QObject::tr(
                                             "Option %1 has type %2! There is no parameter.")
                                          .arg(option->m_name.constData()).arg(typeToString(option->m_type)));
            } else {
               setValue(option->m_name.constData(), nextArg, option->m_type);
               rc = true;
            }
         }
         break;
      case DT_BOOL: {
         // Get the current value:
         const char* value = "t";
         if (opt[0] == '-') {
            opt++;
            value = "f";
         } else if (opt[0] == '+')
            opt++;
         // Invert the default value:
         if (option->m_defaultValue == "t")
            value = value[0] == 't' ? "f" : "t";
         setValue(option->m_name, value, option->m_type);
         again = opt[0] != '\0';
         break;
      }
      default:
         break;
      }
   } while (again);
   return rc;
}

/** @brief Returns a not option argument given by an index.
 *
 * @param index		the index of the wanted program argument which is not an option.
 *
 * @return			NULL: wrong index<br>
 *					otherwise: the wanted argument.
 */
const char* ReProgramArgs::arg(size_t index) const {
   const char* rc = NULL;
   if (index < (size_t) m_argCount)
      rc = m_args[index];
   return rc;
}

/**
 * Returns the argument vector (without options).
 *
 * @return the argument vector
 */
const char** ReProgramArgs::args() const {
   return m_args;
}

/** @brief Returns the count of arguments (without options).
 *
 * @return the count of arguments
 */
int ReProgramArgs::argCount() const {
   return m_argCount;
}

/** @brief Returns the value of a boolean option.
 *
 * @param name	Name of the option.
 *
 * @return The value of the option set in the programs arguments or the default value.
 *
 * @throws ReOptionException 	Unknown name or wrong type.
 */
bool ReProgramArgs::getBool(const char* name) {
   ReProgOption* option = m_options.value(name, NULL);
   if (option == NULL)
      throw ReOptionException(this, QObject::tr("%1 is not an option name").arg(name));
   if (option->m_type != DT_BOOL)
      throw ReOptionException(this,
                              QObject::tr("%1 is not an boolean option. Type is %2").arg(name)
                              .arg(typeToString(option->m_type)));
   bool rc = option->m_value == "t";
   return rc;
}

/** @brief Returns the value of an integer option.
 *
 * @param name	Name of the option.
 *
 * @return The value of the option set in the programs arguments or the default value.
 *
 * @throws ReOptionException 	Unknown name or wrong type.
 */
int ReProgramArgs::getInt(const char* name) {
   ReProgOption* option = m_options.value(name, NULL);
   if (option == NULL)
      throw ReOptionException(this, QObject::tr("%1 is not an option name").arg(name));
   if (option->m_type != DT_INT)
      throw ReOptionException(this,
                              QObject::tr("%1 is not an integer option. Type is %2").arg(name)
                              .arg(typeToString(option->m_type)));
   int rc = atoi(option->m_value.constData());
   return rc;
}

/** @brief Returns the value of a string option.
 *
 * @param name		the name of the option
 * @param buffer	OUT: the buffer for the name
 *
 * @return		NULL: default value is NULL and no argument has been given<br>
 *				the value of the option set in the programs arguments or the default value.
 *
 * @throws ReOptionException 	Unknown name or wrong type.
 */
const char* ReProgramArgs::getString(const char* name, QByteArray& buffer) {
   ReProgOption* option = m_options.value(name, NULL);
   if (option == NULL)
      throw ReOptionException(this, QObject::tr("%1 is not an option name").arg(name));
   if (option->m_type != DT_STRING && option->m_type != DT_STRING_EMPTY)
      throw ReOptionException(this,
                              QObject::tr("%1 is not an string option. Type is %2").arg(name)
                              .arg(typeToString(option->m_type)));
   buffer = option->m_value;
   return buffer == UNDEFINED_STRING ? NULL : buffer.constData();
}

/**
 * Issues a help message.
 *
 * @param message			message to show
 * @param issueLastError	<code>true</code>: the last OS error will be shown
 * @param lines				OUT: a stringlist for the help message
 */
void ReProgramArgs::help(const char* message, bool issueLastError,
                         QByteArrayList& lines) const {
   lines = m_usage;
   lines.append("");
   if (m_options.size() > 0) {
      lines.append(QObject::tr("<options>:").toLatin1());
   }
   QByteArray line;
   QByteArray param;
   ReProgOptionMap::const_iterator it;
   for (it = m_options.cbegin(); it != m_options.cend(); ++it) {
      ReProgOption* opt = it.value();
      param.resize(0);
      switch (opt->m_type) {
      case DT_INT:
         param = I18N::s2b(QObject::tr("<number>"));
         break;
      case DT_STRING:
         param = I18N::s2b(QObject::tr("<not empty string>"));
         break;
      case DT_STRING_EMPTY:
         param = I18N::s2b(QObject::tr("[<string>]"));
         break;
      default:
         break;
      }
      line.resize(0);
      if (opt->m_shortName != UNDEF_SHORT_NAME) {
         line.append("-").append(opt->m_shortName);
         line.append(param).append(' ').append(I18N::s2b(QObject::tr(" or ")));
      }
      line.append("--").append(opt->m_longName);
      if (param.length() > 0) {
         line.append("=", -1).append(param.constData(), -1);
         if (opt->m_type != DT_STRING
               || ! opt->m_defaultValue.isEmpty()) {
            line.append(I18N::s2b(QObject::tr(" Default value: ")));
            if (opt->m_type == DT_STRING)
               line.append('\'');
            line.append(opt->m_defaultValue);
            if (opt->m_type == DT_STRING)
               line.append('\'');
         }
      }
      lines.append(line.constData());
      line.resize(0);
      QByteArrayList desc = opt->m_description.split('\n');
      QByteArrayList::const_iterator it;
      for (it = desc.cbegin(); it != desc.cend(); ++it) {
         line.resize(0);
         line.append(PREFIX_LINE_OPTION).append(*it);
         lines.append(line);
      }
   }
   if (m_examples.count() > 0) {
      lines.append(I18N::s2b(QObject::tr("Example(s):")));
      lines.append(m_examples);
   }
   if (issueLastError && m_lastError.length() > 0) {
      line.resize(0);
      line.append("+++ ").append(m_lastError.constData());
      lines.append(line.constData());
   }
   if (message != NULL && message[0] != '\0') {
      line.resize(0);
      line.append("+++ ").append(message);
      lines.append(line.constData());
   }
}
/**
 * Issues a help message.
 *
 * @param message			message to show
 * @param issueLastError	<code>true</code>: the last OS error will be shown
 * @param stream			OUT: output stream, e.g. stderr
 */
void ReProgramArgs::help(const char* message, bool issueLastError,
                         FILE* stream) const {
   QByteArrayList lines;
   help(message, issueLastError, lines);
   for (int ii = 0; ii < lines.count(); ii++) {
      fputs(lines.at(ii).constData(), stream);
      fputc('\n', stream);
   }
}

/** Initializes the options from the program arguments.
 *
 * While arguments are preceded by an '-' they will be treated as options.
 * The rest of arguments are stored for retrieving with <code>getArg()</code>.
 *
 * @param argc			the count of program arguments (inclusive options)
 * @param argv			the argument vector
 * @param hasProgram	<code>true</code>: the first argument is the program name
 *
 * @throws ReException
 */
void ReProgramArgs::init(int argc, const char* argv[], bool hasProgram) {
   if (hasProgram) {
      m_program = argc <= 0 ? "" : ReFileUtils::nodeOf(argv[0]);
      argv++;
      argc--;
   }
   while (argc > 0 && argv[0][0] == '-') {
      if (argv[0][1] == '-')
         analyseLong(argv[0] + 2);
      else {
         if (analyseShort(argv[0] + 1, argc <= 1 ? NULL : argv[1]))
            argc--, argv++;
      }
      argc--;
      argv++;
   }
   m_argCount = argc;
   m_args = argv;
   for (int ii = 0; ii < m_usage.length(); ii++) {
      m_usage[ii].replace("$0", m_program);
   }
   QByteArray line;
   for (int ii = 0; ii < m_examples.length(); ii++) {
      line = m_examples[ii];
      m_examples[ii] = line.replace("$0", m_program);
   }
}

/** @brief Returns the program name.
 *
 * @return The name of the application.
 */
const char* ReProgramArgs::programName() const {
   return m_program;
}

/** @brief Search the property string of an option.
 *
 * @param shortName		the option`s short name. Not relevant if <code>longName != NULL</code>
 * @param longName		the option`s long name. Not relevant if <code>longName == NULL</code>
 * @return				the option
 *
 * @throws ReOptionException	Unknown option.
 */
ReProgOption* ReProgramArgs::search(char shortName, const char* longName) {
   ReProgOptionMap::const_iterator it;
   ReProgOption* rc = NULL;
   ReProgOption* opt;
   for (it = m_options.cbegin(); rc == NULL && it != m_options.cend(); ++it) {
      opt = it.value();
      if ( (shortName != 0 && shortName == opt->m_shortName)
            || longName == opt->m_longName)
         rc = opt;
   }
   if (rc == NULL) {
      QByteArray name;
      if (longName == NULL)
         name.append(shortName);
      else
         name = longName;
      throw ReOptionException(this, QObject::tr("Unknown option: %1").arg(name.constData()));
   }
   return rc;
}

/** @brief Sets the last error message.
 *
 * @param message	The error message.
 */
void ReProgramArgs::setLastError(const char* message) {
   m_lastError = message;
}

/** @brief Sets the last error message.
 *
 * @param message	The error message.
 */
void ReProgramArgs::setLastError(const QByteArray& message) {
   m_lastError = message;
}

/** @brief Sets the option value.
 *
 * @param name		The option's name.
 * @param value		The option's value.
 * @param dataType	Theo option's data type.
 */
void ReProgramArgs::setValue(const char* name, const char* value,
                             DataType dataType) {
   switch (dataType) {
   case DT_INT:
      if (strspn(value, "01234567890") != strlen(value))
         throw ReOptionException(this,
                                 QObject::tr("Option %1 expect an integer as parameter, not %2").arg(name)
                                 .arg(value));
      break;
   case DT_STRING:
      if (value[0] == '\0')
         throw ReOptionException(this,
                                 QObject::tr("Option %1: Empty parameter is not allowed").arg(name));
      break;
   case DT_STRING_EMPTY:
   case DT_BOOL:
   default:
      break;
   }
   ReProgOption* opt = m_options.value(name);
   opt->m_value = value;
}

