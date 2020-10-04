#include "rlpcmain.hpp"
#include "./ui_rlpcmain.h"
#include "yandexmusic.h"

#include <unistd.h>
#include <sys/types.h>
#if defined(__linux__)
#include <pwd.h>
#include <sys/stat.h>
#elif defined(Q_OS_WIN32)
#include <QTextCodec>
#include <QStandardPaths>
#endif
//#define DEBUG

rlpcMain::rlpcMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::rlpcMain){
      ui->setupUi(this);

      /* Data model for player playlist and search playlist */
      playlist_IModel = new QStandardItemModel(this);
      playlist_IModel->setColumnCount(2);
      ui->playlistView->setModel(playlist_IModel);
      ui->playlistView->horizontalHeader()->setStretchLastSection(true);
      ui->playlistView->hideColumn(1);

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

      ui->ProxyType->addItem("http");
      ui->ProxyType->addItem("https");
      ui->ProxyType->addItem("socks4");
      ui->ProxyType->addItem("socks4a");
      ui->ProxyType->addItem("socks5");
      ui->ProxyType->addItem("socks5h");

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
      connect(playlist, SIGNAL(currentMediaChanged(const QMediaContent)), SLOT(trackTags(void)));

      // Read stylesheet files.
      stylesheetload();
      // Read config file, if file not exist - create it
      chkconf();

      ui->playlistView->resizeRowsToContents();
      settings.State = 2;
      ui->playstate->setCheckable(true);
      chstbtt();

      userinfo = (userInfo*)calloc(1, sizeof(userInfo));
      userinfo->access_token = NULL;

      playlist_id = new unsigned int;
}

void rlpcMain::chkconf(){
    QString home;
    FILE* config = NULL;
#if defined(__linux__)
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
    if(stat(home.toStdString().c_str(), &info) != 0){
        mkdir(home.toStdString().c_str(), 0755);
        home += "/config";
        config = fopen(home.toStdString().c_str(), "w");
        /* theme: white */
        fprintf(config, "%s", "");
        fclose(config);
    }else if(info.st_mode & S_IFDIR){
        home += "/config";
        config = fopen(home.toStdString().c_str(), "r");
        QString proxy_type, proxy_url;
        char param[10], arg[10];
        /* parameter: value */
        while(fscanf(config, "%[^:]: %[^;\n]", param, arg) != EOF){
            if(strcmp(param, "theme") == 0){
                settings.theme = arg;
                ui->theme->setCurrentIndex(ui->theme->findText(settings.theme));
                changeTheme(settings.theme);
            }
            if(strcmp(param, "proxy_type") == 0){
                ui->ProxyType->setCurrentIndex(ui->ProxyType->findText(arg));
            }
            if(strcmp(param, "proxy_url") == 0){
                ui->ProxyUrl_Line->setText(arg);
                proxy_url = arg;
            }
        }
        if(!proxy_type.isEmpty() && !proxy_url.isEmpty()){
            settings.proxy = proxy_type + "://" + proxy_url;
        }
        fclose(config);
    }
#elif defined(Q_OS_WIN32)
    QStringList AppData = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    home = AppData.value(1);
    QString config_path = home;
    config_path += "/config";

    /*
     * If the config directory exists,
     * try to read the config file,
     * if not, create a file and write the standard config to it
     */
    if(GetFileAttributesA(home.toStdString().c_str()) == INVALID_FILE_ATTRIBUTES){
        mkdir(home.toStdString().c_str());
        config = fopen(config_path.toStdString().c_str(), "w");
        ui->theme->setCurrentIndex(ui->theme->findText(settings.theme));
        changeTheme(settings.theme);
        fclose(config);
        return;
    }else if(GetFileAttributesA(config_path.toStdString().c_str()) != INVALID_FILE_ATTRIBUTES){
        config = fopen(config_path.toStdString().c_str(), "r");
        QString proxy_type, proxy_url;
        char param[10], arg[10];
        /* parameter: value */
        while(fscanf(config, "%[^:]: %[^;\n]", param, arg) != EOF){
            if(strcmp(param, "theme") == 0){
                settings.theme = arg;
                ui->theme->setCurrentIndex(ui->theme->findText(settings.theme));
                changeTheme(settings.theme);
            }
            if(strcmp(param, "proxy_type") == 0){
                ui->ProxyType->setCurrentIndex(ui->ProxyType->findText(arg));
            }
            if(strcmp(param, "proxy_url") == 0){
                ui->ProxyUrl_Line->setText(arg);
                proxy_url = arg;
            }
        }
        if(!proxy_type.isEmpty() && !proxy_url.isEmpty()){
            settings.proxy = proxy_type + "://" + proxy_url;
        }
    }
}
#endif
ui->theme->setCurrentIndex(ui->theme->findText(settings.theme));
changeTheme(settings.theme);
}


