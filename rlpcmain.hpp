#ifndef RLPCMAIN_HPP
#define RLPCMAIN_HPP

#include <QMainWindow>
#include <QMediaPlayer>
#include <QStandardItem>

QT_BEGIN_NAMESPACE
namespace Ui { class rlpcMain; }
QT_END_NAMESPACE

class rlpcMain : public QMainWindow
{
  Q_OBJECT

public:
  rlpcMain(QWidget *parent = nullptr);
  ~rlpcMain();

  QString timeToString(qint64);

  QString fileName;
  QMediaPlayer* player;
  QMediaPlaylist* playlist;
  QStandardItemModel* playlist_IModel;

private slots:
  void on_OpenFile_clicked();
  void on_Play_clicked();

  void SetDuration(qint64);
  //void StatusChanged(QMediaPlayer::State);
  //void playlistPositionChanged(int)
  void setTrackPos(int);
  void changeTrackPos(qint64);

private:
  Ui::rlpcMain *ui;
};
#endif // RLPCMAIN_HPP
