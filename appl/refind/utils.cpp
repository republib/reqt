/*
 * utils.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "utils.hpp"

/**
 * Constructor.
 */
ContextHandler::ContextHandler(IntrinsicType intrinsicType, const QString& text) :
   m_text(text),
   m_program(),
   m_arguments(),
   m_directoryMode(DM_UNDEF),
   m_fileType(FT_ALL),
   m_intrinsicType(intrinsicType) {
}

/**
 * Constructor.
 */
ContextHandler::ContextHandler() :
   m_text(),
   m_program(),
   m_arguments(),
   m_directoryMode(DM_TO_PARENT),
   m_fileType(FT_FILE),
   m_intrinsicType(IT_UNDEF) {
}

/**
 * Returns the intrinsic type.
 *
 * @return  the intrinsic type
 */
ContextHandler::IntrinsicType ContextHandler::intrinsicType() const {
   return m_intrinsicType;
}

/**
 * Constructor.
 */
ContextHandlerList::ContextHandlerList() :
   m_list() {
}

/**
 * Constructor.
 */
ContextHandlerList::~ContextHandlerList() {
   clear();
}
/**
 * Copy constructor.
 *
 * @param source    source to copy
 * @return          the instance itself
 */
ContextHandlerList::ContextHandlerList(const ContextHandlerList& source) :
   m_list() {
   copy(source);
}

/**
 * Assignment operator.
 *
 * @param source    source to copy
 * @return          the instance itself
 */
ContextHandlerList& ContextHandlerList::operator =(
   const ContextHandlerList& source) {
   return copy(source);
}

/**
 * Adds the intrinsic context menu actions.
 */
void ContextHandlerList::addIntrinsics() {
   m_list.append(
      new ContextHandler(ContextHandler::IT_COPY, QObject::tr("copy")));
}

/**
 * Frees the resources.
 */
void ContextHandlerList::clear() {
   QList <ContextHandler*>::const_iterator it;
   for (it = m_list.begin(); it != m_list.end(); ++it) {
      delete *it;
   }
   m_list.clear();
}

/**
 * Assignment operator.
 *
 * @param source    source to copy
 * @return          the instance itself
 */
ContextHandlerList& ContextHandlerList::copy(const ContextHandlerList& source) {
   clear();
   QList <ContextHandler*>::const_iterator it;
   for (it = source.m_list.begin(); it != source.m_list.end(); ++it) {
      ContextHandler* handler = *it;
      if (handler->intrinsicType() == ContextHandler::IT_UNDEF)
         m_list.append(handler);
   }
   addIntrinsics();
   return *this;
}

/**
 * Stores the list in the storage.
 *
 * @param storage   IN/OUT: the storage
 */
void ContextHandlerList::save(ReStateStorage& storage) {
   const char* name = "context.program";
   QString value;
   for (int ix = 0; ix < m_list.size(); ix++) {
      ContextHandler* handler = m_list.at(ix);
      if (handler->intrinsicType() == ContextHandler::IT_UNDEF) {
         value = handler->m_text + "\t" + handler->m_program + "\t"
                 + handler->m_arguments + "\t" + QString::number(handler->m_fileType)
                 + "\t" + QString::number(handler->m_directoryMode);
         storage.store(name, value, ix);
      }
   }
}

ContextHandler* createHandlerIfExists(const QString& file,
                                      ContextHandler::FileType fileType = ContextHandler::FT_FILE) {
   QFileInfo info(file);
   ContextHandler* handler = NULL;
   if (info.exists()) {
      handler = new ContextHandler;
      handler->m_text = ReFileUtils::nodeOf(file);
      handler->m_program = file;
      handler->m_arguments = "${full}";
#if defined WIN32
      if (handler->m_text == "cmd.exe")
         handler->m_arguments = "";
#endif
      handler->m_fileType = fileType;
      handler->m_directoryMode =
         fileType == ContextHandler::FT_FILE ?
         ContextHandler::DM_TO_PARENT : ContextHandler::DM_TO_FILE;
   }
   return handler;
}

/**
 * Reads the list from the storage.
 *
 * @param storage
 */
void ContextHandlerList::restore(ReStateStorage& storage) {
   const char* name = "context.program";
   clear();
   int ix = -1;
   QString value;
   QStringList cols;
   ContextHandler* handler;
   while (true) {
      ix++;
      value = storage.restore(name, ix);
      if (value.isEmpty())
         break;
      cols = value.split('\t');
      if (cols.size() < 3)
         continue;
      handler = new ContextHandler;
      handler->m_text = cols.at(0);
      handler->m_program = cols.at(1);
      handler->m_arguments = cols.at(2);
      long int lValue = atol(cols.at(3).toLatin1().constData());
      handler->m_fileType = ContextHandler::FileType(lValue);
      lValue = atol(cols.at(4).toLatin1().constData());
      handler->m_directoryMode = ContextHandler::DirMode(lValue);
      m_list.append(handler);
   }
   if (m_list.size() == 0) {
#if defined __linux__
      handler = createHandlerIfExists("/usr/bin/kate");
      if (handler == NULL)
         handler = createHandlerIfExists("/usr/bin/geany");
      if (handler == NULL)
         handler = createHandlerIfExists("/usr/bin/kwrite");
      if (handler == NULL)
         handler = createHandlerIfExists("/usr/bin/gedit");
      if (handler != NULL)
         m_list.append(handler);
      handler = createHandlerIfExists("/usr/bin/konsole", ContextHandler::FT_DIR);
      if (handler == NULL)
         handler = createHandlerIfExists("/usr/bin/gnome-terminal", ContextHandler::FT_DIR);
      if (handler != NULL)
         m_list.append(handler);
#elif defined WIN32
      handler = createHandlerIfExists("c:\\windows\\system32\\notepad.exe");
      if (handler != NULL)
         m_list.append(handler);
      handler = createHandlerIfExists("c:\\windows\\system32\\cmd.exe", ContextHandler::FT_DIR);
      if (handler != NULL)
         m_list.append(handler);
#endif
   }
   addIntrinsics();
}

/**
 * Constructor.
 */
Statistics::Statistics() {
   clear();
}

/**
 * Resets all members.
 */
void Statistics::clear() {
   m_dirs = m_files = 0;
   m_bytes = 0;
   m_runtimeSeconds = 0;
}

/**
 * Adds the result of another instance to the instance.
 *
 * @param source	instance to add
 */
void Statistics::add(Statistics& source) {
   m_dirs += source.m_dirs;
   m_bytes += source.m_bytes;
   m_files += source.m_bytes;
   m_runtimeSeconds += source.m_runtimeSeconds;
}
