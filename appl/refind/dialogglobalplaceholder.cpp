/*
 * dialogglobalplaceholder.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "dialogglobalplaceholder.hpp"
#include "ui_dialogglobalplaceholder.h"

DialogGlobalPlaceholder::DialogGlobalPlaceholder(QWidget* parent) :
   QDialog(parent), ui(new Ui::DialogGlobalPlaceholder), m_var() {
   ui->setupUi(this);
   connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
}

DialogGlobalPlaceholder::~DialogGlobalPlaceholder() {
   delete ui;
}

/**
 * Handles the push of the OK button.
 */
void DialogGlobalPlaceholder::ok() {
   int row = ui->tableWidget->currentRow();
   if (row >= 0)
      m_var = ui->tableWidget->item(row, COL_VAR)->text();
   close();
}

/**
 * Returns the selected placeholder.
 *
 * @return  "": nothing selected<br>
 *          otherwise: the name of the selected placeholder
 */
QString DialogGlobalPlaceholder::var() const {
   return m_var;
}