//#ifdef __linux__
//void rlpcMain::chkconf(void){
//    QString home;
//    if((home = getenv("HOME")) == NULL){
//        home = getpwuid(getuid())->pw_dir;
//    }
//    struct stat info;
//    home += "/.config/rlpc";

//    /*
//     * If the config directory exists,
//     * try to read the config file,
//     * if not, create a file and write the standard config to it
//     */
//    if(stat(home.toStdString().c_str(), &info) != 0){
//        mkdir(home.toStdString().c_str(), 0755);
//        home += "/config";
//        FILE* config = fopen(home.toStdString().c_str(), "w");
//        /* theme: white */
//        fprintf(config, "%s", "theme: white");
//        fclose(config);
//    }else if(info.st_mode & S_IFDIR){
//        home += "/config";
//        FILE* config = fopen(home.toStdString().c_str(), "r");
//        char param[10], arg[10];
//        /* parameter: value */
//        while(fscanf(config, "%[^:]: %[^;\n]", param, arg) != EOF){
//            if(strcmp(param, "theme") == 0){
//                settings.theme = arg;
//                ui->theme->setCurrentIndex(ui->theme->findText(settings.theme));
//                changeTheme(settings.theme);
//            }
//        }
//        fclose(config);
//    }
//}
//#endif
//#ifdef Q_OS_WIN32
//void rlpcMain::chkconf(void){
//    QString home;
//    QStringList AppData = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
//    home = AppData.value(1);
//    QString config_path = home;
//    config_path += "/config";

//    /*
//     * If the config directory exists,
//     * try to read the config file,
//     * if not, create a file and write the standard config to it
//     */
//    if(GetFileAttributesA(home.toStdString().c_str()) == INVALID_FILE_ATTRIBUTES){
//        mkdir(home.toStdString().c_str());
//        FILE* config = fopen(config_path.toStdString().c_str(), "w");
//        /* theme: white */
//        fprintf(config, "%s", "theme: white");
//        settings.theme = "white";
//        fclose(config);
//    }else if(GetFileAttributesA(config_path.toStdString().c_str()) != INVALID_FILE_ATTRIBUTES){
//        FILE* config = fopen(config_path.toStdString().c_str(), "r");
//        char param[10], arg[10];
//        /* parameter: value */
//        while(fscanf(config, "%[^:]: %[^;\n]", param, arg) != EOF){
//            if(strcmp(param, "theme") == 0){
//                settings.theme = arg;
//            }
//        }
//        fclose(config);
//    }
//    ui->theme->setCurrentIndex(ui->theme->findText(settings.theme));
//    changeTheme(settings.theme);
//}
//#endif

rlpcMain::~rlpcMain(){
  delete playlist;
  delete playlist_id;
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

            TagLib::MPEG::File track(QFile::encodeName(filePath).constData());
            QString name = TagLib::String(track.tag()->title()).toCString();
            name += "   -   ";
            name += TagLib::String(track.tag()->artist()).toCString();
            items.append(new QStandardItem(name));
            playlist_IModel->appendRow(items);
            qDebug() << "ADD FILE: " << QFile::encodeName(filePath).constData();
            playlist->addMedia(QUrl::fromLocalFile(QFile::encodeName(filePath).constData()));
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
    player->setVolume(50);
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
    /* (res_path)/res/(action)_(current_theme).svg */
    if(state == QMediaPlayer::PlayingState){
        ui->Play->setIcon(QIcon(res_path + "res/pause_" + ui->theme->currentText() + ".svg"));
    }else{
        ui->Play->setIcon(QIcon(res_path + "res/play_" + ui->theme->currentText() + ".svg"));
    }
}

void rlpcMain::changeTheme(QString theme){
    settings.theme = theme;
    if(settings.theme == "white"){
        setStyleSheet(white_theme);
        ui->Next->setIcon(QIcon(res_path + "res/next_white.svg"));
        ui->Previous->setIcon(QIcon(res_path + "res/prev_white.svg"));
    }else if(settings.theme == "black"){
        setStyleSheet(black_theme);
        ui->Next->setIcon(QIcon(res_path + "res/next_black.svg"));
        ui->Previous->setIcon(QIcon(res_path + "res/prev_black.svg"));
    }
}

