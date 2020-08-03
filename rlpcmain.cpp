#include "rlpcmain.hpp"
#include "./ui_rlpcmain.h"
#include "yandexmusic.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>

rlpcMain::rlpcMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::rlpcMain){
      ui->setupUi(this);

      /* Data model for player playlist and search playlist */
      playlist_IModel = new QStandardItemModel(this);
      ui->playlistView->setModel(playlist_IModel);
      ui->playlistView->horizontalHeader()->setStretchLastSection(true);

      playlistSearch_IModel = new QStandardItemModel(this);
      ui->PlaylistSearch->setModel(playlistSearch_IModel);
      ui->PlaylistSearch->horizontalHeader()->setStretchLastSection(true);

      /* Player elements:
       *  - Player
       *  - Playlist
       */
      player = new QMediaPlayer(this);
      playlist = new QMediaPlaylist(player);
      player->setPlaylist(playlist);
      playlist->setCurrentIndex(0);
      ui->trackImage->setScene(coverscene);

      ui->theme->addItem("white");
      ui->theme->addItem("black");

      /*
       * Events:
       * - change timestamp
       * - change duration if slider moved
       * - change track if playlist item changed
       * - change playing state if event
       * - update playlist if playlist item changed
       * - change the appearance if the theme is changed in the settings
       * - update track info if track changed
       */
      connect(player, SIGNAL(durationChanged(qint64)), SLOT(SetDuration(qint64)));
      connect(ui->timeslider, SIGNAL(sliderMoved(int)), SLOT(setTrackPos(int)));
      connect(player, SIGNAL(positionChanged(qint64)), SLOT(changeTrackPos(qint64)));
      connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(StatusChanged(QMediaPlayer::State)));
      connect(playlist, SIGNAL(currentMediaChanged(const QMediaContent)), SLOT(playlistUpdate(void)));
      //theme changer
      connect(ui->theme, SIGNAL(currentTextChanged(const QString)), SLOT(changeTheme(QString)));
      connect(playlist, SIGNAL(currentMediaChanged(const QMediaContent)), SLOT(trackTags(void)));

      // Read config file, if file not exist - create it
      chkconf();

      ui->replay->setCheckable(true);
}

void rlpcMain::chkconf(void){
    QString home;

    if((home = getenv("HOME")) == NULL){
        home = getpwuid(getuid())->pw_dir;
    }
    struct stat info;

    home += "/.config/rlpc";

    /*
     * If the config directory exists,
     * try to read the config file,
     * if not, create a file and write the standard config to it
     */
    if(stat(home.toStdString().c_str(), &info) != 0)
        printf("cannot access %s\n", home.toStdString().c_str());
    else if(info.st_mode & S_IFDIR){
        home += "/config";
        FILE* config = fopen(home.toStdString().c_str(), "r");
        char param[10], arg[10];
        /* parameter: value */
        while(fscanf(config, "%[^:]: %[^;\n]", param, arg) != EOF){
            if(strcmp(param, "theme") == 0){
                settings.theme = arg;
                ui->theme->setCurrentIndex(ui->theme->findText(settings.theme));
                changeTheme(settings.theme);
            }
        }
      }else{
         mkdir(home.toStdString().c_str(), 0755);
         home += "/config";
         FILE* config = fopen(home.toStdString().c_str(), "w");
         /* "theme: white" */
         fprintf(config, "%s", "theme: white");
        }
}

rlpcMain::~rlpcMain(){
  delete playlist;
  delete player;
  delete playlist_IModel;
  delete playlistSearch_IModel;
  delete ui;
}

void rlpcMain::on_OpenFile_clicked(void){
  QStringList files = QFileDialog::getOpenFileNames(this,
      tr("Open File"), "", tr("Audio Files (*.mp3 *.flac *.m4a)"));
  //If file path not empty, add file to playlist.
  if(!files.isEmpty()){
     foreach (QString filePath, files) {
            QList<QStandardItem *> items;
            TagLib::MPEG::File track(QString(filePath).toStdString().c_str());
            items.append(new QStandardItem(TagLib::String(track.tag()->title()).toCString()));
            playlist_IModel->appendRow(items);
            playlist->addMedia(QUrl::fromLocalFile(filePath));
        }
      //Enable buttons. If it is enabled when playlist is empty, player will crash.
      if(!ui->Play->isEnabled()){enablePlayButt();}
    }
}

