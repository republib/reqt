/*
 * ReEdit.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REEDITOR_HPP
#define REEDITOR_HPP

#include <QWidget>
#include <QGraphicsView>
#include <QFont>
#include <QBrush>
#include <QPen>
#ifndef REBASE_HPP
#include "base/rebase.hpp"
#endif
class ReEdit;
/**
 * The presentation of a piece of text with font, color, background...
 */
class ReLook {
public:
   enum ForeGround {
      FG_STANDARD,
      FG_CURRENT_LINE,
      FG_SELECTED,
      FG_CURRENT_SELECTED,
      FG_RED_LIGHT,
      FG_RED_DARK,
      FG_BLUE_LIGHT,
      FG_BLUE_DARK,
      FG_GREEN_LIGHT,
      FG_GREEN_DARK,
      FG_GREY_LIGHT,
      FG_GREY_DARK,
      FG_YELLOW_LIGHT,
      FG_YELLOW_DARK,
      FG_MAGENTA_LIGHT,
      FG_MAGENTA_DARK,
      FG_CYAN_LIGHT,
      FG_CYAN_DARK,
      FG_COUNT
   };
   enum BackGround {
      BG_STANDARD,
      BG_CURRENT_LINE,
      BG_SELECTED,
      BG_CURRENT_SELECTED,
      BG_SCROLLBAR,
      BG_SLIDER,
      BG_SEARCHED,
      BG_SAME_WORD,
      BG_YELLOW,
      BG_GREY,
      BG_RED,
      BG_GREEN,
      BG_BLUE,
      BG_COUNT
   };
public:
   ReLook();
public:
   QFont* m_font;
   QFontMetrics* m_metrics;
   ForeGround m_foreground;
   BackGround m_background;
   ReEdit* m_edit;
   QBrush* m_brush;
   QPen* m_pen;
};

/**
 * A piece of text with the same presentation (for all of its characters).
 */
class ReEditText {
public:
   ReEditText(const QString& text, ReLook* ReLook);
   /** Copy constructor.
    * @param source    source to copy
    */
   inline ReEditText(const ReEditText& source) :
      m_text(source.m_text),
      m_look(source.m_look) {
   }
   /** Assignment operator.
    * @param source    source to copy
    * @return the instance
    */
   inline ReEditText& operator =(const ReEditText& source) {
      m_text = source.m_text;
      m_look = source.m_look;
      return *this;
   }

public:
   /** Returns the look of the instance.
    * @return  the look
    */
   ReLook* look() const {
      return m_look;
   }
   /** Sets the look of the instance
    * @param look  the new look
    */
   void setLook(ReLook* look) {
      m_look = look;
   }
   /** Returns the text.
    * @return the text
    */
   inline const QString& text() const {
      return m_text;
   }
private:
   QString m_text;
   ReLook* m_look;
};

/**
 * A logical line (with potential multiple physical lines).
 *
 * A paragraph can contain texts with different presentations.
 */
class ReParagraph: public QList<ReEditText*> {
public:
   virtual ~ReParagraph();
public:
   void draw(QPainter& painter, int& top, int left);
public:
   // number of columns of the paragraph (length with expanded tabs).
   int m_columns;
};

/**
 * A manipulator of a paragraph.
 *
 * A <code>ReParagraphBuilder</code> changes the presentation of the given text
 * inside the paragraph. It should <b>never</b> change the text!
 */
class ReParagraphBuilder {
public:
   virtual void buildParagraph(ReParagraph& paragraph, int lineNo,
                               ReEdit* edit);
};

class ReCursortLineBuilder: public ReParagraphBuilder {
   // ReParagraphBuilder interface
public:
   virtual void buildParagraph(ReParagraph& paragraph, int lineNo,
                               ReEdit* edit);
};

/**
 * A list of paragraphs.
 *
 * It contains a list of paragraph builders which compounds the presentation
 * of the piece of texts, e.g. a syntax highlighter make the color of the words
 * and a search engine paints up the last hits of the search.
 */
