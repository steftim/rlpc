#include "search.hpp"
#include "./ui_search.h"
#include <QDialog>
//extern "C" {
//#include "yandexmusic.h"
//}
#include <string.h>

search::search(QWidget *parent) : QDialog(parent), ui(new Ui::search){
    ui->setupUi(this);
}

search::~search(){
    delete ui;
}

void search::on_search_button_clicked(){
    /*
     *   *** fuche ***
     *
     *     tracks rdata;
     *   yam_search((char*)ui->search_prompt->text().toStdString().c_str(), rdata);
     *
     */
}
