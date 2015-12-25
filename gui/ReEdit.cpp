/*
 * ReEdit.cpp
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
#include <QPaintEvent>

QStringList ReParagraphs::m_tabStrings;
// RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
QChar ReParagraphs::m_tabChar = QChar(0xBB);
int ReParagraphs::m_tabWidth = 4;

/**
 * Calculates the full line height (with gap between lines)
 *
 * @param height    the line height
 * @return          the full line height (with gap)
 */
inline int heightToFullHeight(int height) {
   return height * 7 / 6;
}

/**
 * Constructor.
 */
ReLook::ReLook() :
   m_font(NULL),
   m_metrics(NULL),
   m_foreground(FG_STANDARD),
   m_background(BG_STANDARD),
   m_edit(NULL) {
}

/**
 * Constructor.
 *
 * @param text  text of the part of paragraph
 * @param look  the presentation of the text
 */
ReEditText::ReEditText(const QString& text, ReLook* look) :
   m_text(text),
   m_look(look) {
}

/**
 * Sets the presentation of the cursor line.
 *
 * @param paragraph the paragraph to change
 * @param lineNo    the line number of the paragraph. Actions will done only
 *                  if the line number is the current line of the edit field
 * @param edit      the edit field (parent)
 */
void ReCursortLineBuilder::buildParagraph(ReParagraph& paragraph, int lineNo,
      ReEdit* edit) {
   if (lineNo == edit->cursorLineNo()) {
      for (int ix = 0; ix < paragraph.length(); ix++) {
         ReEditText* text = paragraph.at(ix);
         ReLook::ForeGround foreground = text->look()->m_foreground;
         switch (foreground) {
         case ReLook::FG_STANDARD:
            foreground = ReLook::FG_CURRENT_LINE;
            break;
         case ReLook::FG_SELECTED:
            foreground = ReLook::FG_CURRENT_SELECTED;
            break;
         default:
            // don't change!
            break;
         }
         ReLook::BackGround background = text->look()->m_background;
         switch (background) {
         case ReLook::BG_SELECTED:
            background = ReLook::BG_CURRENT_SELECTED;
            break;
         default:
            background = ReLook::BG_CURRENT_LINE;
            break;
         }
         text->setLook(edit->lookOf(foreground, background));
      }
   }
}

/**
 * Constructor.
 *
 * @param parent    NULL or a widget which destroy the instance
 */
ReEdit::ReEdit(QWidget* parent) :
   QWidget(parent),
   ReMouseCatcher(),
   m_widthEdit(0),
   m_heightEdit(0),
   m_insertMode(true),
   m_breakLines(false),
   m_widthLineNumbers(50),
   m_widthVScrollBar(16),
   m_heightHScrollBar(16),
   m_looks(),
   m_standardBrush(new QBrush(Qt::SolidPattern)),
   m_scrollbarBrush(new QBrush(Qt::SolidPattern)),
   m_sliderBrush(new QBrush(Qt::ConicalGradientPattern)),
   m_brushColors(),
   m_standardPen(new QPen(Qt::SolidLine)),
   m_standardFont(NULL),
   m_standardMetrics(NULL),
   m_fontColors(),
   m_keyAlt(),
   m_keyAltControl(),
   m_keyAltControlShift(),
   m_keyAltShift(),
   m_keyControl(),
   m_keyControlShift(),
   m_keyRaw(),
   m_keyShift() {
   setFocusPolicy(Qt::WheelFocus);
   m_standardFont = new QFont("Courier");
   m_standardFont->setStyleHint(QFont::TypeWriter);
   m_standardFont->setPixelSize(16);
   m_standardMetrics = new QFontMetrics(*m_standardFont);
   m_standardBrush->setColor(Qt::white);
   QColor color1(214, 210, 208);
   m_scrollbarBrush->setColor(color1);
   m_sliderBrush->setColor(Qt::lightGray);
   memset(m_looks, 0, sizeof m_looks);
   memset(m_brushColors, 0, sizeof m_brushColors);
   assignColorsStandard();
   setTabStrings(4);
   appendBuilder(new ReParagraphBuilder());
   appendBuilder(new ReCursortLineBuilder());
   assignKeysStandard();
}

/**
 * Destructor.
 */
ReEdit::~ReEdit() {
   for (size_t ix = 0; ix < sizeof(m_fontColors) / sizeof m_fontColors[0]; ix++)
      delete m_fontColors[ix];
   for (size_t ix = 0; ix < sizeof(m_brushColors) / sizeof m_brushColors[0]; ix++)
      delete m_brushColors[ix];
   delete m_standardBrush;
   delete m_scrollbarBrush;
   delete m_sliderBrush;
   delete m_standardPen;
   delete m_standardFont;
   delete m_standardMetrics;
}
/**
 * Assigns a standard version of colors to symbolic colors.
 */
