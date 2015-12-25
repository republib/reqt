/*
 * cuReEdit.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "base/rebase.hpp"
#include "gui/regui.hpp"
#include "QKeyEvent"
/** @file
 * @brief Unit test of the basic exceptions.
 */

class TestReEdit: public ReTest, protected ReEdit {
public:
   TestReEdit() :
      ReTest("ReEdit"),
      ReEdit(NULL) {
      doIt();
   }

public:
   ReLines m_lines;
   void init(const QString& lines) {
      m_lines.clear();
      m_lines.insertLines(0, lines, false);
      setLines(&m_lines);
   }
   void pushKey(int key, const QString& text = "",
                Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
      QKeyEvent event(QEvent::KeyPress, key, modifiers, text);
      keyPressEvent(&event);
   }

   void checkFirstPos(int col, int expectedFirstLine, int expectedFirstCol) {
      m_cursorCol = col;
      ensureCursorVisible();
      checkEqu(expectedFirstLine, m_firstLine);
      checkEqu(expectedFirstCol, m_firstCol);
   }

   void testEnsureCursorVisible() {
      init("abc\n123456789_12\nxyz");
      m_screenWidth = 5;
      m_firstLine = 0;
      m_firstCol = 0;
      checkFirstPos(0, 0, 0);
      m_cursorLineNo = 1;
      int col;
      for (col = 0; col < m_screenWidth; col++) {
         checkFirstPos(col, 1, 0);
      }
      int lineLength = 12;
      int colMax = lineLength - m_screenWidth;
      checkFirstPos(m_screenWidth, 1, m_screenWidth);
      checkFirstPos(m_screenWidth + 1, 1, m_screenWidth + 1);
      for (col = colMax; col < lineLength + 2; col++) {
         checkFirstPos(col, 1, colMax);
      }
   }
   void checkCursorPos(int key, int expectedLineNo, int expectedCol) {
      pushKey(key);
      checkEqu(expectedLineNo, m_cursorLineNo);
      checkEqu(expectedCol, m_cursorCol);
   }
   void checkCursorPos(EditorAction action, int expectedLineNo,
                       int expectedCol) {
      editorAction(action);
      checkEqu(expectedLineNo, m_cursorLineNo);
      checkEqu(expectedCol, m_cursorCol);
   }

   void testCursorMove() {
      init("abc\n123456789_12\nxy");
      m_screenWidth = 5;
      m_firstLine = 0;
      m_firstCol = 0;
      m_cursorCol = -1;
      m_cursorLineNo = 0;
      // Left, right, pos1 and end:
      checkCursorPos(Qt::Key_Right, 0, 0);
      checkCursorPos(Qt::Key_Right, 0, 1);
      checkCursorPos(Qt::Key_Right, 0, 2);
      // next line:
      checkCursorPos(Qt::Key_Right, 1, -1);
      checkCursorPos(Qt::Key_Right, 1, 0);
      checkCursorPos(Qt::Key_Right, 1, 1);
      checkCursorPos(Qt::Key_Home, 1, -1);
      checkCursorPos(Qt::Key_End, 1, 11);
      // next line:
      checkCursorPos(Qt::Key_Right, 2, -1);
      checkCursorPos(Qt::Key_Right, 2, 0);
      checkCursorPos(Qt::Key_Right, 2, 1);
      // EoF reached: position remains
      checkCursorPos(Qt::Key_Right, 2, 1);
      checkCursorPos(Qt::Key_Left, 2, 0);
      checkCursorPos(Qt::Key_Left, 2, -1);
      // prev. line:
      checkCursorPos(Qt::Key_Left, 1, 11);
      checkCursorPos(Qt::Key_Left, 1, 10);
      checkCursorPos(Qt::Key_Home, 1, -1);
      // prev. line:
      checkCursorPos(Qt::Key_Left, 0, 2);
      checkCursorPos(Qt::Key_Left, 0, 1);
      checkCursorPos(Qt::Key_Left, 0, 0);
      checkCursorPos(Qt::Key_Left, 0, -1);
      // BoF: position remains:
      checkCursorPos(Qt::Key_Left, 0, -1);
      // Right when column to left:
      checkCursorPos(Qt::Key_Down, 1, -1);
      checkCursorPos(Qt::Key_End, 1, 11);
      // the column remains at 11!
      checkCursorPos(Qt::Key_Up, 0, 11);
      // the last index is 2
      checkCursorPos(Qt::Key_Left, 0, 1);
      checkCursorPos(Qt::Key_Left, 0, 0);
      checkCursorPos(Qt::Key_Left, 0, -1);
      // Up and down, first column:
      // first line: position remains:
      checkCursorPos(Qt::Key_Up, 0, -1);
      checkCursorPos(Qt::Key_Down, 1, -1);
      checkCursorPos(Qt::Key_Down, 2, -1);
      // last line: position remains:
      checkCursorPos(Qt::Key_Down, 2, -1);
      checkCursorPos(Qt::Key_Up, 1, -1);
      checkCursorPos(Qt::Key_Up, 0, -1);
      checkCursorPos(Qt::Key_Down, 1, -1);
      // Up and down, last column:
      checkCursorPos(Qt::Key_End, 1, 11);
      checkCursorPos(Qt::Key_Up, 0, 11);
      checkCursorPos(Qt::Key_Up, 0, 11);
      checkCursorPos(Qt::Key_Down, 1, 11);
      checkCursorPos(Qt::Key_Down, 2, 11);
      checkCursorPos(Qt::Key_Down, 2, 11);
      // BoF and EoF:
      checkCursorPos(EA_BEGIN_OF_FILE, 0, -1);
      checkCursorPos(EA_BEGIN_OF_FILE, 0, -1);
      checkCursorPos(EA_END_OF_FILE, 2, 1);
      checkCursorPos(EA_END_OF_FILE, 2, 1);
      checkCursorPos(EA_BEGIN_OF_FILE, 0, -1);
   }

