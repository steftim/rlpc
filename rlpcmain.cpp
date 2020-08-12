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
      ui->trackImage->setScene(coverScene);
      ui->trackImage->show();

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

      ui->playlistView->resizeRowsToContents();
      settings.State = 2;
      ui->playstate->setCheckable(true);
      chstbtt();

      userinfo->access_token = NULL;
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
            items.append(new QStandardItem(TagLib::String(track.tag()->artist()).toCString()));
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
        ui->Next->setIcon(QIcon(icon_path + "res/next_white.svg"));
        ui->Previous->setIcon(QIcon(icon_path + "res/prev_white.svg"));
        ui->main->setStyleSheet("background-color: #eff0f1;");
        ui->trackName->setStyleSheet("color: black;");
        ui->trackAuthor->setStyleSheet("color: black;");
        ui->time->setStyleSheet("color: black;");
        ui->duration->setStyleSheet("color: black;");
        ui->playstate->setStyleSheet("color: black;");
        ui->tabs->setStyleSheet("color: black;");
        ui->OpenFile->setStyleSheet("color: black;");
        ui->search_line->setStyleSheet("color: black;");
        ui->PlaylistSearch->setStyleSheet("color: black;");
        ui->search_butt->setStyleSheet("color: black;");
        ui->playlistView->setStyleSheet("color: black;");
        ui->theme->setStyleSheet("color: black;");
/*                                  border:                 none;               \
                                    color:                  black;              \
                                    font-weight:            bold;               \
                                    padding:                5px                 \
                                  }                                             \
                                                                                \
                                  QComboBox::drop-down{                         \
                                    border:              none;                  \
                                    background-color:    rgb(87, 96, 134);      \
                                    color:               black;                 \
                                    font-weight:         bold;                  \
                                    padding:             0px;                   \
                                    margin:              3px;                   \
                                  }"); */
        ui->theme_L->setStyleSheet("color: black;");
        ui->playlistView->setStyleSheet("color: black");
        ui->usrnm_line->setStyleSheet("color: black;");
        ui->usrnm_label->setStyleSheet("color: black;");
        ui->pass_line->setStyleSheet("color: black;");
        ui->pass_label->setStyleSheet("color: black;");
        ui->login_button->setStyleSheet("color: black;");
    }else if(theme == "black"){

        ui->Next->setIcon(QIcon(icon_path + "res/next_black.svg"));
        ui->Previous->setIcon(QIcon(icon_path + "res/prev_black.svg"));
        ui->main->setStyleSheet("background-color: #31363b;");
        ui->trackName->setStyleSheet("color: white;");
        ui->trackAuthor->setStyleSheet("color: white;");
        ui->time->setStyleSheet("color: white;");
        ui->duration->setStyleSheet("color: white;");
        ui->playstate->setStyleSheet("color: white;");
        ui->tabs->setStyleSheet("color: white;");
        ui->OpenFile->setStyleSheet("color: white;");
        ui->search_line->setStyleSheet("color: white;");
        ui->PlaylistSearch->setStyleSheet("color: white;");
        ui->search_butt->setStyleSheet("color: white;");
        ui->playlistView->setStyleSheet("color: white;");
        ui->theme->setStyleSheet("color: white;");
        ui->theme_L->setStyleSheet("color: white;");
        ui->usrnm_line->setStyleSheet("color: white;");
        ui->usrnm_label->setStyleSheet("color: white;");
        ui->pass_line->setStyleSheet("color: white;");
        ui->pass_label->setStyleSheet("color: white;");
        ui->login_button->setStyleSheet("color: white;");
    }
    StatusChanged(player->state());
    chstbtt();
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
                TagLib::ID3v2::AttachedPictureFrame *coverData = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());

                QImage cover;
                cover.loadFromData((const uchar*)coverData->picture().data(), coverData->picture().size());
                QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(cover));

                coverScene->clear();
                coverScene->addItem(item);
                ui->trackImage->fitInView(coverScene->sceneRect(), Qt::KeepAspectRatio);
            }


    }else if(!player->currentMedia().request().url().isEmpty()){
        ui->trackAuthor->setText(tracks_struct->item[playlist->currentIndex()].artist[0].name);
        ui->trackName->setText(tracks_struct->item[playlist->currentIndex()].title);

        QImage coverImg;

        QString url = tracks_struct->item[playlist->currentIndex()].album[0].coverUri;
        url.replace("%%", "200x200");
        cover* coverData = get_cover((char*)url.toStdString().c_str());

        coverImg.loadFromData((uchar*)coverData->data, coverData->len + 5);
        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(coverImg));

        coverScene->clear();
        coverScene->addItem(item);
        ui->trackImage->fitInView(coverScene->sceneRect(), Qt::KeepAspectRatio);
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

void rlpcMain::on_search_butt_clicked(){
    playlistSearch_IModel->clear();
    tracks_struct = yam_search((char*)ui->search_line->text().toStdString().c_str(), userinfo);
    uint i;
    if(tracks_struct != NULL){
        for(i = 0; i < tracks_struct->tracks_col; i++){
            QList<QStandardItem *> items;
            QString tmp;
            tmp += tracks_struct->item[i].title;
            tmp += "  -  ";
            tmp += tracks_struct->item[i].artist[0].name;
            items.append(new QStandardItem(tmp));
            playlistSearch_IModel->appendRow(items);
        }
    }
}

void rlpcMain::on_PlaylistSearch_doubleClicked(const QModelIndex &index){
    char* link = get_download_url(tracks_struct->item[index.row()].id, userinfo);
    if(link != NULL)playlist->addMedia(QUrl(link));
    QString tmp;
    tmp += tracks_struct->item[index.row()].title;
    tmp += "  -  ";
    tmp += tracks_struct->item[index.row()].artist[0].name;
    if(link != NULL)playlist_IModel->appendRow(new QStandardItem(tmp));
    if(!ui->Play->isEnabled()){enablePlayButt();}
}

/* Pressing the Return button does the same as pressing the Search button */
void rlpcMain::on_search_line_returnPressed(){
    on_search_butt_clicked();
}

void rlpcMain::chstbtt(void){
    if(settings.State == REPEAT_ALL){
        playlist->setPlaybackMode(playlist->Loop);
        ui->playstate->setIcon(QIcon(icon_path + "res/loop_" + settings.theme + ".svg"));
        ui->playstate->setChecked(true);
    }else if(settings.State == REPEAT_ONE){
        playlist->setPlaybackMode(playlist->CurrentItemInLoop);
        ui->playstate->setIcon(QIcon(icon_path + "res/currentloop_" + settings.theme + ".svg"));
        ui->playstate->setChecked(true);
    }else if(settings.State == ALL_ONCE){
        playlist->setPlaybackMode(playlist->Sequential);
        ui->playstate->setIcon(QIcon(icon_path + "res/loop_" + settings.theme + ".svg"));
        ui->playstate->setChecked(false);
    }
}

void rlpcMain::on_playstate_clicked(){
    if(settings.State == 2){
        settings.State = 0;
    }else{
        settings.State++;
    }
    chstbtt();
}

void rlpcMain::on_login_button_clicked(){
    userinfo = get_token((char*)"password", (char*)ui->usrnm_line->text().toStdString().c_str(), (char*)ui->pass_line->text().toStdString().c_str());
    qDebug() << "token: " << userinfo->access_token << "\nuid: " << userinfo->uid << "\nexpires in: " << userinfo->expires_in << "\ntoken type: " << userinfo->token_type;
}
