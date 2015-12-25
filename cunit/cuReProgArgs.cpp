/*
 * cuReProgArgs.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the QByteArray tools.
 */
#include "base/rebase.hpp"
/**
 * @brief Unit test for <code>ReProgArguments</code>.
 */
class TestReProgArgs: public ReTest {
public:
   TestReProgArgs() :
      ReTest("ReProgArgs") {
      doIt();
   }

public:
   void testBase() {
      ReProgramArgs args("usage: example prog", "example -a");
      args.addBool("boolArg", "a bool arg", 'b', "bool-arg", false);
      args.addInt("intArg", "integer arg", 'i', "int-arg", 99);
      args.addString("stringArg", "string argument", 's', "string-arg", true, "");
      const char* arguments[] = { "example" };
      args.init(1, arguments, false);
      checkF(args.getBool("boolArg"));
      checkEqu(99, args.getInt("intArg"));
      QByteArray buffer = "123";
      checkEqu("", args.getString("stringArg", buffer));
      checkEqu("", buffer);
   }

   void testBool() {
      ReProgramArgs args("usage: example prog", "example -a");
      args.addBool("trueArg", "a bool arg", 't', "true-arg", false);
      args.addBool("falseArg", "a bool arg", 'f', "false-arg", true);
      args.addBool("trueArg2", "a bool arg", 0, "true-arg2", false);
      args.addBool("falseArg2", "a bool arg", 0, "false-arg2", true);
      args.addBool("trueArg3", "a bool arg", 0, "true-arg3", false);
      args.addBool("falseArg3", "a bool arg", 0, "false-arg3", true);
      args.addBool("trueArg4", "a bool arg", 0, "true-arg4", false);
      args.addBool("falseArg4", "a bool arg", 0, "false-arg4", true);
      const char* arguments[] = {
         "example",
         "--true-arg4=n", "--false-arg3=y",
         "--true-arg3=y", "--false-arg3=n",
         "--true-arg2", "--false-arg2",
         "-t", "-f",
         NULL
      };
      args.init(9, arguments, true);
      checkT(args.getBool("trueArg"));
      checkF(args.getBool("falseArg"));
      checkT(args.getBool("trueArg2"));
      checkF(args.getBool("falseArg2"));
      checkT(args.getBool("trueArg3"));
      checkF(args.getBool("falseArg3"));
      checkF(args.getBool("trueArg4"));
      checkT(args.getBool("falseArg4"));
   }
   void testInt() {
      ReProgramArgs args("usage: example prog", "example -a");
      args.addInt("intArg", "an int arg", 'i', "arg", 4711);
      args.addInt("intArg2", "an int arg", 0, "arg2", 4712);
      const char* arguments[] = {
         "example",
         "-i", "2244",
         "--arg2=3355",
         NULL
      };
      args.init(4, arguments, true);
      checkEqu(2244, args.getInt("intArg"));
      checkEqu(3355, args.getInt("intArg2"));
      ReProgramArgs args2("usage: example prog", "example -a");
      args2.addInt("intArg", "an int arg", 'i', "arg2", 4711);
      const char* arguments2[] = {
         "example",
         "--arg2",
         NULL
      };
      try {
         args.init(2, arguments2, true);
         error("exception expected: missing parameter");
      } catch(ReOptionException e) {
         checkT(e.getMessage().indexOf("arg2") > 0);
      }
   }
   void testString() {
      ReProgramArgs args("usage: example prog", "example -a");
      args.addString("stringArg", "a string arg", 's', "arg", false, "abc");
      args.addString("stringArg2", "a string arg", 0, "arg2", true, "bcd");
      args.addString("stringArg3", "a string arg", 0, "arg3", true, "def");
      const char* arguments[] = {
         "example",
         "-s", "wow",
         "--arg2=1 2 3",
         "--arg3=",
         NULL
      };
      args.init(5, arguments, true);
      QByteArray buffer;
      checkEqu("wow", args.getString("stringArg", buffer));
      checkEqu("1 2 3", args.getString("stringArg2", buffer));
      checkEqu("", args.getString("stringArg3", buffer));
      ReProgramArgs args2("usage: example prog", "example -a");
      args2.addString("stringArg", "a string arg", 'i', "arg2", false, "king");
      const char* arguments2[] = {
         "example",
         "--arg2",
         NULL
      };
      try {
         args2.init(2, arguments2, true);
         error("exception expected: missing parameter");
      } catch(ReOptionException e) {
         checkT(e.getMessage().indexOf("arg2") > 0);
      }
      ReProgramArgs args3("usage: example prog", "example -a");
      args3.addString("stringArg", "a string arg", 0, "arg", true, NULL);
      args3.addString("stringArg2", "a string arg", 'i', "arg2", false, "king");
      const char* arguments3[] = {
         "example",
         "--arg=",
         "--arg2=",
         NULL
      };
      try {
         args3.init(3, arguments3, true);
         error("exception expected: empty string is not allowed");
      } catch(ReOptionException e) {
         checkT(e.getMessage().indexOf("arg2") > 0);
      }
   }
   void testHelp() {
      ReProgramArgs args("usage: example prog", "example -a");
      args.addBool("boolArg", "a bool arg", 'b', "bool-arg", false);
      args.addInt("intArg", "integer arg", 'i', "int-arg", 99);
      args.addString("stringArg", "string argument", 's', "string-arg", true, "");
      const char* arguments[] = { "example" };
      QByteArrayList list;
      args.help("dummy error", false, list);
      QByteArray expected = "usage: example prog\n"
                            "\n"
                            "<options>:\n"
                            "-b  or --bool-arg\n"
                            "   a bool arg\n"
                            "-i<number>  or --int-arg=<number> Default value: 99\n"
                            "   integer arg\n"
                            "-s[<string>]  or --string-arg=[<string>] Default value: \n"
                            "   string argument\n"
                            "Example(s):\n"
                            "example -a\n"
                            "+++ dummy error\n";
      checkEqu(expected, list.join('\n'));
   }
   virtual void runTests() {
      try {
         testHelp();
         testString();
         testInt();
         testBool();
         testBase();
      } catch (ReOptionException e) {
         error(e.getMessage());
      }
      log("ready");
   }
};


void testReProgArgs() {
   TestReProgArgs test;
}

