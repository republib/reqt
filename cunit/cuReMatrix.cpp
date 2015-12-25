/*
 * cuReMatrix.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the matrices.
 */

#include "base/rebase.hpp"
#include "math/remath.hpp"

class TestRplMatrix: public ReTest {
public:
   TestRplMatrix() :
      ReTest("RplMatrix") {
      doIt();
   }

public:
   void fillMatrix(RplMatrix& mx, MatVal offset = 0) {
      for (int row = 0; row < mx.getRows(); row++) {
         for (int col = 0; col < mx.getCols(); col++) {
            mx.set(row, col, 100.0 * row + col + offset);
         }
      }
   }
   void checkMatrix(const RplMatrix& mx, MatVal offset = 0) {
      int count = 0;
      for (int row = 0; row < mx.getRows(); row++) {
         for (int col = 0; col < mx.getCols(); col++) {
            checkEqu(100.0 * row + col + offset, mx.get(row, col));
            count++;
         }
      }
      checkEqu(mx.getCols() * mx.getRows(), count);
   }
   void fillConst(RplMatrix& mx, MatVal value) {
      for (int row = 0; row < mx.getRows(); row++) {
         for (int col = 0; col < mx.getCols(); col++) {
            mx.set(row, col, value);
         }
      }
   }
   void checkConst(const RplMatrix& mx, MatVal value) {
      int count = 0;
      for (int row = 0; row < mx.getRows(); row++) {
         for (int col = 0; col < mx.getCols(); col++) {
            checkEqu(value, mx.get(row, col));
            count++;
         }
      }
      checkEqu(mx.getCols() * mx.getRows(), count);
   }

   void testBasic() {
      Tuple2 tuple(-2.0, 0.5);
      checkEqu(-2.0, tuple.m_value1);
      checkEqu(0.5, tuple.m_value2);
      RplMatrix mat("mx");
      try {
         throw RplMatrixException(mat, "String: %s and int %d", "Hi", -333);
         checkF(true);
      } catch (RplMatrixException exc) {
         checkEqu("mx: String: Hi and int -333", exc.getMessage());
      }
      RplMatrix mat2;
      try {
         throw RplMatrixException(mat2, "String: %s and int %d", "Hi", -333);
         checkF(true);
      } catch (RplMatrixException exc) {
         checkEqu("String: Hi and int -333", exc.getMessage());
      }
      checkEqu("mx", mat.getName());
      checkEqu("", mat2.getName());
      RplMatrix m2x3(2, 3, "m2x3");
      checkEqu("m2x3", m2x3.getName());
      checkEqu(2, m2x3.getRows());
      checkEqu(3, m2x3.getCols());
      fillMatrix(m2x3);
      checkMatrix(m2x3);
      RplMatrix mxCopy(m2x3);
      checkEqu("m2x3-copy", mxCopy.getName());
      checkEqu(2, mxCopy.getRows());
      checkEqu(3, mxCopy.getCols());
      checkMatrix(mxCopy);
      RplMatrix mxCopy2("mxCopy2");
      mxCopy2 = m2x3;
      checkEqu("mxCopy2", mxCopy2.getName());
      checkEqu(2, mxCopy2.getRows());
      checkEqu(3, mxCopy2.getCols());
      checkMatrix(mxCopy2);
   }
   void testAddOperators() {
      RplMatrix m1(3, 2, "m1");
      fillMatrix(m1);
      checkMatrix(m1);
      RplMatrix m2(3, 2, "m2");
      fillMatrix(m2, 42);
      checkMatrix(m2, 42);
      RplMatrix m3(3, 2, "m3");
      fillMatrix(m3, -42);
      checkMatrix(m3, -42);
      m1 += 42;
      checkMatrix(m1, 42);
      checkT(m1 == m2);
      checkF(m1 == m3);
      m1 -= 42;
      checkMatrix(m1);
      m1 -= m1;
      checkConst(m1, 0);
      fillMatrix(m1);
      m1 -= m3;
      checkConst(m1, 42);
      m1 += m2;
      checkMatrix(m1, 42 * 2);
   }
   void testCompareOperators() {
      RplMatrix m1(3, 2, "m1");
      fillMatrix(m1);
      checkMatrix(m1);
      RplMatrix m2(3, 2, "m2");
      fillMatrix(m2);
      checkT(m1 == m2);
      checkF(m1 != m2);
      // modify each element, comparism must return false:
      int row, col;
      for (row = 0; row < m2.getRows(); row++)
         for (col = 0; col < m2.getCols(); col++) {
            fillMatrix(m2);
            m2.set(row, col, -1);
            checkF(m1 == m2);
            checkT(m1 != m2);
         }
      fillConst(m1, 42);
      checkT(m1 == 42);
      checkF(m1 == 43);
      checkT(m1 != 43);
      for (row = 0; row < m1.getRows(); row++)
         for (col = 0; col < m1.getCols(); col++) {
            fillMatrix(m1, 42);
            m1.set(row, col, -1);
            checkF(m1 == 42);
            checkT(m1 != 42);
         }
   }