void ReEdit::assignColorsStandard() {
   m_fontColors[ReLook::FG_STANDARD] = new QColor(Qt::black);
   m_fontColors[ReLook::FG_CURRENT_LINE] = new QColor(Qt::blue);
   m_fontColors[ReLook::FG_SELECTED] = new QColor(Qt::blue);
   m_fontColors[ReLook::FG_CURRENT_SELECTED] = new QColor(Qt::blue);
   m_fontColors[ReLook::FG_RED_LIGHT] = new QColor(Qt::red);
   m_fontColors[ReLook::FG_RED_DARK] = new QColor(Qt::darkRed);
   m_fontColors[ReLook::FG_BLUE_LIGHT] = new QColor(Qt::blue);
   m_fontColors[ReLook::FG_BLUE_DARK] = new QColor(Qt::darkBlue);
   m_fontColors[ReLook::FG_GREEN_LIGHT] = new QColor(Qt::green);
   m_fontColors[ReLook::FG_GREEN_DARK] = new QColor(Qt::darkGreen);
   m_fontColors[ReLook::FG_GREY_LIGHT] = new QColor(Qt::lightGray);
   m_fontColors[ReLook::FG_GREY_DARK] = new QColor(Qt::darkGray);
   m_fontColors[ReLook::FG_YELLOW_LIGHT] = new QColor(Qt::yellow);
   m_fontColors[ReLook::FG_YELLOW_DARK] = new QColor(Qt::darkYellow);
   m_fontColors[ReLook::FG_MAGENTA_LIGHT] = new QColor(Qt::magenta);
   m_fontColors[ReLook::FG_MAGENTA_DARK] = new QColor(Qt::darkMagenta);
   m_fontColors[ReLook::FG_CYAN_LIGHT] = new QColor(Qt::cyan);
   m_fontColors[ReLook::FG_CYAN_DARK] = new QColor(Qt::darkCyan);
   m_brushColors[ReLook::BG_STANDARD] = new QColor(Qt::white);
   m_brushColors[ReLook::BG_CURRENT_LINE] = new QColor(Qt::lightGray);
   m_brushColors[ReLook::BG_SELECTED] = new QColor(Qt::blue);
   m_brushColors[ReLook::BG_CURRENT_SELECTED] = new QColor(Qt::blue);
   m_brushColors[ReLook::BG_SCROLLBAR] = new QColor(216, 214, 212);
   m_brushColors[ReLook::BG_SLIDER] = new QColor(231, 230, 228);
   m_brushColors[ReLook::BG_SEARCHED] = new QColor(Qt::yellow);
   m_brushColors[ReLook::BG_SAME_WORD] = new QColor(Qt::yellow);
   m_brushColors[ReLook::BG_YELLOW] = new QColor(Qt::yellow);
   m_brushColors[ReLook::BG_GREY] = new QColor(Qt::lightGray);
   m_brushColors[ReLook::BG_RED] = new QColor(Qt::red);
   m_brushColors[ReLook::BG_GREEN] = new QColor(Qt::green);
   m_brushColors[ReLook::BG_BLUE] = new QColor(Qt::blue);
}

/**
 * Assigns a the standard version of keys to editor actions.
 */
void ReEdit::assignKeysStandard() {
   m_keyRaw.clear();
   m_keyControl.clear();
   m_keyShift.clear();
   m_keyAlt.clear();
   m_keyAltControl.clear();
   m_keyAltControlShift.clear();
   m_keyControlShift.clear();
   m_keyRaw[Qt::Key_Left] = EA_CHAR_LEFT;
   m_keyRaw[Qt::Key_Right] = EA_CHAR_RIGHT;
   m_keyRaw[Qt::Key_Up] = EA_LINE_UP;
   m_keyRaw[Qt::Key_Down] = EA_LINE_DOWN;
   m_keyRaw[Qt::Key_Home] = EA_BEGIN_OF_LINE;
   m_keyRaw[Qt::Key_End] = EA_END_OF_LINE;
   m_keyControl[Qt::Key_Home] = EA_BEGIN_OF_FILE;
   m_keyControl[Qt::Key_End] = EA_END_OF_FILE;
   m_keyRaw[Qt::Key_PageUp] = EA_PAGE_UP;
   m_keyRaw[Qt::Key_PageDown] = EA_PAGE_DOWN;
   m_keyRaw[Qt::Key_Delete] = EA_DEL_CHAR;
   m_keyRaw[Qt::Key_Backspace] = EA_BACKSPACE;
   m_keyControl[Qt::Key_Delete] = EA_DEL_END_OF_LINE;
   m_keyControl[Qt::Key_Backspace] = EA_DEL_BEGIN_OF_LINE;
   m_keyShift[Qt::Key_Delete] = EA_DEL_LINE;
   m_keyControl[Qt::Key_Z] = EA_UNDO;
   m_keyControlShift[Qt::Key_Z] = EA_REDO;
   m_keyAltControl[Qt::Key_Left] = EA_VIEW_LEFT;
   m_keyAltControl[Qt::Key_Right] = EA_VIEW_RIGHT;
   m_keyAltControlShift[Qt::Key_Left] = EA_PAGE_LEFT;
   m_keyAltControlShift[Qt::Key_Right] = EA_PAGE_RIGHT;
   m_keyRaw[Qt::Key_Return] = EA_NEWLINE;
   m_keyRaw[Qt::Key_Enter] = EA_NEWLINE;
}

/**
 * Calculates the size/position of a slider from a relative value [0..1].
 * @param size          the size of the scrollbar (width/height for horiz/vert bar)
 * @param minSize       the minimum size of the slider
 * @param sizeFactor    the factor of the slider size: [0..1]
 * @param posFactor     the factor of the slider position [0..1]
 * @param position      OUT: the slider position in pixel from the start
 * @param length        OUT: the slider length in pixel
 */
void calcSliderSize(int size, int minSize, double sizeFactor, double posFactor,
                    int& position, int& length) {
   if (sizeFactor > 1.0)
      sizeFactor = 1.0;
   if (posFactor > 100)
      posFactor = 100;
   length = roundInt(size * sizeFactor);
   if (length < minSize)
      length = minSize;
   position = roundInt((size - length) * posFactor);
   //ReLogger::globalLogger()->logv(LOG_INFO, 4,
   //	"calcSize: size: %d / %d sizefac: %.2f posfac: %.2f: pos/len: %d/%d",
   //		size, minSize, sizeFactor, posFactor, position, length);
}