class ReParagraphs {
   friend class ReParagraphBuilder;
   friend class ReMouseCatcher;
public:
   ReParagraphs();
   virtual ~ReParagraphs();
public:
   /** Appends a paragraph builder to the list
    * @param builder   the paragraph builder to append
    */
   void appendBuilder(ReParagraphBuilder* builder) {
      m_builders.append(builder);
   }
   void clear();
   ReParagraph* cursorParagraph();
   void draw(QPainter& painter, int top, int left);
   int columnToIndex(int cursorCol);
   /** Returns the paragraph with a given index from the list.
    * @param ix    the index [0..N-1] of the paragraph in the list
    * @return      NULL: wrong index<br>
    *              the wanted paragraph
    */
   virtual ReParagraph* lineAt(int ix) {
      return ix < 0 || ix >= m_list.length() ? NULL : m_list.at(ix);
   }
   void load(int lineNo, int count, int width, ReEdit* edit);
   int indexToColumn(int index);

public:
   void setLines(ReLines* lines);
public:
   static int columnToIndex(int column, int tabWidth, const QString& string);
   static int indexToColumn(int index, int tabWidth, const QString& string);
protected:
   QList<ReParagraphBuilder*> m_builders;
   /// the m_list.at(0) belongs to m_lines.atLine(m_firstLine)
   int m_firstLine;
   /// the first visible column (horizontal scrolling)
   int m_firstCol;
   /// the file line number containing the insertion cursor (0..N-1)
   int m_cursorLineNo;
   /// the file line column of the insertion cursor (0..N-1)
   int m_cursorCol;
   ReLines* m_lines;
   QList<ReParagraph*> m_list;
   int m_maxCols;
   int m_screenWidth;
   /// true: the text cursor is visible (blinking)
   bool m_cursorVisible;
protected:
   static QStringList m_tabStrings;
   static QChar m_tabChar;
   static int m_tabWidth;
};

/**
 * Handles the mouse clicks inside of the edit field.
 */
class ReMouseCatcher {
public:
   enum ClickObjType {
      CO_UNDEF,
      CO_HSCROLLBAR,
      CO_HSLIDER,
      CO_VSCROLLBAR,
      CO_VSLIDER,
      CO_BOOKMARK
   };
   class ClickPosition: public QRect {
   public:
      ClickPosition(ClickObjType type) :
         QRect(0, 0, 0, 0),
         m_type(type),
         m_title(),
         m_object(NULL) {
      }
   public:
      bool operator <(const ClickPosition& op) {
         return y() < op.y() || (y() == op.y() && x() < op.x());
      }
   public:
      ClickObjType m_type;
      QString m_title;
      void* m_object;
   };
public:
   ReMouseCatcher();
   ~ReMouseCatcher();
public:
   void insertClickObject(ClickPosition* object);
   bool handleVScrollBar(QMouseEvent* event, bool isDragged, ReEdit* edit);
   bool handleHScrollBar(QMouseEvent* event, bool isDragged, ReEdit* edit);
public:
   QList<ClickPosition*> m_clickObjects;
   ClickPosition* m_vScrollBar;
   ClickPosition* m_hScrollBar;
   ClickPosition* m_hSlider;
   ClickPosition* m_vSlider;
   QPoint m_lastMousePosition;
   int m_lastTopVSlider;
   int m_lastLeftHSlider;
};

/**
 * An text display and edit unit.
 *
 * It displays a set of paragraphs with line number, vertical and horizontal
 * scroll bars and handle the key and mouse events to implement an editor.
 */
