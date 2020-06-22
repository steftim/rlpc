#include "rlpcmain.hpp"
#include "./ui_rlpcmain.h"
#include <QFileDialog>
#include <QMediaPlayer>
#include <QListWidget>
#include <QTime>
#include <iostream>

bool isPlay = false;

rlpcMain::rlpcMain(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::rlpcMain){
      ui->setupUi(this);
      player = new QMediaPlayer;
      connect(player, SIGNAL(durationChanged(qint64)), SLOT(SetDuration(qint64)));
      connect(ui->timeslider, SIGNAL(sliderMoved(int)), SLOT(setTrackPos(int)));
      connect(player, SIGNAL(positionChanged(qint64)), SLOT(changeTrackPos(qint64)));
      //connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(StatusChanged(QMediaPlayer::State)));
}

rlpcMain::~rlpcMain(){
  delete ui;
}

void rlpcMain::on_OpenFile_clicked(){
  fileName = QFileDialog::getOpenFileName(this,
      tr("mp3 files"), "", tr("mp3 files (*.mp3)"));
  if(!fileName.isEmpty()){
      ui->Playlist->addItem(fileName);
      ui->Previous->setEnabled(true);
      ui->Play->setEnabled(true);
      ui->Next->setEnabled(true);
    }
}

void rlpcMain::on_Play_clicked(){
    QUrl file = QUrl::fromLocalFile(ui->Playlist->item(0)->text());
    player->setMedia(file);
    player->setVolume(50);
    if(isPlay == false){
        player->play();
        isPlay = true;
      }else{
        player->pause();
        isPlay = false;
      }
}

QString rlpcMain::timeToString(qint64 time){
  int dHours = (time / (60 * 60 * 1000));
  int dMinutes = ((time % (60 * 60 * 1000)) / (60 * 1000));
  int dSeconds = ((time % (60 * 1000)) / 1000);

  return QTime(dHours, dMinutes, dSeconds).toString("hh:mm:ss");
}

void rlpcMain::SetDuration(qint64 duration){
    ui->timeslider->setRange(0, duration);
    ui->time->setText(timeToString(0));
    ui->duration->setText(timeToString(duration));
}

void rlpcMain::setTrackPos(int pos){
    player->setPosition(pos);
}

void rlpcMain::changeTrackPos(qint64 pos){
    ui->timeslider->setValue(pos);
    ui->time->setText(timeToString(pos));
}