/**
 * Creates a brush with a given color.
 *
 * @param background    the symbolic background color
 * @return  an new created instance of a brush
 */
QBrush* ReEdit::createBrush(ReLook::BackGround background) {
   QBrush* rc = new QBrush();
   *rc = *m_standardBrush;
   rc->setColor(*m_brushColors[background]);
   return rc;
}

/**
 * Returns the line number of the cursor line.
 *
 * @return the line number [0..N-1]
 */
int ReEdit::cursorLineNo() const {
   return m_cursorLineNo;
}

/**
 * Draws the scrollbars.
 *
 * @param painter           the paint unit
 * @param rect              the full area of the edit field
 * @param sizeVertical      the size of the scrollbar as factor [0..1]
 * @param posVertical       the position of the scrollbar as factor [0..1]
 * @param sizeHorizontal    the size of the scrollbar as factor [0..1]
 * @param posHorizontal     the position of the scrollbar as factor [0..1]
 */
void ReEdit::drawScrollbars(QPainter& painter, const QRect& rect,
                            double sizeVertical, double posVertical, double sizeHorizontal,
                            double posHorizontal) {
   // We paint the vertical scrollbar:
   QBrush brush(*m_brushColors[ReLook::BG_SCROLLBAR], Qt::SolidPattern);
   painter.setBrush(brush);
   int x = rect.right() - m_widthVScrollBar;
   // Note: the frame is outside of the rectangle (edit field too!):
   static int width = 1;
   static int width2 = 2 * width;
   m_vScrollBar->setRect(x + width2, rect.top(), m_widthVScrollBar - width2,
                         rect.height() - m_heightHScrollBar - width);
   painter.drawRect(*m_vScrollBar);
   // We paint the horizontal scrollbar:
   m_hScrollBar->setRect(rect.left() + m_widthLineNumbers,
                         rect.bottom() - m_heightHScrollBar + width,
                         rect.width() - m_widthVScrollBar - m_widthLineNumbers,
                         m_heightHScrollBar - width2);
   painter.drawRect(*m_hScrollBar);
   // Slider (vertical)
   QBrush brush2(*m_brushColors[ReLook::BG_SLIDER], Qt::SolidPattern);
   painter.setBrush(brush2);
   int sliderSize = 0;
   int sliderPos = 0;
   calcSliderSize(rect.height() - m_heightHScrollBar, m_heightHScrollBar,
                  sizeVertical, posVertical, sliderPos, sliderSize);
   m_vSlider->setRect(x + width2, rect.top() + sliderPos + width,
                      m_widthVScrollBar - width2, sliderSize - width2);
   painter.drawRect(*m_vSlider);
   int middle = m_vSlider->top() + m_vSlider->height() / 2;
   painter.drawLine(m_vSlider->left() + 2, middle, m_vSlider->right() - 2,
                    middle);
   middle -= m_heightHScrollBar / 2 - 2;
   painter.drawLine(m_vSlider->left() + 2, middle, m_vSlider->right() - 2,
                    middle);
   middle += m_heightHScrollBar - 4;
   painter.drawLine(m_vSlider->left() + 2, middle, m_vSlider->right() - 2,
                    middle);
   // Slider (horizontal)
   calcSliderSize(rect.width() - m_widthLineNumbers - m_widthVScrollBar,
                  m_heightHScrollBar, sizeHorizontal, posHorizontal, sliderPos,
                  sliderSize);
   m_hSlider->setRect(rect.left() + m_widthLineNumbers + sliderPos,
                      rect.bottom() - m_heightHScrollBar + width, sliderSize - width,
                      m_heightHScrollBar - width2);
   painter.drawRect(*m_hSlider);
   middle = m_hSlider->left() + m_hSlider->width() / 2;
   painter.drawLine(middle, m_hSlider->top() + 2, middle,
                    m_hSlider->bottom() - 2);
   middle -= m_heightHScrollBar / 2 - 2;
   painter.drawLine(middle, m_hSlider->top() + 2, middle,
                    m_hSlider->bottom() - 2);
   middle += m_heightHScrollBar - 4;
   painter.drawLine(middle, m_hSlider->top() + 2, middle,
                    m_hSlider->bottom() - 2);
}

/**
 * Does an editor action.
 *
 * @param action    action to do
 */
