#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
    class search;
}
QT_END_NAMESPACE

class search : public QDialog
{
    Q_OBJECT
public:
    explicit search(QWidget *parent = nullptr);
    ~search();

signals:

private slots:
    void on_search_button_clicked();

private:
    Ui::search *ui;
};