class ReEdit: public QWidget, protected ReMouseCatcher, public ReParagraphs {
   Q_OBJECT
public:
   enum EditorAction {
      EA_UNDEF,
      EA_CHAR_LEFT,
      EA_CHAR_RIGHT,
      EA_LINE_UP,
      EA_LINE_DOWN,
      EA_BEGIN_OF_LINE,
      EA_END_OF_LINE,
      EA_BEGIN_OF_FILE,
      EA_END_OF_FILE,
      EA_PAGE_UP,
      EA_PAGE_DOWN,
      EA_DEL_CHAR,
      EA_BACKSPACE,
      EA_DEL_END_OF_LINE,
      EA_DEL_BEGIN_OF_LINE,
      EA_DEL_LINE,
      EA_UNDO,
      EA_REDO,
      EA_VIEW_LEFT,
      EA_VIEW_RIGHT,
      EA_PAGE_LEFT,
      EA_PAGE_RIGHT,
      EA_NEWLINE,
   };
public:
   explicit ReEdit(QWidget* parent = 0);
   virtual ~ReEdit();
public:
   void assignColorsStandard();
   void assignKeysStandard();
   int cursorLineNo() const;
   void editorAction(EditorAction action);
   /** Returns the last column of the cursor line
    * @return -1:	the length of the cursor line is 0<br>
    *				otherwise: the last index of the cursor line
    */
   inline
   int lastColOfCurrent() const {
      return m_lines->lineAt(m_cursorLineNo).length() - 1;
   }
   ReLines& lines();
   ReLook* lookOf(ReLook::ForeGround foreground,
                  ReLook::BackGround background);
   /** Returns the current page size.
    * return    number of visible lines in the edit field
    */
   inline
   int pageSize() {
      return m_list.length();
   }
   void reposition(int firstLine, int firstCol);
   void setCursorLine(int cursorParagraph);
   /** Returns the array of the foreground colors.
    * @return the array of the foreground colors
    */
   inline const QColor* const* foregroundColors() const {
      return m_fontColors;
   }
   /** Returns the array of the background colors.
    * @return the array of the background colors
    */
   inline const QColor* const* backgroundColors() const {
      return m_brushColors;
   }
protected:
   QBrush* createBrush(ReLook::BackGround background);
   void drawScrollbars(QPainter& painter, const QRect& rect,
                       double sizeVertical, double posVertical, double sizeHorizontal,
                       double posHorizontal);
   void ensureCursorVisible();
protected slots:
   void keyPressEvent(QKeyEvent* event);
   void paintEvent(QPaintEvent*);
   void mouseMoveEvent(QMouseEvent* event);
   void mousePressEvent(QMouseEvent* event);
   void mouseReleaseEvent(QMouseEvent* event);
public:
   /** Returns the tabulator expanding string.
    * @param position   the position of the tabulator (0..N-1)
    * @return           the tabulator expanding string
    */
   inline static const QString& tabString(int position) {
      return m_tabStrings.at(position % m_tabWidth);
   }
   static void setTabStrings(int tabWidth);
protected:
   // number of pixels of the width of the edit field
   int m_widthEdit;
   // number of pixels of the height of the edit field
   int m_heightEdit;
   /// true: keys will be inserted at the cursor position
   bool m_insertMode;
   /// true: a file line will be displayed in multiple lines (if long enough)
   bool m_breakLines;
   /// number of pixels for the line number
   int m_widthLineNumbers;
   /// number of pixels for the right scroll bar
   int m_widthVScrollBar;
   /// number of pixels for the bottom scroll bar
   int m_heightHScrollBar;
   ReLook* m_looks[ReLook::BG_COUNT * ReLook::FG_COUNT];
   QBrush* m_standardBrush;
   QBrush* m_scrollbarBrush;
   QBrush* m_sliderBrush;
   const QColor* m_brushColors[ReLook::BG_COUNT];
   QPen* m_standardPen;
   QFont* m_standardFont;
   QFontMetrics* m_standardMetrics;
   const QColor* m_fontColors[ReLook::FG_COUNT];
   QMap<int, EditorAction> m_keyAlt;
   QMap<int, EditorAction> m_keyAltControl;
   QMap<int, EditorAction> m_keyAltControlShift;
   QMap<int, EditorAction> m_keyAltShift;
   QMap<int, EditorAction> m_keyControl;
   QMap<int, EditorAction> m_keyControlShift;
   QMap<int, EditorAction> m_keyRaw;
   QMap<int, EditorAction> m_keyShift;
   ReMouseCatcher m_mouseCatcher;
};

#endif // REEDITOR_HPP
