#include "rlpcmain.hpp"
#include "./ui_rlpcmain.h"
#include "search.hpp"

#include <QFileDialog>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStandardItem>
#include <QListWidget>
#include <QTableView>
#include <QTime>
#include <QSvgWidget>
#include <QMediaMetaData>
#include <taglib/tag.h>
#include <taglib/fileref.h>

rlpcMain::rlpcMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::rlpcMain){
      ui->setupUi(this);

      //data model for player playlist
      playlist_IModel = new QStandardItemModel(this);
      ui->playlistView->setModel(playlist_IModel);

      ui->playlistView->horizontalHeader()->setStretchLastSection(true);

      //Player elements
      player = new QMediaPlayer(this);
      playlist = new QMediaPlaylist(player);
      player->setPlaylist(playlist);
      playlist->setCurrentIndex(0);

      ui->theme->addItem("white");
      ui->theme->addItem("black");

      //Events
      connect(player, SIGNAL(durationChanged(qint64)), SLOT(SetDuration(qint64)));
      connect(ui->timeslider, SIGNAL(sliderMoved(int)), SLOT(setTrackPos(int)));
      connect(player, SIGNAL(positionChanged(qint64)), SLOT(changeTrackPos(qint64)));
      connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(StatusChanged(QMediaPlayer::State)));
      connect(playlist, SIGNAL(currentMediaChanged(const QMediaContent)), SLOT(playlistUpdate(void)));
      //theme changer
      connect(ui->theme, SIGNAL(currentTextChanged(const QString)), SLOT(changeTheme(QString)));
      connect(playlist, SIGNAL(currentMediaChanged(const QMediaContent)), SLOT(trackTags(void)));
      changeTheme("white");

      extern search search_w;
      ui->replay->setCheckable(true);
}

rlpcMain::~rlpcMain(){
  delete playlist;
  delete player;
  delete ui;
}

void rlpcMain::on_OpenFile_clicked(void){
  QStringList files = QFileDialog::getOpenFileNames(this,
      tr("Open File"), "", tr("Audio Files (*.mp3 *.flac *.m4a)"));
  //If file path not empty, add file to playlist.
  if(!files.isEmpty()){
     foreach (QString filePath, files) {
            QList<QStandardItem *> items;
            TagLib::FileRef track(QString(filePath).toStdString().c_str());
            items.append(new QStandardItem(TagLib::String(track.tag()->title()).toCString()));
            playlist_IModel->appendRow(items);
            playlist->addMedia(QUrl::fromLocalFile(filePath));
        }
      //Enable buttons. If it is enabled when playlist is empty, player will crash.
      ui->Previous->setEnabled(true);
      ui->Play->setEnabled(true);
      ui->Next->setEnabled(true);
    }
}

void rlpcMain::on_Play_clicked(void){
    player->setVolume(50); //delete this line. (feature)
    if(player->state() != QMediaPlayer::PlayingState){
        player->play();
      }else{
        player->pause();
      }
}

QString rlpcMain::timeToString(qint64 time){
  int hrs = (time / (60 * 60 * 1000));
  int min = ((time % (60 * 60 * 1000)) / (60 * 1000));
  int sec = ((time % (60 * 1000)) / 1000);

  if(hrs == 0)
    return QTime(hrs, min, sec).toString("mm:ss");
  else
    return QTime(hrs, min, sec).toString("hh:mm:ss");
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

void rlpcMain::on_Previous_clicked(void){
             playlist->setCurrentIndex(playlist->previousIndex());
}

void rlpcMain::on_Next_clicked(void){
    playlist->setCurrentIndex(playlist->nextIndex());
}

void rlpcMain::StatusChanged(QMediaPlayer::State state){
    if(state == QMediaPlayer::PlayingState){
        ui->Play->setIcon(QIcon(icon_path + "res/pause_" + ui->theme->currentText() + ".svg"));
    }else{
        ui->Play->setIcon(QIcon(icon_path + "res/play_" + ui->theme->currentText() + ".svg"));
    }
}

void rlpcMain::changeTheme(QString theme){
    if(theme == "white"){
        StatusChanged(player->state());
        ui->Next->setIcon(QIcon(icon_path + "res/next_white.svg"));
        ui->Previous->setIcon(QIcon(icon_path + "res/prev_white.svg"));
        ui->main->setStyleSheet("background-color: #eff0f1;");
        ui->playlistView->setStyleSheet("color: black;");
        ui->trackName->setStyleSheet("color: black;");
        ui->trackAuthor->setStyleSheet("color: black;");
        ui->OpenFile->setStyleSheet("color: black;");
        ui->time->setStyleSheet("color: black;");
        ui->duration->setStyleSheet("color: black;");
        ui->theme->setStyleSheet("color: black;");
        ui->replay->setStyleSheet("color: black;");
        ui->search->setStyleSheet("color: black;");
    }else if(theme == "black"){
        StatusChanged(player->state());
        ui->Next->setIcon(QIcon(icon_path + "res/next_black.svg"));
        ui->Previous->setIcon(QIcon(icon_path + "res/prev_black.svg"));
        ui->main->setStyleSheet("background-color: #31363b;");
        ui->playlistView->setStyleSheet("color: white;");
        ui->trackName->setStyleSheet("color: white;");
        ui->trackAuthor->setStyleSheet("color: white;");
        ui->OpenFile->setStyleSheet("color: white;");
        ui->time->setStyleSheet("color: white;");
        ui->duration->setStyleSheet("color: white;");
        ui->theme->setStyleSheet("color: white;");
        ui->replay->setStyleSheet("color: white;");
        ui->search->setStyleSheet("color: white;");
    }
}

void rlpcMain::trackTags(void){
    if(!player->currentMedia().request().url().isEmpty()){
        TagLib::FileRef currentTrack(player->currentMedia().request().url().toString().remove(0,7).toStdString().c_str());          //ui->trackName->setText(TagLib::String(currentTrack.tag()->title().toCString()));
        ui->trackAuthor->setText(currentTrack.tag()->artist().toCString());
        ui->trackName->setText(currentTrack.tag()->title().toCString());
    }
}

void rlpcMain::playlistUpdate(void){
    ui->playlistView->selectRow(playlist->currentIndex());
    trackTags();
}

void rlpcMain::on_playlistView_clicked(const QModelIndex &index){
    playlist->setCurrentIndex(index.row());
    trackTags();
}


/*
 *   Not working now. Bruh...
 */

void rlpcMain::on_search_clicked(){

/*
 *    search search_w;
 *    search_w.exec();
 */

}

void rlpcMain::on_replay_toggled(bool checked){
    if(checked == true){
        playlist->setPlaybackMode(playlist->CurrentItemInLoop);
    }else{
        playlist->setPlaybackMode(playlist->Sequential);
    }
}
