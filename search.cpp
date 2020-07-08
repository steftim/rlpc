#include "search.hpp"
#include "./ui_search.h"
#include <QDialog>
#include <QDebug>
extern "C" {
#include <yandex_api.h>
}
#include <curl/curl.h>
#include <string.h>

search::search(QWidget *parent) : QDialog(parent), ui(new Ui::search){
    ui->setupUi(this);
}

search::~search(){
    delete ui;
}

void search::on_search_button_clicked(){
    yandex_search((char*)ui->search_prompt->text().toStdString().c_str());
}