   void testCheckDefinition() {
      RplMatrix m1(3, 2, "m1");
      fillMatrix(m1);
      checkMatrix(m1);
      RplMatrix m2(3, 2, "m2");
      fillMatrix(m2);
      m1.checkDefinition(1, 1);
      m1.checkDefinition(1000, 1000);
      m1.checkDefinition(0, 0);
      try {
         m1.checkDefinition(-1, 1);
         checkT(false);
      } catch (RplMatrixException exc) {
         checkEqu("m1: row number negative: -1", exc.getMessage());
      }
      try {
         m1.checkDefinition(1, -1);
         checkT(false);
      } catch (RplMatrixException exc) {
         checkEqu("m1: column number negative: -1", exc.getMessage());
      }
   }
   void testCheck() {
      RplMatrix m1(3, 2, "m1");
      fillMatrix(m1);
      checkMatrix(m1);
      m1.check(0, 0);
      m1.check(3 - 1, 2 - 1);
      try {
         m1.check(-1, 1);
         checkT(false);
      } catch (RplMatrixException exc) {
         checkEqu("m1: invalid row: -1 not in [0,3[", exc.getMessage());
      }
      try {
         m1.check(3, 1);
         checkT(false);
      } catch (RplMatrixException exc) {
         checkEqu("m1: invalid row: 3 not in [0,3[", exc.getMessage());
      }
      try {
         m1.check(1, -1);
         checkT(false);
      } catch (RplMatrixException exc) {
         checkEqu("m1: invalid column: -1 not in [0,2[", exc.getMessage());
      }
      try {
         m1.check(1, 2);
         checkT(false);
      } catch (RplMatrixException exc) {
         checkEqu("m1: invalid column: 2 not in [0,2[", exc.getMessage());
      }
   }
   void testCheckSameDimension() {
      RplMatrix m1(3, 2, "m1");
      RplMatrix m2(3, 2, "m2");
      m1.checkSameDimension(m2);
      m2.resize(2, 2);
      try {
         m1.checkSameDimension(m2);
         checkT(false);
      } catch (RplMatrixException exc) {
         checkEqu("m1: m2 has a different row count: 3 / 2",
                  exc.getMessage());
      }
      m2.resize(3, 3);
      try {
         m1.checkSameDimension(m2);
         checkT(false);
      } catch (RplMatrixException exc) {
         checkEqu("m1: m2 has a different column count: 2 / 3",
                  exc.getMessage());
      }
   }
   void testResize() {
      RplMatrix m1(3, 2, "m1");
      fillMatrix(m1);
      checkMatrix(m1);
      RplMatrix m2(2, 4, "m2");
      fillConst(m2, 0);
      checkConst(m2, 0);
      m1.resize(2, 4);
      checkEqu(2, m1.getRows());
      checkEqu(4, m1.getCols());
      checkT(m1 == m2);
   }

