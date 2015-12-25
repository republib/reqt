/*
 * remodules.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RPLMODULES_HPP
#define RPLMODULES_HPP

enum {
   LOC_LOGGER = 101,
   LOC_CONFIG,
   LOC_CONTAINER,
   LOC_EXCEPTION,
   LOC_TEST, // 105
   LOC_TCPSERVER,
   LOC_TCPCLIENT,
   LOC_TCPPEER,
   LOC_TERMINATOR,
   LOC_ASTREE, // 110
   LOC_ASCLASSES,
   LOC_LEXER,
   LOC_SOURCE,
   LOC_VM,
   LOC_MFPARSER, // 115
   LOC_TRAVERSER,
   LOC_SETTINGS,
   LOC_FILE,
   LOC_FILETREE,
   LOC_STATESTORAGE,	// 120
   LOC_FILESYSTEM,
   LOC_RANDOMIZER,
   LOC_CRYPTFILESYSTEM,
};
#define LOC_FIRST_OF(moduleNo) (moduleNo*100+1)
class RplModules {
public:
   static int fileToNumber(const char* file);
   static const char* numberToFile(int location);
};

#endif // RPLMODULES_HPP
