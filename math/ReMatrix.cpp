/*
 * ReMatrix.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Implements 2 dimensional matrices.
 */
/** @file math/ReMatrix.hpp
 *
 * @brief Definitions for 2 dimensional matrices.
 */

#include "base/rebase.hpp"
#include "math/remath.hpp"

RplMatrixException::RplMatrixException(const RplMatrix& RplMatrix,
                                       const char* format, ...) :
   ReException() {
   if (!RplMatrix.getName().isEmpty())
      m_message = RplMatrix.getName() + ": ";
   char buffer[16 * 1024];
   va_list args;
   va_start(args, format);
   qvsnprintf(buffer, sizeof buffer, format, args);
   va_end(args);
   m_message += buffer;
}

/**
 * Constructor.
 */
RplMatrix::RplMatrix(const char* name) :
   m_rows(0),
   m_cols(0),
   m_values(NULL),
   m_name(name) {
}

/**
 * Constructor.
 *
 * @param rows		number of rows
 * @param cols		number of columns
 * @param name      the name of the matrix
 */
RplMatrix::RplMatrix(int rows, int cols, const char* name) :
   m_rows(rows),
   m_cols(cols),
   m_values(new MatVal[rows* cols]),
   m_name(name) {
}
/**
 * Destructor.
 */
RplMatrix::~RplMatrix() {
   delete m_values;
   m_values = NULL;
}

/**
 * Copy constructor.
 *
 * @param source	source to copy
 */
RplMatrix::RplMatrix(const RplMatrix& source) :
   m_rows(0),
   m_cols(0),
   m_values(NULL),
   m_name(source.m_name + QByteArray("-copy")) {
   resize(source.m_rows, source.m_cols, source.m_values);
}

/**
 * Checks the validity of the definition parameters.
 *
 * @param rows		the row number
 * @param cols		the column number
 * @throws RplMatrixException
 */
void RplMatrix::checkDefinition(int rows, int cols) const {
   if (rows < 0)
      throw RplMatrixException(*this, "row number negative: %d", rows);
   if (cols < 0)
      throw RplMatrixException(*this, "column number negative: %d", cols);
   if (double(rows) * cols > 1.0 * 1000 * 1000)
      throw RplMatrixException(*this, "too many elements: %d*%d", rows, cols);
}

/**
 * Checks the validity of the indexes.
 *
 * @param row	the RplMatrix row number: 0..N-1
 * @param col	the RplMatrix column number: 0..M-1
 * @throws RplMatrixException
 */
void RplMatrix::check(int row, int col) const {
   if (row < 0 || row >= m_rows)
      throw RplMatrixException(*this, "invalid row: %d not in [0,%d[", row,
                               m_rows);
   if (col < 0 || col >= m_cols)
      throw RplMatrixException(*this, "invalid column: %d not in [0,%d[", col,
                               m_cols);
}
/**
 * Checks whether a given Matrix has the same dimensions.
 *
 * @param operand	Matrix to compare
 * @throws RplMatrixException
 */
void RplMatrix::checkSameDimension(const RplMatrix& operand) const {
   if (m_rows != operand.getRows())
      throw RplMatrixException(*this, "%s has a different row count: %d / %d",
                               operand.getName().constData(), m_rows, operand.getRows());
   if (m_cols != operand.getCols())
      throw RplMatrixException(*this,
                               "%s has a different column count: %d / %d",
                               operand.getName().constData(), m_cols, operand.getCols());
}

/**
 * Assignment operator.
 *
 * @param source	the source to copy
 */
RplMatrix& RplMatrix::operator =(const RplMatrix& source) {
   resize(source.m_rows, source.m_cols, source.m_values);
   return *this;
}
/**
 * Adds a Matrix to the instance.
 *
 * @param operand	Matrix to add
 * @return			the instance itself
 */
RplMatrix& RplMatrix::operator +=(const RplMatrix& operand) {
   checkSameDimension(operand);
   for (int ix = m_rows * m_cols - 1; ix >= 0; ix--) {
      m_values[ix] += operand.m_values[ix];
   }
   return *this;
}
/**
 * Subtracts a matrix from the instance.
 *
 * @param operand	matrix to subtract
 * @return			the instance itself
 */
RplMatrix& RplMatrix::operator -=(const RplMatrix& operand) {
   checkSameDimension(operand);
   for (int ix = m_rows * m_cols - 1; ix >= 0; ix--) {
      m_values[ix] -= operand.m_values[ix];
   }
   return *this;
}
/**
 * Builds the sum of the instance and a given matrix.
 *
 * @param operand	RplMatrix to add
 * @return			a new RplMatrix with the sum
 */
RplMatrix RplMatrix::operator +(const RplMatrix& operand) {
   RplMatrix rc(*this);
   rc += operand;
   return rc;
}
/**
 * Builds the difference of the instance and a given matrix.
 *
 * @param operand	matrix to subtract
 * @return			a new matrix with the difference
 */
