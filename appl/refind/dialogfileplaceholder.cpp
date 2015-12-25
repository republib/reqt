/*
 * dialogfileplaceholder.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "dialogfileplaceholder.hpp"
#include "ui_dialogfileplaceholder.h"

DialogFilePlaceholder::DialogFilePlaceholder(QWidget* parent) :
   QDialog(parent), ui(new Ui::DialogFilePlaceHolders) {
   ui->setupUi(this);
   connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
}

DialogFilePlaceholder::~DialogFilePlaceholder() {
   delete ui;
}

/**
 * Handles the push of the OK button.
 */
void DialogFilePlaceholder::ok() {
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
QString DialogFilePlaceholder::var() const {
   return m_var;
}