void rlpcMain::stylesheetload(void){
    QFile blackstylesheet(res_path + "res/black.qss");
    QFile whitestylesheet(res_path + "res/white.qss");
    blackstylesheet.open(QFile::ReadOnly);
    whitestylesheet.open(QFile::ReadOnly);
    black_theme = blackstylesheet.readAll();
    white_theme = whitestylesheet.readAll();
#ifdef DEBUG
    qDebug() << "\n\n\n\n" << black_theme << "\n\n\n\n \
        ============================================================\
        \n\n\n\n" << white_theme << "\n\n\n\n";
#endif
}

void rlpcMain::trackTags(void){
    /*
     * If the current element is a local file, read its tags,
     * if it's a URL, take data from the tracks_struct structure
     */
    if(!player->currentMedia().request().url().isEmpty() && player->currentMedia().request().url().isLocalFile()){
        coverScene->clear();
#if defined(__linux__)
        int rem = 7;
#elif defined(Q_OS_WINDOWS)
        int rem = 8;
#endif
        TagLib::MPEG::File track(QFile::encodeName(player->currentMedia().request().url().toString().remove(0,rem)).constData());
#ifdef DEBUG
        qDebug() << player->currentMedia().request().url().toString().remove(0,rem);
#endif

        TagLib::ID3v2::Tag* currentTrack = track.ID3v2Tag();

        if(currentTrack == NULL){
            ui->trackName->setText("ERROR");
          }else{
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

          }
    }else if(!player->currentMedia().request().url().isEmpty()){
        coverScene->clear();
        track* currentTrack = get_track_info_from_id(playlist_id[playlist->currentIndex()], userinfo, (char*)settings.proxy.toStdString().c_str());
        if(currentTrack == NULL){
            return;
        }
        ui->trackAuthor->setText(currentTrack->artist[0].name);
        ui->trackName->setText(currentTrack->title);

        QImage coverImg;

        QString url = currentTrack->album[0].coverUri;
        url.replace("%%", "200x200");
        cover* coverData = get_cover((char*)url.toStdString().c_str(), (char*)settings.proxy.toStdString().c_str());

        coverImg.loadFromData((uchar*)coverData->data, coverData->len + 5);
        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(coverImg));

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
    /* If fail returns NULL */
    tracks_struct = yam_search((char*)ui->search_line->text().toStdString().c_str(), userinfo, (char*)settings.proxy.toStdString().c_str());
    if(tracks_struct == NULL){
        errorbox("Yandex music is not available, please check the connection.");
        return;
    }
    uint i;
    /* Add all items from tracks_struct to list */
    if(tracks_struct != NULL){
        for(i = 0; i < tracks_struct->tracks_col; i++){
            QList<QStandardItem *> items;
            QString tmp;
            tmp += tracks_struct->item[i].title;
            tmp += "  -  ";
            tmp += tracks_struct->item[i].artist->name;
            items.append(new QStandardItem(tmp));
            playlistSearch_IModel->appendRow(items);
        }
    }
}

void rlpcMain::on_PlaylistSearch_doubleClicked(const QModelIndex &index){
    /* Get track url and add track to playlist, then add track id to playlist_id array. */
    char* link = get_download_url(tracks_struct->item[index.row()].id, userinfo, (char*)settings.proxy.toStdString().c_str());
    if(link != NULL){
        playlist->addMedia(QUrl(link));
        QList<QStandardItem *> items;
        QString tmp;
        tmp += tracks_struct->item[index.row()].title;
        tmp += "  -  ";
        tmp += tracks_struct->item[index.row()].artist[0].name;
        items.append(new QStandardItem(tmp));
        uint indx = (uint)playlist_IModel->rowCount() + 1;
        playlist_id[indx - 1] = tracks_struct->item[index.row()].id;
        playlist_IModel->appendRow(items);
        if(!ui->Play->isEnabled()){
            enablePlayButt();
        }
    }else{
        errorbox("Yandex music is not available, please check the connection.");
        return;
    }
}

/* Pressing the Return button does the same as pressing the Search button */
void rlpcMain::on_search_line_returnPressed(){
    on_search_butt_clicked();
}