RplMatrix RplMatrix::operator -(const RplMatrix& operand) {
   RplMatrix rc(*this);
   rc -= operand;
   return rc;
}
/**
 * Adds a scalar to the instance.
 *
 * @param scalar	scalar to add
 * @return			the instance itself
 */
RplMatrix& RplMatrix::operator +=(MatVal scalar) {
   for (int ix = m_rows * m_cols - 1; ix >= 0; ix--) {
      m_values[ix] += scalar;
   }
   return *this;
}
/**
 * Adds a scalar to the instance.
 *
 * @param scalar	scalar to add
 * @return			the instance itself
 */
RplMatrix& RplMatrix::operator -=(MatVal scalar) {
   for (int ix = m_rows * m_cols - 1; ix >= 0; ix--) {
      m_values[ix] -= scalar;
   }
   return *this;
}
/**
 * Builds the sum of the instance and a given scalar.
 *
 * @param scalar	scalar to add
 * @return			a new matrix with the sum
 */
RplMatrix RplMatrix::operator +(MatVal scalar) {
   RplMatrix rc(*this);
   rc += scalar;
   return rc;
}
/**
 * Builds the difference of the instance and a given scalar.
 *
 * @param scalar	scalar to subtract
 * @return			a new matrix with the sum
 */
RplMatrix RplMatrix::operator -(MatVal scalar) {
   RplMatrix rc(*this);
   rc -= scalar;
   return rc;
}
/**
 * Tests the equiness of the instance with a given matrix.
 *
 * @param operand	the matrix to compare
 * @return			true: the matrices are equal<br>
 * 					false: otherwise
 */
bool RplMatrix::operator ==(const RplMatrix& operand) const {
   checkSameDimension(operand);
   bool rc = true;
   for (int ix = m_rows * m_cols - 1; ix >= 0; ix--) {
      if (m_values[ix] != operand.m_values[ix]) {
         rc = false;
         break;
      }
   }
   return rc;
}
/**
 * Compares the instance with a given scalar.
 *
 * @param scalar	the scalar to compare
 * @return			true: all elements are equal to the scalar<br>
 * 					false: otherwise
 */
bool RplMatrix::operator ==(MatVal scalar) const {
   bool rc = true;
   for (int ix = m_rows * m_cols - 1; ix >= 0; ix--) {
      if (m_values[ix] != scalar) {
         rc = false;
         break;
      }
   }
   return rc;
}
/**
 * Sets a new row-column pair.
 */
RplMatrix& RplMatrix::resize(int rows, int cols, const MatVal values[],
                             MatVal defaultValue) {
   checkDefinition(rows, cols);
   if (rows != m_rows || cols != m_cols) {
      delete m_values;
      m_values = new MatVal[rows * cols];
      m_rows = rows;
      m_cols = cols;
   }
   if (values == NULL) {
      for (int ix = rows * cols - 1; ix >= 0; ix--) {
         m_values[ix] = defaultValue;
      }
   } else {
      for (int ix = rows * cols - 1; ix >= 0; ix--) {
         m_values[ix] = values[ix];
      }
   }
   return *this;
}
/**
 * Returns the minimum and the maximum of the instance.
 *
 * @return		a tuple with the minimum and the maximum
 */
Tuple2 RplMatrix::minMax() const {
#ifndef DBL_MAX
#define DBL_MAX std::numeric_limits<qreal>::max()
#define DBL_MIN std::numeric_limits<qreal>::min()
#endif
   Tuple2 rc(DBL_MAX, DBL_MIN);
   for (int ix = m_rows * m_cols - 1; ix >= 0; ix--) {
      MatVal x;
      if ((x = m_values[ix]) < rc.m_value1)
         rc.m_value1 = x;
      if (x > rc.m_value2)
         rc.m_value2 = x;
   }
   return rc;
}

/**
 * Builds a matrix with exchanged rows and cols.
 *
 * @return	the transposed matrix
 */