void ReEdit::editorAction(ReEdit::EditorAction action) {
   int pageSize = m_list.length() - 1;
   switch (action) {
   case EA_UNDEF:
      break;
   case EA_NEWLINE:
      m_lines->splitLine(m_cursorLineNo, m_cursorCol + 1, true);
      m_cursorCol = -1;
      m_cursorLineNo++;
      break;
   case EA_CHAR_LEFT:
      if (--m_cursorCol < -1) {
         if (m_cursorLineNo == 0)
            m_cursorCol = -1;
         else {
            m_cursorLineNo--;
            m_cursorCol = lastColOfCurrent();
         }
      } else if (m_cursorCol >= lastColOfCurrent())
         m_cursorCol = lastColOfCurrent() - 1;
      ensureCursorVisible();
      break;
   case EA_CHAR_RIGHT:
      if (++m_cursorCol > lastColOfCurrent()) {
         int oldLine = m_cursorLineNo;
         m_cursorCol = -1;
         editorAction(EA_LINE_DOWN);
         if (m_cursorLineNo == oldLine)
            m_cursorCol = lastColOfCurrent();
      }
      ensureCursorVisible();
      break;
   case EA_LINE_UP:
      if (--m_cursorLineNo < 0)
         m_cursorLineNo = 0;
      ensureCursorVisible();
      break;
   case EA_LINE_DOWN:
      if (++m_cursorLineNo >= m_lines->lineCount())
         m_cursorLineNo = m_lines->lineCount() - 1;
      ensureCursorVisible();
      break;
   case EA_BEGIN_OF_LINE:
      m_cursorCol = -1;
      ensureCursorVisible();
      break;
   case EA_END_OF_LINE:
      m_cursorCol = lastColOfCurrent();
      ensureCursorVisible();
      break;
   case EA_BEGIN_OF_FILE:
      m_cursorLineNo = 0;
      m_cursorCol = -1;
      ensureCursorVisible();
      break;
   case EA_END_OF_FILE:
      m_cursorLineNo = m_lines->lineCount() - 1;
      m_cursorCol = lastColOfCurrent();
      ensureCursorVisible();
      break;
   case EA_PAGE_UP:
      // Do not change cursor line!
      reposition(m_firstLine - pageSize, m_firstCol);
      break;
   case EA_PAGE_DOWN:
      // Do not change cursor line!
      reposition(m_firstLine + pageSize, m_firstCol);
      break;
   case EA_DEL_CHAR:
      m_lines->removePart(m_cursorLineNo, m_cursorCol + 1, 1, true);
      break;
   case EA_BACKSPACE: {
      int currentCol = m_cursorCol;
      m_cursorCol = max(-1, currentCol - 1);
      if (currentCol == -1 && m_cursorLineNo > 0) {
         // join the previous and the current line:
         // the cursor position will be the end of the previous line:
         m_cursorCol = m_lines->lineAt(m_cursorLineNo - 1).length() - 1;
      }
      if (m_lines->removePart(m_cursorLineNo, columnToIndex(currentCol), 1,
                              true))
         m_cursorLineNo = max(0, m_cursorLineNo - 1);
      break;
   }
   case EA_DEL_END_OF_LINE: {
      int lastIx = lastColOfCurrent();
      if (m_cursorCol <= lastIx) {
         m_lines->removePart(m_cursorLineNo, m_cursorCol + 1,
                             lastIx - m_cursorCol, true);
         ensureCursorVisible();
      }
      break;
   }
   case EA_DEL_BEGIN_OF_LINE:
      if (m_cursorCol >= 0) {
         m_lines->removePart(m_cursorLineNo, 0, m_cursorCol + 1, true);
         m_cursorCol = -1;
         ensureCursorVisible();
      }
      break;
   case EA_DEL_LINE:
      m_lines->removeLines(m_cursorLineNo, 1, true);
      m_cursorLineNo = min(m_cursorLineNo, m_lines->lineCount() - 1);
      m_cursorCol = m_firstCol == 0 ? -1 : m_firstCol;
      reposition(min(m_firstLine, m_firstLine - pageSize), m_firstCol - 1);
      ensureCursorVisible();
      break;
   case EA_UNDO:
      m_lines->undo(m_cursorLineNo, m_cursorCol);
      break;
   case EA_REDO:
      break;
   case EA_VIEW_LEFT:
      reposition(m_firstLine, m_firstCol - 1);
      break;
   case EA_VIEW_RIGHT:
      reposition(m_firstLine, m_firstCol + 1);
      break;
   case EA_PAGE_RIGHT:
      reposition(m_firstLine, m_firstCol + m_screenWidth);
      break;
   case EA_PAGE_LEFT:
      reposition(m_firstLine, m_firstCol - m_screenWidth);
      break;
   default:
      break;
   }
   emit repaint();
}

/**
 * Ensures that the cursor is visible.
 */
void ReEdit::ensureCursorVisible() {
   if (m_cursorLineNo < m_firstLine
         || m_cursorLineNo >= m_firstLine + pageSize()) {
      reposition(m_cursorLineNo, m_cursorCol);
   }
   if (m_cursorCol < 0)
      m_firstCol = 0;
   else if (m_cursorCol < m_firstCol) {
      m_firstCol = m_cursorCol;
   } else if (m_cursorCol >= m_firstCol + m_screenWidth) {
      int length = lastColOfCurrent() + 1;
      m_firstCol = max(0, min(m_cursorCol, length - m_screenWidth));
   }
}

/**
 * Handles the key click event.
 *
 * @param event     the description of the key click
 */
void ReEdit::keyPressEvent(QKeyEvent* event) {
   bool shift = event->modifiers() & Qt::ShiftModifier;
   bool control = event->modifiers() & Qt::ControlModifier;
   bool alt = event->modifiers() & Qt::MetaModifier;
   int key = event->key();
   QString keyText = event->text();
   if (!keyText.isEmpty() && !shift && !control && !alt) {
      switch (key) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Delete:
      case Qt::Key_Backspace:
         editorAction(m_keyRaw[key]);
         break;
      default:
         // the inserted char should be visible:
         ensureCursorVisible();
         m_lines->insertText(m_cursorLineNo, m_cursorCol + 1, keyText);
         m_cursorCol++;
         // the new position should be visible too:
         ensureCursorVisible();
         break;
      }
   } else if (shift && !keyText.isEmpty() && key != Qt::Key_Delete
              && key != Qt::Key_Backspace) {
      m_lines->insertText(m_cursorLineNo, m_cursorCol + 1, keyText);
      m_cursorCol++;
   } else {
      QMap<int, EditorAction>* map;
      if (!shift && !alt && !control)
         map = &m_keyRaw;
      else if (shift && !alt && !control)
         map = &m_keyShift;
      else if (alt && !shift && !control)
         map = &m_keyAlt;
      else if (control && !alt && !shift)
         map = &m_keyControl;
      else if (alt && control && !shift)
         map = &m_keyAltControl;
      else if (control && shift && !alt)
         map = &m_keyControlShift;
      else
         map = &m_keyAltControlShift;
      if (map->contains(event->key())) {
         EditorAction action = (*map)[key];
         editorAction(action);
      }
   }
   emit repaint();
}

