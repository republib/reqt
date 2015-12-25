/*
 * ReMatrix.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef RplMatrix_HPP_
#define RplMatrix_HPP_

class RplMatrix;
/**
 * Implements a RplMatrix specific exception.
 */
class RplMatrixException: public ReException {
public:
   RplMatrixException(const RplMatrix& RplMatrix, const char* format, ...);
};

/**
 * The type of a RplMatrix element.
 */
typedef qreal MatVal;

class Tuple2 {
public:
   Tuple2(MatVal value1, MatVal value2) :
      m_value1(value1),
      m_value2(value2) {
   }
public:
   MatVal m_value1;
   MatVal m_value2;
};
/**
 * Implements a RplMatrix with 2 dimensions.
 */
class RplMatrix {
public:
   RplMatrix(const char* name = NULL);
   RplMatrix(int rows, int cols, const char* name = NULL);
   virtual ~RplMatrix();
   RplMatrix(const RplMatrix& source);
   RplMatrix& operator =(const RplMatrix& source);
public:
   RplMatrix& operator +=(const RplMatrix& operand);
   RplMatrix& operator -=(const RplMatrix& operand);
   RplMatrix operator +(const RplMatrix& operand);
   RplMatrix operator -(const RplMatrix& operand);
   RplMatrix& operator +=(MatVal scalar);
   RplMatrix& operator -=(MatVal scalar);
   RplMatrix operator +(MatVal scalar);
   RplMatrix operator -(MatVal scalar);
   bool operator ==(const RplMatrix& operand) const;
   bool operator ==(MatVal scalar) const;
   inline bool operator !=(const RplMatrix& operand) const {
      return !(*this == operand);
   }
   inline bool operator !=(MatVal operand) {
      return !(*this == operand);
   }
public:
   inline const QByteArray& getName() const {
      return m_name;
   }
   inline MatVal get(int row, int col) const {
      check(row, col);
      return m_values[row * m_cols + col];
   }
   inline RplMatrix& set(int row, int col, MatVal value) {
      check(row, col);
      m_values[row * m_cols + col] = value;
      return *this;
   }
   inline int getRows() const {
      return m_rows;
   }
   inline int getCols() const {
      return m_cols;
   }
public:
   void checkDefinition(int rows, int cols) const;
   void check(int row, int col) const;
   void checkSameDimension(const RplMatrix& operand) const;
   RplMatrix& resize(int rows, int cols, const MatVal values[] = NULL,
                     MatVal defaultValue = 0.0);
   Tuple2 minMax() const;
   RplMatrix transpose() const;
   QByteArray toString(const char* prefix = NULL, const char* format = "%f",
                       const char* rowSeparator = "\n", const char* colSeparator = ",") const;
   void readFromCvs(const char* filename, int maxLineLength = 1024 * 1024);
   void readFromXml(const char* filename, const char* tagCol,
                    const char* tagRow, const char* tagTable,
                    int maxLineLength = 1024 * 1024);
protected:
   int m_rows;
   int m_cols;
   MatVal* m_values;
   QByteArray m_name;
};

#endif /* RplMatrix_HPP_ */
