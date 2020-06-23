#include "rlpcmain.hpp"
#include "./ui_rlpcmain.h"
#include <QFileDialog>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStandardItem>
#include <QListWidget>
#include <QTableView>
#include <QTime>

bool isPlay = false;

rlpcMain::rlpcMain(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::rlpcMain){
      ui->setupUi(this);

      //data model for player playlist
      playlist_IModel = new QStandardItemModel(this);
      ui->playlistView->setModel(playlist_IModel);

      ui->playlistView->hideColumn(1);

      //Player elements
      player = new QMediaPlayer(this);
      playlist = new QMediaPlaylist(player);
      player->setPlaylist(playlist);

      //Events
      connect(player, SIGNAL(durationChanged(qint64)), SLOT(SetDuration(qint64)));
      connect(ui->timeslider, SIGNAL(sliderMoved(int)), SLOT(setTrackPos(int)));
      connect(player, SIGNAL(positionChanged(qint64)), SLOT(changeTrackPos(qint64)));
      connect(ui->playlistView, &QTableView::doubleClicked, [this](const QModelIndex &index){
              playlist->setCurrentIndex(index.row());
          });
      connect(playlist, &QMediaPlaylist::currentIndexChanged, [this](int index){
          ui->trackName->setText(playlist_IModel->data(playlist_IModel->index(index, 0)).toString());
      });
      //connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(StatusChanged(QMediaPlayer::State)));
      //connect(playlist, SIGNAL(currentIndexChanged(int)), SLOT(playlistPositionChanged(int)));

      //ui->trackImage->setBackgroundBrush(QImage(:""))

}

rlpcMain::~rlpcMain(){
  delete playlist;
  delete player;
  delete ui;
}

void rlpcMain::on_OpenFile_clicked(){
  QStringList files = QFileDialog::getOpenFileNames(this,
      tr("Audio files"), "", tr("Audio files (*.mp3)"));
  //If file path not empty, add file to playlist.
  if(!files.isEmpty()){
     foreach (QString filePath, files) {
            QList<QStandardItem *> items;
            items.append(new QStandardItem(QDir(filePath).dirName()));
            //items.append(new QStandardItem(filePath));
            playlist_IModel->appendRow(items);
            playlist->addMedia(QUrl(filePath));
        }
      //Enable buttons. If it is enabled when playlist is empty, player will crash.
      ui->Previous->setEnabled(true);
      ui->Play->setEnabled(true);
      ui->Next->setEnabled(true);
    }
}

void rlpcMain::on_Play_clicked(){
    playlist->setCurrentIndex(1);
    //delete this linez. (feature)
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