/**
 * Returns the text source of the instance.
 *
 * @return  the text source
 */
ReLines& ReEdit::lines() {
   if (m_lines == NULL)
      m_lines = new ReLines();
   return *m_lines;
}

/**
 * Returns a look with the given specifications.
 *
 * @param foreground    the symbolic name of the foreground
 * @param background    the symbolic name of the background
 * @return
 */
ReLook* ReEdit::lookOf(ReLook::ForeGround foreground,
                       ReLook::BackGround background) {
   int index = foreground * ReLook::BG_COUNT + background;
   ReLook* rc = m_looks[index];
   if (rc == NULL) {
      rc = m_looks[index] = new ReLook();
      rc->m_edit = this;
      rc->m_foreground = foreground;
      rc->m_background = background;
      rc->m_brush = createBrush(background);
      if (foreground != ReLook::FG_CURRENT_LINE)
         rc->m_font = m_standardFont;
      else {
         rc->m_font = new QFont(*m_standardFont);
         rc->m_font->setBold(true);
      }
      rc->m_metrics = m_standardMetrics;
      rc->m_pen = new QPen(*m_standardPen);
      rc->m_pen->setColor(*m_fontColors[foreground]);
   }
   return rc;
}
/**
 * Handles the event when a drag action is done
 * @param event
 */
void ReEdit::mouseMoveEvent(QMouseEvent* event) {
   if (m_lastMousePosition.x() >= 0
         && (handleHScrollBar(event, true, this)
             || handleVScrollBar(event, true, this))) {
      emit repaint();
   }
}

/**
 * Handles the mouse click event.
 *
 * @param event     the description of the mouse click
 */
void ReEdit::mousePressEvent(QMouseEvent* event) {
   if (handleVScrollBar(event, false, this)) {
   } else if (handleHScrollBar(event, false, this)) {
   } else {
      QPoint position = event->pos();
      m_cursorLineNo = position.y()
                       / heightToFullHeight(m_standardMetrics->height()) + m_firstLine;
      int x = position.x();
      int charWidth = m_standardMetrics->width('x');
      x -= m_widthLineNumbers;
      if (x >= 0
            && x < m_widthEdit - m_widthLineNumbers - m_widthVScrollBar) {
         if (x <= +charWidth / 2)
            m_cursorCol = m_firstCol - 1;
         else
            m_cursorCol = m_firstCol + columnToIndex(x / charWidth);
      }
   }
   m_lastMousePosition = event->pos();
   m_lastTopVSlider = m_vSlider->top() - m_vScrollBar->top();
   m_lastLeftHSlider = m_hSlider->left() - m_hScrollBar->left();
   emit repaint();
}

/**
 * Handles the mouse click event.
 *
 * @param event     the description of the mouse click
 */
void ReEdit::mouseReleaseEvent(QMouseEvent* event) {
   // avoid warning:
   ReUseParameter(event);
   m_lastMousePosition.setX(-1);
   m_lastMousePosition.setY(-1);
}
inline static double fraction(int a, int b, double exception) {
   double rc = b == 0 ? exception : (double) a / b;
   return rc;
}

/**
 * Draw the edit field.
 *
 * @param event     the trigger event
 */
void ReEdit::paintEvent(QPaintEvent* event) {
   clock_t start = clock();
   QRect rect = event->rect();
   m_widthEdit = rect.width();
   m_heightEdit = rect.height();
   int lineHeight = heightToFullHeight(m_standardMetrics->height());
   int pageSize = (rect.height() - m_heightHScrollBar) / lineHeight;
   int charWidth = m_standardMetrics->averageCharWidth();
   int pageWidth = (rect.width() - m_widthVScrollBar - m_widthLineNumbers)
                   / charWidth;
   int firstLine = m_firstLine;
   load(firstLine, pageSize, pageWidth, this);
   QPainter painter(this);
   ReLook* look = lookOf(ReLook::FG_STANDARD, ReLook::BG_STANDARD);
   painter.setBrush(*look->m_brush);
   QRect editArea(rect.left() + m_widthLineNumbers, rect.top(),
                  rect.right() - m_widthVScrollBar, rect.bottom() - m_heightHScrollBar);
   // Painting the frame of the edit field:
   painter.drawRect(editArea);
   // Painting the edit field area (text...)
   draw(painter, rect.top(), rect.left() + m_widthLineNumbers);
   // Painting the line numbers:
   int left = rect.left() + m_widthLineNumbers - 3;
   left = rect.left();
   int y = 0;
   int lineNo = firstLine + 1;
   ReLook* lookStd = lookOf(ReLook::FG_STANDARD, ReLook::BG_STANDARD);
   int maxIx = min(m_list.length(), m_lines->lineCount() - m_firstLine);
   for (int ix = 0; ix < maxIx; ix++, lineNo++) {
      QString number = QString::number(lineNo);
      ReLook* look =
         lineNo == m_cursorLineNo + 1 ?
         lookOf(ReLook::FG_CURRENT_LINE, ReLook::BG_CURRENT_LINE) :
         lookStd;
      int width = look->m_metrics->width(number);
      if (ix == 0)
         y = rect.top() + look->m_metrics->height()
             - look->m_metrics->descent();
      painter.setFont(*look->m_font);
      painter.setPen(*look->m_pen);
      painter.drawText(left + m_widthLineNumbers - width - 5, y, number);
      y += lineHeight;
   }
   // We paint the cursor:
   if (m_cursorVisible && m_cursorLineNo >= firstLine
         && m_cursorLineNo < firstLine + pageSize) {
      ReParagraph* cursorPara = cursorParagraph();
      int col = min(m_cursorCol, cursorPara->m_columns - 1);
      col = indexToColumn(col + 1, m_tabWidth,
                          m_lines->lineAt(m_cursorLineNo)) - m_firstCol;
      int x = rect.left() + m_widthLineNumbers + 1
              + col * lookStd->m_metrics->width('x');
      int y = rect.top() + (m_cursorLineNo - firstLine) * lineHeight;
      painter.setPen(*look->m_pen);
      painter.drawLine(x, y, x, y + lineHeight);
   }
   int maxLines = max(1, m_lines->lineCount() - pageSize);
   drawScrollbars(painter, rect, fraction(pageSize, maxLines, 1.0),
                  fraction(m_firstLine, maxLines, 0.0),
                  fraction(m_screenWidth, m_maxCols, 1.0),
                  fraction(m_firstCol, max(0, m_maxCols - m_screenWidth), 0.0));
   ReLogger::globalLogger()->logv(LOG_INFO, 3, "draw: %.4f",
                                  double(clock() - start) / CLOCKS_PER_SEC);
}