void rlpcMain::enablePlayButt(void){
    ui->Previous->setEnabled(true);
    ui->Play->setEnabled(true);
    ui->Next->setEnabled(true);
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
    settings.theme = theme;
    if(theme == "white"){
        StatusChanged(player->state());
        ui->Next->setIcon(QIcon(icon_path + "res/next_white.svg"));
        ui->Previous->setIcon(QIcon(icon_path + "res/prev_white.svg"));
        ui->main->setStyleSheet("background-color: #eff0f1;");
        ui->trackName->setStyleSheet("color: black;");
        ui->trackAuthor->setStyleSheet("color: black;");
        ui->time->setStyleSheet("color: black;");
        ui->duration->setStyleSheet("color: black;");
        ui->replay->setStyleSheet("color: black;");
        ui->tabs->setStyleSheet("color: black;");
        ui->OpenFile->setStyleSheet("color: black;");
        ui->search_line->setStyleSheet("color: black;");
        ui->PlaylistSearch->setStyleSheet("color: black;");
        ui->search_butt->setStyleSheet("color: black;");
        ui->playlistView->setStyleSheet("color: black;");
    }else if(theme == "black"){
        StatusChanged(player->state());
        ui->Next->setIcon(QIcon(icon_path + "res/next_black.svg"));
        ui->Previous->setIcon(QIcon(icon_path + "res/prev_black.svg"));
        ui->main->setStyleSheet("background-color: #31363b;");
        ui->trackName->setStyleSheet("color: white;");
        ui->trackAuthor->setStyleSheet("color: white;");
        ui->time->setStyleSheet("color: white;");
        ui->duration->setStyleSheet("color: white;");
        ui->replay->setStyleSheet("color: white;");
        ui->tabs->setStyleSheet("color: white;");
        ui->OpenFile->setStyleSheet("color: white;");
        ui->search_line->setStyleSheet("color: white;");
        ui->PlaylistSearch->setStyleSheet("color: white;");
        ui->search_butt->setStyleSheet("color: white;");
        ui->playlistView->setStyleSheet("color: white;");
    }
}

void rlpcMain::trackTags(void){
    /*
     * If the current element is a local file, read its tags,
     * if it's a URL, take data from the tracks_struct structure
     */
    if(!player->currentMedia().request().url().isEmpty() && player->currentMedia().request().url().isLocalFile()){
        TagLib::MPEG::File track(player->currentMedia().request().url().toString().remove(0,7).toStdString().c_str());
        TagLib::ID3v2::Tag* currentTrack = track.ID3v2Tag();

        ui->trackName->setText(currentTrack->title().toCString());
        ui->trackAuthor->setText(currentTrack->artist().toCString());

        TagLib::ID3v2::FrameList frameList = currentTrack->frameList("APIC");
            if (!frameList.isEmpty()){
                TagLib::ID3v2::AttachedPictureFrame *coverData = (TagLib::ID3v2::AttachedPictureFrame*)frameList.front();
                QImage cover;
                cover.loadFromData((const uchar*)coverData->picture().data(), coverData->picture().size());

                QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(cover));
                coverscene->addItem(item);

                ui->trackImage->show();
                ui->trackImage->fitInView(coverscene->sceneRect(), Qt::KeepAspectRatio);
            }
    }else if(!player->currentMedia().request().url().isEmpty()){
        ui->trackAuthor->setText(tracks_struct->item[playlist->currentIndex()].artist[0].name);
        ui->trackName->setText(tracks_struct->item[playlist->currentIndex()].title);
    }
}

void rlpcMain::playlistUpdate(void){
    ui->playlistView->selectRow(playlist->currentIndex());
    trackTags();
}

void rlpcMain::on_playlistView_clicked(const QModelIndex &index){
    playlist->setCurrentIndex(index.row());
    trackTags();
    if(player->currentMedia().request().url().isLocalFile()){

        //ui->trackImage->setBackgroundBrush()
    }
}

void rlpcMain::on_replay_toggled(bool checked){
    if(checked == true){
        playlist->setPlaybackMode(playlist->CurrentItemInLoop);
    }else{
        playlist->setPlaybackMode(playlist->Sequential);
    }
}

void rlpcMain::on_search_butt_clicked(){
    tracks_struct = NULL;
    tracks_struct = yam_search((char*)ui->search_line->text().toStdString().c_str());
    uint i;
    if(tracks_struct != NULL){
    for(i = 0; i < tracks_struct->tracks_col; i++){
        QStandardItem* title = new QStandardItem();
        QString tmp;
        tmp += tracks_struct->item[i].title;
        tmp += "  -  ";
        tmp += tracks_struct->item[i].artist[0].name;
        title->setText(tmp);
        playlistSearch_IModel->setItem(i, 0, title);

    }
    }
}

void rlpcMain::on_PlaylistSearch_doubleClicked(const QModelIndex &index){
    QString link = NULL;
    link = get_download_url(tracks_struct->item[index.row()].id);
    if(link != NULL)playlist->addMedia(QUrl(link));
    if(link != NULL)playlist_IModel->appendRow(new QStandardItem(tracks_struct->item[index.row()].title));
    if(!ui->Play->isEnabled()){enablePlayButt();}

}

/* Pressing the Return button does the same as pressing the Search button */
void rlpcMain::on_search_line_returnPressed(){
    on_search_butt_clicked();
}