void rlpcMain::chstbtt(void){
    /*
     *  enum Playstate{
     *  REPEAT_ALL,         | 0
     *  REPEAT_ONE,         | 1
     *  ALL_ONCE,           | 2
     *  };
     *
     */

    if(settings.State == REPEAT_ALL){
        playlist->setPlaybackMode(playlist->Loop);
        ui->playstate->setIcon(QIcon(res_path + "res/loop_" + settings.theme + ".svg"));
        ui->playstate->setChecked(true);
    }else if(settings.State == REPEAT_ONE){
        playlist->setPlaybackMode(playlist->CurrentItemInLoop);
        ui->playstate->setIcon(QIcon(res_path + "res/currentloop_" + settings.theme + ".svg"));
        ui->playstate->setChecked(true);
    }else if(settings.State == ALL_ONCE){
        playlist->setPlaybackMode(playlist->Sequential);
        ui->playstate->setIcon(QIcon(res_path + "res/loop_" + settings.theme + ".svg"));
        ui->playstate->setChecked(false);
    }
}

void rlpcMain::on_playstate_clicked(){
    /* Each press of the button changes the state by 1 forward */
    if(settings.State == 2){
        settings.State = 0;
    }else{
        settings.State++;
    }
    chstbtt();
}

void rlpcMain::on_login_button_clicked(){
    userinfo = get_token((char*)"password", (char*)ui->usrnm_line->text().toStdString().c_str(), (char*)ui->pass_line->text().toStdString().c_str(), (char*)settings.proxy.toStdString().c_str());
    if(userinfo == NULL){
        errorbox("Yandex music is not available, please check the connection.");
        return;
    }
    /* struct userInfo structName = get_token(grant_type, username, password); */
    #ifdef DEBUG
    qDebug() << "token: " << userinfo->access_token << "\nuid: " << userinfo->uid << "\nexpires in: " << userinfo->expires_in << "\ntoken type: " << userinfo->token_type;
    #endif
}

void rlpcMain::on_usrnm_line_returnPressed(){
    on_login_button_clicked();
}

void rlpcMain::on_pass_line_returnPressed(){
    on_login_button_clicked();
}

void rlpcMain::errorbox(QString Text){
    ErrorBox.close();
    ErrorBox.critical(0, "Error", Text);
    ErrorBox.setFixedSize(500,200);

}

//    if(!ui->ProxyUrl_Line->text().isEmpty()){
//        settings.proxy.type = (ui->ProxyType->currentIndex() - 1);
//        settings.proxy.url = (char*)ui->ProxyUrl_Line->text().toStdString().c_str();
//    }else{
//        errorbox("Please enter proxy address.");
//    }

void rlpcMain::on_SaveSettings_clicked(){
    changeTheme(ui->theme->currentText());
    StatusChanged(player->state());
    chstbtt();
    if(!ui->ProxyUrl_Line->text().isEmpty()){
        settings.proxy = ui->ProxyType->currentText() + "://" + ui->ProxyUrl_Line->text();
    }

    QString home;
    FILE* config = NULL;
#if defined(__linux__)
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
    if(stat(home.toStdString().c_str(), &info) != 0){
        mkdir(home.toStdString().c_str(), 0755);
    }
    home += "/config";
    config = fopen(home.toStdString().c_str(), "w");
    fprintf(config,
            "%s%s%s%s%s%s",
            "theme: ",          settings.theme.toStdString().c_str(),
            "\nproxy_type: ",   ui->ProxyType->currentText().toStdString().c_str(),
            "\nproxy_url: ",    ui->ProxyUrl_Line->text().toStdString().c_str()
            );
    fclose(config);
#elif defined(Q_OS_WIN32)
    QStringList AppData = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    home = AppData.value(1);
    QString config_path = home;
    config_path += "/config";

    /*
     * If the config directory exists,
     * try to read the config file,
     * if not, create a file and write the standard config to it
     */
    if(GetFileAttributesA(home.toStdString().c_str()) == INVALID_FILE_ATTRIBUTES){
        mkdir(home.toStdString().c_str());
    }else if(GetFileAttributesA(config_path.toStdString().c_str()) != INVALID_FILE_ATTRIBUTES){
        config = fopen(config_path.toStdString().c_str(), "w");
        fprintf(config,
                "%s%s%s%s%s%s",
                "theme: ",          settings.theme.toStdString().c_str(),
                "\nproxy_type: ",   ui->ProxyType->currentText().toStdString().c_str(),
                "\nproxy_url: ",    ui->ProxyUrl_Line->text().toStdString().c_str()
                );
    }
}
#endif
}