/**
 * Ensures that a given line / column is visible.
 *
 * @param firstLine	number of the line which should be visible
 * @param firstCol	the first column which should be visible
 */
void ReEdit::reposition(int firstLine, int firstCol) {
   if (firstLine != m_firstLine) {
      int pageSize = m_list.length();
      if (firstLine <= 0)
         firstLine = 0;
      else if (firstLine >= m_lines->lineCount() - pageSize)
         firstLine = m_lines->lineCount() - pageSize + 1;
      // We do not load because each redraw loads it:
      m_firstLine = firstLine;
   }
   if (firstCol != m_firstCol) {
      if (firstCol < 0)
         firstCol = 0;
      else if (firstCol > m_maxCols - m_screenWidth)
         firstCol = max(0, m_maxCols - m_screenWidth);
      m_firstCol = firstCol;
   }
}

/**
 * Sets the line number of the cursor line.
 * @param cursorLine    the line number [0..N-1]
 */
void ReEdit::setCursorLine(int cursorLine) {
   m_cursorLineNo = cursorLine;
}

/**
 * Calculates the tabulator expanding strings.
 *
 * Example (tab width = 3):
 * <pre>
 *   "\tz" -> '   z' -> TAB + '  '
 *  "x\tz" -> 'x  z' -> TAB + ' '
 * "xy\tz" -> 'xy z' -> TAB
 * </pre>
 * @param tabWidth
 */
void ReEdit::setTabStrings(int tabWidth) {
   m_tabWidth = tabWidth;
   m_tabStrings.clear();
   QString blanks;
   blanks.fill(' ', tabWidth);
   for (int ix = 0; ix < tabWidth; ix++) {
      m_tabStrings.append(m_tabChar + blanks.mid(0, tabWidth - 1 - ix));
   }
}

/**
 * Constructor.
 */
ReParagraphs::ReParagraphs() :
   m_builders(),
   m_firstLine(0),
   m_firstCol(0),
   m_cursorLineNo(0),
   m_cursorCol(-1),
   m_lines(NULL),
   m_list(),
   m_maxCols(0),
   m_screenWidth(0),
   m_cursorVisible(true) {
}

/**
 * Destructor.
 */
ReParagraphs::~ReParagraphs() {
   clear();
}

/**
 * Makes the list empty and frees the resources.
 */
void ReParagraphs::clear() {
   m_maxCols = 0;
   for (int ix = m_list.length() - 1; ix >= 0; ix--) {
      ReParagraph* current = m_list.at(ix);
      delete current;
   }
   m_list.clear();
}

/**
 * Calculates the line string index from the screen column.
 *
 * The index may be larger because of expanded tabulators.
 * Example:
 * <pre>"x\ty", tabulator width: 4
 * _01230123
 * _	x	index: 0 blanks: 3
 * _x	x	index: 1 blanks: 2
 * _xx	x	index: 2 blanks: 1
 * _xxx	x	index: 3 blanks: 0
 * </pre>
 * @param column	the column to convert (0..N-1)
 * @param tabWidth	the tabulator width: the maximal width of a tabulator on the screen
 * @param string	the string which will displayed
 * @return			-1: column < 0<br>
 *					otherwise: the position of the string index in a string with
 *					expanded tabs
 */
int ReParagraphs::columnToIndex(int column, int tabWidth,
                                const QString& string) {
   int rc = 0;
   if (column < 0)
      rc = -1;
   else if (column == 0)
      rc = 0;
   else {
      int cursor = 0;
      int length = string.length();
      while (rc < length - 1 && cursor < column) {
         rc++;
         if (string.at(rc) != m_tabChar)
            cursor++;
         else {
            cursor += tabWidth - 1 - (cursor % tabWidth);
            if (cursor >= column) {
               break;
            }
         }
      }
   }
   return rc;
}

/**
 * Returns the cursor line.
 *
 * @return the cursor line
 */