   void testMinMax() {
      RplMatrix m1(4, 5, "m1");
      fillMatrix(m1);
      checkMatrix(m1);
      m1.set(0, 0, -98);
      m1.set(3, 4, 9999);
      Tuple2 miniMax = m1.minMax();
      checkEqu(-98.0, miniMax.m_value1);
      checkEqu(9999.0, miniMax.m_value2);
      fillMatrix(m1);
      checkMatrix(m1);
      m1.set(1, 1, 7777);
      m1.set(3, 4, -987);
      miniMax = m1.minMax();
      checkEqu(-987.0, miniMax.m_value1);
      checkEqu(7777.0, miniMax.m_value2);
   }

   void testTranspose() {
      RplMatrix m1(1, 5, "m1");
      fillMatrix(m1);
      RplMatrix m2(m1.transpose());
      checkEqu(5, m2.getRows());
      checkEqu(1, m2.getCols());
      int row, col;
      col = 0;
      for (row = 0; row < 5; row++) {
         checkEqu(qreal(col * 100 + row), m2.get(row, 0));
      }
      m1.resize(35, 73);
      fillMatrix(m1);
      m2 = m1.transpose();
      checkEqu(73, m2.getRows());
      checkEqu(35, m2.getCols());
      int count = 0;
      for (row = 0; row < m2.getRows(); row++) {
         for (col = 0; col < m2.getCols(); col++) {
            checkEqu(qreal(col * 100 + row), m2.get(row, col));
            count++;
         }
      }
      checkEqu(73 * 35, count);
   }
   void testToString() {
      RplMatrix m1(1, 1, "m1");
      m1.set(0, 0, 2.34);
      checkEqu("[2.340000,\n]", m1.toString().constData());
      checkEqu("jonny[2.34000 |]",
               m1.toString("jonny", "%.5f", "|", " ").constData());
      m1.resize(2, 1);
      m1.set(0, 0, 2.34);
      m1.set(1, 0, 55.5);
      checkEqu("[2.340000,\n55.500000,\n]", m1.toString().constData());
      checkEqu("jonny[2.34000 |55.50000 |]",
               m1.toString("jonny", "%.5f", "|", " ").constData());
      log("");
   }
   void testReadCsv() {
      QByteArray fn = getTempFile("rplmatrixtest.csv");
      const char* content;
      RplMatrix m1(1, 1, "m1");
      fillMatrix(m1);
      content = ",Port0,Port1,Port2\n"
                "element1,5,  -3E-99  , 0.5\n"
                "element2,7,-22.3,44\n"
                "\n"
                "2 Elements, 3, Ports";
      ReStringUtil::write(fn, content);
      m1.readFromCvs(fn, 256);
      checkEqu(2, m1.getRows());
      checkEqu(3, m1.getCols());
      checkEqu(5.0, m1.get(0, 0));
      checkEqu(-3.0E-99, m1.get(0, 1));
      checkEqu(0.5, m1.get(0, 2));
      checkEqu(7.0, m1.get(1, 0));
      checkEqu(-22.3, m1.get(1, 1));
      checkEqu(44.0, m1.get(1, 2));
      fillMatrix(m1);
      content = "Port0,Port1,Port2\n"
                "5,  -3E-99  , 0.5\n";
      ReStringUtil::write(fn, content);
      m1.readFromCvs(fn, 256);
      checkEqu(1, m1.getRows());
      checkEqu(3, m1.getCols());
      checkEqu(5.0, m1.get(0, 0));
      checkEqu(-3.0E-99, m1.get(0, 1));
      checkEqu(0.5, m1.get(0, 2));
      /*
       void readFromCvs(const char* filename, int maxLineLength = 1024*1024);
       void readFromXml(const char* filename, const char* tagCol,
       const char* tagRow, const char* tagTable,
       int maxLineLength = 1024*1024);
       */
   }
   virtual void run(void) {
      testBasic();
      testAddOperators();
      testCompareOperators();
      testCheckDefinition();
      testCheck();
      testCheckSameDimension();
      testResize();
      testMinMax();
      testTranspose();
      testToString();
      testReadCsv();
   }
};
void testRplMatrix() {
   TestRplMatrix test;
}