   void testEnterText() {
      init("abc\n1234\nxy");
      m_screenWidth = 5;
      m_firstLine = 0;
      m_firstCol = 0;
      // Split line at the end:
      m_cursorLineNo = 0;
      m_cursorCol = 2;
      pushKey(Qt::Key_Enter, "\n");
      checkEqu("abc", m_lines.lineAt(0));
      checkEqu("", m_lines.lineAt(1));
      checkEqu("1234", m_lines.lineAt(2));
      // Split line in the middle of the line:
      m_cursorLineNo = 0;
      m_cursorCol = 1;
      init("abc\n1234\nxy");
      pushKey(Qt::Key_Enter, "\n");
      checkEqu("ab", m_lines.lineAt(0));
      checkEqu("c", m_lines.lineAt(1));
      checkEqu("1234", m_lines.lineAt(2));
      // ... in the last line:
      // Split line at the end:
      m_cursorLineNo = 2;
      m_cursorCol = 2;
      init("abc\n1234\nxy");
      pushKey(Qt::Key_Return, "\n");
      checkEqu("xy", m_lines.lineAt(2));
      checkEqu("", m_lines.lineAt(3));
      // Split line in the middle of the line:
      m_cursorLineNo = 2;
      m_cursorCol = 1;
      init("abc\n1234\nxyz");
      pushKey(Qt::Key_Return, "\n");
      checkEqu("xy", m_lines.lineAt(2));
      checkEqu("z", m_lines.lineAt(3));
   }