ReParagraph* ReParagraphs::cursorParagraph() {
   ReParagraph* rc = NULL;
   if (m_cursorLineNo >= m_firstLine
         && m_cursorLineNo < m_firstLine + m_list.length()) {
      rc = m_list.at(m_cursorLineNo - m_firstLine);
   }
   return rc;
}

/**
 * Calculates the index of the line string from the cursor column (of the current line).
 *
 * @param cursorCol     the cursor position
 * @return              the index of the content string
 */
int ReParagraphs::columnToIndex(int cursorCol) {
   int rc = columnToIndex(cursorCol, m_tabWidth,
                          m_lines->lineAt(m_cursorLineNo));
   return rc;
}

/**
 * Renders the texts of the paragraph list.
 *
 * @param painter
 * @param top       the position of the first line
 * @param left      the position of the first line
 */
void ReParagraphs::draw(QPainter& painter, int top, int left) {
   for (int ix = 0; ix < m_list.length(); ix++) {
      ReParagraph* current = m_list.at(ix);
      current->draw(painter, top, left);
      //if (top > rect.y()  + rect.height())
      //    break;
   }
}

/**
 * Calculates the cursor column from the line string index.
 *
 * The index may be larger because of expanded tabulators.
 * Example:
 * <pre>tabulator width: 4
 * _01230123
 * _	x	index: 0 blanks: 3
 * _x	x	index: 1 blanks: 2
 * _xx	x	index: 2 blanks: 1
 * _xxx	x	index: 3 blanks: 0
 * </pre>
 * @param index
 * @param tabWidth
 * @param string
 * @return
 */
int ReParagraphs::indexToColumn(int index, int tabWidth,
                                const QString& string) {
   int rc = 0;
   if (index > 0) {
      int length = string.length();
      for (int cursor = 0; cursor < index && cursor < length; cursor++) {
         if (string.at(cursor) != '\t')
            rc++;
         else {
            // if the cursor points to the last char (tab): do not expand!
            if (cursor < index)
               rc += tabWidth - (cursor % tabWidth);
         }
      }
   }
   return rc;
}

/**
 * Calculates the cursor column from the line string index (of the current line).
 *
 * @param index the index in the line string
 * @return      the column in the edit field
 */
int ReParagraphs::indexToColumn(int index) {
   int rc = indexToColumn(index, m_tabWidth, m_lines->lineAt(m_cursorLineNo));
   return rc;
}

/**
 * Transfers some lines starting with a given start into a paragraph list.
 *
 * @param lineNo    the line number of the first line to transfer
 * @param count     the number of lines to transfer
 * @param width		width of the screen (in chars)
 * @param edit		the parent (instance of the editor)
 */
void ReParagraphs::load(int lineNo, int count, int width, ReEdit* edit) {
   clear();
   m_maxCols = 0;
   m_firstLine = lineNo;
   m_screenWidth = width;
   for (int ix = lineNo; ix < lineNo + count; ix++) {
      ReParagraph* para = new ReParagraph();
      m_list.append(para);
      for (int builder = 0; builder < m_builders.length(); builder++)
         m_builders.at(builder)->buildParagraph(*para, ix, edit);
      m_maxCols = max(m_maxCols, para->m_columns);
   }
}

/**
 * Sets the text source of the instance.
 *
 * @param lines the new text source
 */
void ReParagraphs::setLines(ReLines* lines) {
   m_lines = lines;
}

/**
 * Changes the look of a paragraph.
 *
 * @param paragraph the paragraph to change
 * @param lineNo    the line number (0..N-1) of the paragraph in the source
 * @param edit      the parent, the edit field
 */
void ReParagraphBuilder::buildParagraph(ReParagraph& paragraph, int lineNo,
                                        ReEdit* edit) {
   if (paragraph.length() == 0) {
      int firstCol = edit->m_firstCol;
      const QString& text = edit->lines().lineAt(lineNo);
      ReLook* look = edit->lookOf(ReLook::FG_STANDARD, ReLook::BG_STANDARD);
      ReLook* lookTab = edit->lookOf(ReLook::FG_GREY_LIGHT,
                                     ReLook::BG_STANDARD);
      paragraph.m_columns = 0;
      int ixTab;
      ReEditText* part;
      int start = 0;
      int cursor = 0;
      int length, length2, start2;
      int maxCol = edit->m_maxCols;
      while ((ixTab = text.indexOf('\t', start)) >= 0) {
         if (ixTab > start) {
            length = ixTab - start;
            if (cursor + length > firstCol && cursor < maxCol) {
               start2 = start;
               length2 = length;
               if (cursor < firstCol) {
                  start2 -= firstCol - cursor;
                  length2 -= firstCol - cursor;
               }
               part = new ReEditText(text.mid(start2, length2), look);
               paragraph.append(part);
            }
            cursor += length;
         }
         QString tabs = ReEdit::tabString(cursor);
         length = tabs.length();
         if (cursor + length > firstCol && cursor < maxCol) {
            if (cursor < firstCol)
               tabs = tabs.left(length - firstCol - cursor);
            paragraph.append(new ReEditText(tabs, lookTab));
         }
         cursor += length;
         start = ixTab + 1;
      }
      if (cursor < firstCol) {
         start2 = start + (firstCol - cursor);
         cursor += text.length() - start;
         part = new ReEditText(text.mid(start2), look);
      } else {
         cursor += text.length() - start;
         part = new ReEditText(start == 0 ? text : text.mid(start), look);
      }
      paragraph.m_columns = cursor;
      paragraph.append(part);
   }
}

