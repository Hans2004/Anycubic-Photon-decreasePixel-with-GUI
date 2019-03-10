#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <memory>
//#include <QDebug>
//#include "fileutils.h"
#include "photonfile.h"

extern pixelType showBuf[1440][2560];

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //static pixelType pixelBuf[1440][2560];

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_triggered();

    void on_verticalScrollBar_valueChanged(int value);

    void on_txtCurrentLayer_textChanged(const QString &arg1);

    void on_btnDecrease_clicked();

private:
    Ui::MainWindow *ui;
    QString inFile="", outFile="";
    unique_ptr<PhotonFile> photon;
    //PhotonFile *photon=nullptr;
    //pixelType pixelBuf[1440][2560];
};

#endif // MAINWINDOW_H