   void testDeleteText() {
      init("abcd\n1234\nxy");
      m_screenWidth = 5;
      m_firstLine = 0;
      m_firstCol = 0;
      // first line:
      m_cursorLineNo = 0;
      // in the middle of the line:
      m_cursorCol = 1;
      editorAction(EA_DEL_BEGIN_OF_LINE);
      checkEqu("cd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      checkEqu(-1, m_cursorCol);
      checkEqu(0, m_cursorLineNo);
      // at the begin of the line:
      init("abcd\n1234\nxy");
      m_cursorCol = -1;
      editorAction(EA_DEL_BEGIN_OF_LINE);
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      // in the middle of the line:
      init("abcd\n1234\nxy");
      m_cursorCol = 1;
      editorAction(EA_DEL_END_OF_LINE);
      checkEqu("ab", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      checkEqu(1, m_cursorCol);
      checkEqu(0, m_cursorLineNo);
      // at the end of the line:
      init("abcd\n1234\nxy");
      m_cursorCol = 3;
      editorAction(EA_DEL_END_OF_LINE);
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      // last line:
      init("abcd\n1234\nxy");
      m_cursorLineNo = 2;
      // in the middle of the line:
      m_cursorCol = 0;
      editorAction(EA_DEL_BEGIN_OF_LINE);
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      checkEqu("y", m_lines.lineAt(2));
      checkEqu(-1, m_cursorCol);
      checkEqu(2, m_cursorLineNo);
      // at the begin of the line:
      init("abcd\n1234\nxy");
      m_cursorCol = -1;
      editorAction(EA_DEL_BEGIN_OF_LINE);
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      // in the middle of the line:
      init("abcd\n1234\nxy");
      m_cursorCol = 0;
      editorAction(EA_DEL_END_OF_LINE);
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      checkEqu("x", m_lines.lineAt(2));
      checkEqu(0, m_cursorCol);
      checkEqu(2, m_cursorLineNo);
      // at the end of the line:
      init("abcd\n1234\nxy");
      m_cursorCol = 1;
      editorAction(EA_DEL_END_OF_LINE);
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      checkEqu("xy", m_lines.lineAt(2));
   }
   void testDeleteLine() {
      init("abcd\n1234\nxy");
      m_screenWidth = 5;
      m_firstLine = 0;
      m_firstCol = 0;
      // first line:
      m_cursorLineNo = 0;
      m_cursorCol = 1;
      editorAction(EA_DEL_LINE);
      checkEqu(2, m_lines.lineCount());
      checkEqu("1234", m_lines.lineAt(0));
      checkEqu("xy", m_lines.lineAt(1));
      checkEqu(-1, m_cursorCol);
      checkEqu(0, m_cursorLineNo);
      // middle line:
      init("abcd\n1234\nxy");
      m_cursorLineNo = 1;
      m_cursorCol = 1;
      editorAction(EA_DEL_LINE);
      checkEqu(2, m_lines.lineCount());
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("xy", m_lines.lineAt(1));
      checkEqu(-1, m_cursorCol);
      checkEqu(1, m_cursorLineNo);
      // last line:
      init("abcd\n1234\nxy");
      m_cursorLineNo = 2;
      m_cursorCol = 1;
      editorAction(EA_DEL_LINE);
      checkEqu(2, m_lines.lineCount());
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      checkEqu(-1, m_cursorCol);
      checkEqu(1, m_cursorLineNo);
      // wrong line numbers:
      init("abcd\n1234\nxy");
      m_cursorCol = 1;
      m_cursorLineNo = -1;
      editorAction(EA_DEL_LINE);
      checkEqu(3, m_lines.lineCount());
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      checkEqu("xy", m_lines.lineAt(2));
      m_cursorLineNo = 3;
      editorAction(EA_DEL_LINE);
      checkEqu(3, m_lines.lineCount());
      checkEqu("abcd", m_lines.lineAt(0));
      checkEqu("1234", m_lines.lineAt(1));
      checkEqu("xy", m_lines.lineAt(2));
      log("ok");
   }
   void testIndexToColumn() {
      /*
       * <pre>tabulator width: 4
       * _01230123
       * _	x	index: 0 blanks: 3
       * _x	x	index: 1 blanks: 2
       * _xx	x	index: 2 blanks: 1
       * _xxx	x	index: 3 blanks: 0
       * </pre>
       */
      int tabWidth = 4;
      checkEqu(0, ReParagraphs::indexToColumn(0, tabWidth, "\tx"));
      checkEqu(4, ReParagraphs::indexToColumn(1, tabWidth, "\tx"));
      checkEqu(5, ReParagraphs::indexToColumn(2, tabWidth, "\tx"));
      checkEqu(5, ReParagraphs::indexToColumn(3, tabWidth, "\tx"));
      checkEqu(1, ReParagraphs::indexToColumn(1, tabWidth, "1\tx"));
      checkEqu(4, ReParagraphs::indexToColumn(2, tabWidth, "1\tx"));
      checkEqu(5, ReParagraphs::indexToColumn(3, tabWidth, "1\tx"));
      checkEqu(1, ReParagraphs::indexToColumn(1, tabWidth, "12\tx"));
      checkEqu(2, ReParagraphs::indexToColumn(2, tabWidth, "12\tx"));
      checkEqu(4, ReParagraphs::indexToColumn(3, tabWidth, "12\tx"));
      checkEqu(5, ReParagraphs::indexToColumn(5, tabWidth, "12\tx"));
      checkEqu(1, ReParagraphs::indexToColumn(1, tabWidth, "123\tx"));
      checkEqu(2, ReParagraphs::indexToColumn(2, tabWidth, "123\tx"));
      checkEqu(3, ReParagraphs::indexToColumn(3, tabWidth, "123\tx"));
      checkEqu(4, ReParagraphs::indexToColumn(4, tabWidth, "123\tx"));
   }

   virtual void runTests() {
      testIndexToColumn();
      testDeleteLine();
      testDeleteText();
      testEnterText();
      testCursorMove();
      testEnsureCursorVisible();
   }
};
void testReEdit() {
   TestReEdit test;
   ReUseParameter(&test);
}