/**
 * Destructor.
 */
ReParagraph::~ReParagraph() {
   for (int ix = length() - 1; ix >= 0; ix--)
      delete at(ix);
   clear();
}

/**
 * Renders a paragraph.
 *
 * @param painter   the painting unit
 * @param top       IN/OUT: the y position<br>
 *                  IN: the most top pixel of the line to draw
 *                  OUT: the first position under the drawn line
 * @param left      the starting x position (left hand)
 */
void ReParagraph::draw(QPainter& painter, int& top, int left) {
   int x = left;
   QFontMetrics* metrics = at(0)->look()->m_metrics;
   x += metrics->width('x') / 2;
   int height = metrics->height();
   int y = top + height - metrics->descent();
   top += heightToFullHeight(height);
   for (int ix = 0; ix < length(); ix++) {
      ReEditText* current = at(ix);
      ReLook* look = current->look();
      painter.setFont(*look->m_font);
      const ReLook::ForeGround fg = look->m_foreground;
      QPen pen(*look->m_edit->foregroundColors()[fg]);
      painter.setPen(pen);
      painter.drawText(x, y, current->text());
      x += metrics->width(current->text());
   }
}

/**
 * Constructor.
 */
ReMouseCatcher::ReMouseCatcher() :
   m_clickObjects(),
   m_vScrollBar(new ClickPosition(CO_VSCROLLBAR)),
   m_hScrollBar(new ClickPosition(CO_HSCROLLBAR)),
   m_hSlider(new ClickPosition(CO_HSLIDER)),
   m_vSlider(new ClickPosition(CO_VSLIDER)),
   m_lastMousePosition(),
   m_lastTopVSlider(0),
   m_lastLeftHSlider(0) {
}
/**
 * Destructor.
 */
ReMouseCatcher::~ReMouseCatcher() {
   delete m_vScrollBar;
   delete m_hScrollBar;
   delete m_vSlider;
   delete m_hSlider;
   m_vScrollBar = m_hScrollBar = m_vSlider = m_hSlider = NULL;
}

/**
 * Inserts an object which can be clicked into the list.
 *
 * @param object    the object to insert.
 */
void ReMouseCatcher::insertClickObject(ReMouseCatcher::ClickPosition* object) {
   if (!m_clickObjects.contains(object))
      m_clickObjects.append(object);
}

/**
 * Does some things if the mouse position is inside the horizontal scrollbar.
 *
 * @param event the mouse click event
 * @param isDragged <code>true</code>: called from <code>mouseMoveEvent</code>
 * @param edit  the edit field
 * @return      <code>true</code>: the mouse click is inside the horizontal sb
 */
bool ReMouseCatcher::handleHScrollBar(QMouseEvent* event, bool isDragged,
                                      ReEdit* edit) {
   QPoint pos = event->pos();
   bool rc = rectContains(*m_hScrollBar, pos, "hScrollBar")
             || (isDragged && m_hScrollBar->contains(m_lastMousePosition));
   if (rc) {
      if (isDragged) {
         int distance = pos.x() - m_lastMousePosition.x();
         int sliderPos = m_lastLeftHSlider + distance;
         int moveGap = m_hScrollBar->width() - m_hSlider->width();
         double position = moveGap == 0 ? 0.0 : double(sliderPos) / moveGap;
         int col = roundInt(
                      (edit->m_maxCols - edit->m_screenWidth)
                      * max(0.0, min(position, 1.0)));
         //ReLogger::globalLogger()->logv(LOG_INFO, 4,
         //	"x: %d dist: %d last: %d slPos: %d pos: %.2f gap: %d col: %d / %d scw: %d",
         //	pos.x(), distance, m_lastLeftHSlider, sliderPos, position, moveGap,
         //	col, edit->m_maxCols, edit->m_screenWidth);
         edit->reposition(edit->m_firstLine, col);
      } else {
         if (pos.x() < m_hSlider->left())
            edit->editorAction(ReEdit::EA_PAGE_LEFT);
         else if (pos.x() > m_hSlider->right())
            edit->editorAction(ReEdit::EA_PAGE_RIGHT);
      }
   }
   return rc;
}

/**
 * Does some things if the mouse position is inside the vertical scrollbar.
 *
 * @param event     the mouse click event
 * @param isDragged <code>true</code>: called from <code>mouseMoveEvent</code>
 * @param edit      the edit field
 * @return          <code>true</code>: the mouse click is inside the vertical sb
 */
bool ReMouseCatcher::handleVScrollBar(QMouseEvent* event, bool isDragged,
                                      ReEdit* edit) {
   QPoint pos = event->pos();
   bool rc = rectContains(*m_vScrollBar, pos, "vScrollBar")
             || (isDragged && m_vScrollBar->contains(m_lastMousePosition));
   if (rc) {
      if (isDragged) {
         int distance = pos.y() - m_lastMousePosition.y();
         int sliderPos = m_lastTopVSlider + distance;
         int moveGap = m_vScrollBar->height() - m_vSlider->height();
         double position = moveGap == 0 ? 0.0 : double(sliderPos) / moveGap;
         int line = roundInt(
                       (edit->lines().lineCount() - edit->pageSize())
                       * max(0.0, min(position, 1.0)));
         edit->reposition(line, edit->m_cursorCol);
      } else {
         if (pos.y() < m_vSlider->top())
            edit->editorAction(ReEdit::EA_PAGE_UP);
         else if (pos.y() > m_vSlider->bottom())
            edit->editorAction(ReEdit::EA_PAGE_DOWN);
      }
   }
   return rc;
}

