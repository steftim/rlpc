#pragma once

#include <QMainWindow>
#include <QMediaPlayer>
#include <QStandardItem>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStandardItem>
#include <QListWidget>
#include <QTableView>
#include <QVariant>
#include <QTime>
#include <QSvgWidget>
#include <QMediaMetaData>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <taglib/tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>
#include "yandexmusic.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class rlpcMain;
}
QT_END_NAMESPACE

class rlpcMain : public QMainWindow
{
  Q_OBJECT

public:
  rlpcMain(QWidget *parent = nullptr);
  ~rlpcMain();

private slots:
  void on_OpenFile_clicked(void);
  void on_Play_clicked(void);
  void on_Previous_clicked(void);
  void on_Next_clicked(void);
  void SetDuration(qint64);
  void StatusChanged(QMediaPlayer::State);
  void setTrackPos(int);
  void changeTrackPos(qint64);
  void changeTheme(QString);
  void trackTags(void);
  void playlistUpdate(void);
  void on_playlistView_clicked(const QModelIndex &index);
  void on_search_butt_clicked();
  void on_PlaylistSearch_doubleClicked(const QModelIndex &index);
  void on_search_line_returnPressed();
  void on_playstate_clicked();
  void on_login_button_clicked();
  void on_usrnm_line_returnPressed();
  void on_pass_line_returnPressed();
  void on_SaveSettings_clicked();

private:
  Ui::rlpcMain *ui;

  /* change this variable if you want to specify your path to the "res" directory */
  QString res_path = "";

  QString timeToString(qint64);

  enum Playstate{
      REPEAT_ALL,
      REPEAT_ONE,
      ALL_ONCE
  };

  struct settings{
      QString theme = "white";
      qint8 State;
      QString proxy;
      QString proxy_type;
  }settings;

  QString fileName;
  QMediaPlayer* player;
  QMediaPlaylist* playlist;
  QStandardItemModel* playlist_IModel;
  QStandardItemModel* playlistSearch_IModel;
  QGraphicsScene* coverScene = new QGraphicsScene();
  tracks* tracks_struct;
  /* fill in the table after clicking the "Search" button */
  void table_fill(tracks*);
  /* check config file */
  FILE* openconf(FILE*);
  void chkconf(void);
  void addToPlaylist(const QList<QUrl>);
  void stylesheetload(void);
  void enablePlayButt(void);
  void chstbtt(void);
  void connection_error(void);
  void proxytype(void);
  void errorbox(QString);
  userInfo* userinfo;
  unsigned int* playlist_id;
  track* currentTrackInfo;
  QString white_theme;
  QString black_theme;
  QMessageBox ErrorBox;
  QStringList configpath;
};
