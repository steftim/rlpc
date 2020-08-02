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
#include <QTime>
#include <QSvgWidget>
#include <QMediaMetaData>
#include <taglib/tag.h>
#include <taglib/fileref.h>
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
  void on_replay_toggled(bool checked);
  void on_playlistView_clicked(const QModelIndex &index);
  void on_search_butt_clicked();
  void on_PlaylistSearch_doubleClicked(const QModelIndex &index);
  void on_search_line_returnPressed();

private:
  Ui::rlpcMain *ui;

  /* change this variable if you want to specify your path to the "res" directory */
  QString icon_path = "";

  QString timeToString(qint64);

  struct settings{
      QString theme;
  }settings;

  QString fileName;
  QMediaPlayer* player;
  QMediaPlaylist* playlist;
  QStandardItemModel* playlist_IModel;
  QStandardItemModel* playlistSearch_IModel;
  tracks* tracks_struct;
  /* fill in the table after clicking the "Search" button */
  void table_fill(tracks*);
  /* check config file */
  void chkconf(void);
  void enablePlayButt(void);
};