RplMatrix RplMatrix::transpose() const {
   RplMatrix rc(m_cols, m_rows);
   for (int row = 0; row < m_rows; row++) {
      for (int col = 0; col < m_cols; col++) {
         rc.m_values[m_rows * col + row] = m_values[row * m_cols + col];
      }
   }
   return rc;
}
QByteArray RplMatrix::toString(const char* prefix, const char* format,
                               const char* rowSeparator, const char* colSeparator) const {
   char buffer[128];
   Tuple2 minMaxi(minMax());
   QByteArray rc;
   qsnprintf(buffer, sizeof buffer, format, minMaxi.m_value1);
   int length = strlen(buffer);
   qsnprintf(buffer, sizeof buffer, format, minMaxi.m_value2);
   int length2 = strlen(buffer);
   if (length < length2)
      length = length2;
   qsnprintf(buffer, sizeof buffer, format,
             (minMaxi.m_value1 + minMaxi.m_value2) / 2);
   length2 = strlen(buffer);
   if (length < length2)
      length = length2;
   if (prefix == NULL)
      prefix = "";
   length = m_rows * m_cols * (length + strlen(colSeparator))
            + m_rows * strlen(rowSeparator) + strlen(prefix) + 20;
   rc.reserve(length);
   rc += prefix;
   rc += "[";
   for (int row = 0; row < m_rows; row++) {
      for (int col = 0; col < m_cols; col++) {
         qsnprintf(buffer, sizeof buffer, format,
                   m_values[m_cols * row + col]);
         rc += buffer;
         rc += colSeparator;
      }
      rc += rowSeparator;
   }
   rc += "]";
   return rc;
}
/**
 * Finds the length of a column.
 *
 * @param text		the text to inspect
 * @param separator	the column separator
 * @return			the count of chars between start and the next separator
 */
static int lengthOfColumn(const char* text, char separator) {
   const char* ptr = text;
   while (*ptr == ' ')
      ptr++;
   char delimiter = 0;
   if (*ptr == '"' || *ptr == '\'') {
      delimiter = *ptr++;
   }
   while (*ptr) {
      if (*ptr == '\\') {
         ptr++;
         if (*ptr != '\0')
            ptr++;
      } else if (*ptr == separator)
         break;
      else if (*ptr != delimiter) {
         ptr++;
         while (*ptr && *ptr != separator)
            ptr++;
      }
   }
   int rc = ptr - text;
   return rc;
}
/**
 * Skips all columns with a content other than a numeric value.
 *
 * @param line		the text line
 * @param separator	the column separator
 * @return			the start of a number or ""
 */
static const char* skipNonNumbers(const char* line, char separator) {
   int len1, len2 = 0;
   while ((len1 = ReStringUtil::lengthOfNumber(line)) == 0 && (len2 =
             lengthOfColumn(line, separator)) > 0)
      line += len2;
   if (*line == separator)
      line++;
   return line;
}
/**
 * Returns the count of numeric numbers in a CSV line.
 *
 * @param line	the line from a CSV file
 * @return		0: not only numbers are in the line<br>
 * 				otherwise: the count of numeric columns in the line
 */
static int countNumbers(const char* line, char separator) {
   line = skipNonNumbers(line, separator);
   bool again = true;
   int rc = 0;
   char cc;
   while (again && (cc = *line) != '\0' && cc != '\n' && cc != '\r') {
      int length = ReStringUtil::lengthOfNumber(line, true);
      if (length == 0) {
         rc = 0;
         again = false;
      } else {
         line += length;
         rc++;
         if (*line == separator)
            line++;
      }
   }
   return rc;
}
/**
 * Reads a file with the CSV (comma separated values) format
 * into the instance.
 */
void RplMatrix::readFromCvs(const char* filename, int maxLineLength) {
   FILE* fp = fopen(filename, "r");
   if (fp == NULL)
      throw RplMatrixException(*this, "Cannot open %s (%d)", filename, errno);
   char* buffer = new char[maxLineLength + 1];
   const char* line;
   char separator = ReStringUtil::findCsvSeparator(fp, buffer, maxLineLength);
   int rows = 0;
   int cols = 0;
   int nCols;
   // find the count of rows and columns:
   while ((line = fgets(buffer, maxLineLength, fp)) != NULL) {
      if ((nCols = countNumbers(line, separator)) > 0) {
         rows++;
         if (nCols > cols)
            cols = nCols;
      }
   }
   resize(rows, cols);
   // find the values
   fseek(fp, 0, SEEK_SET);
   int row = -1;
   while ((line = fgets(buffer, maxLineLength, fp)) != NULL) {
      int nCols;
      if ((nCols = countNumbers(line, separator)) > 0) {
         row++;
         line = skipNonNumbers(line, separator);
         int col = -1;
         int length;
         const char* ptr;
         while ((length = ReStringUtil::lengthOfNumber(line, true)) > 0) {
            col++;
            ptr = line;
            line += length;
            while (*ptr == ' ')
               ptr++;
            MatVal value = atof(ptr);
            m_values[m_cols * row + col] = value;
            if (*line == separator)
               line++;
            else
               break;
         }
      }
   }
   fclose(fp);
   delete buffer;
}
void RplMatrix::readFromXml(const char* filename, const char* tagCol,
                            const char* tagRow, const char* tagTable, int maxLineLength) {
   throw RplMatrixException(*this,
                            "readFromXml not implementes: %s %s %s %s %d", filename, tagCol, tagRow,
                            tagTable, maxLineLength);
}

