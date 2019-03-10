#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <QFileInfo>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>

pixelType showBuf[1440][2560];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->geometry().width(),this->geometry().height());
    ui->verticalScrollBar->setEnabled(false);
}

MainWindow::~MainWindow() {
    delete ui;
    //if (photon!=nullptr) delete photon;
}

void MainWindow::on_actionOpen_triggered() {
    inFile = QFileDialog::getOpenFileName(this, "Select file to decrease Pixels from",
                                          QDir::homePath(),
                                          "Photon Files (*.photon)");

    if (inFile=="") return;

    setWindowTitle("decreasePixel: " + inFile);
    //photon.reset(new PhotonFile(fp));       // This is safer; Prevents memory leaks.

    photon = std::make_unique<PhotonFile>(inFile.toStdString());


    QString tempStr;
    QTextStream outStr(&tempStr);
    outStr << "=============" << endl << "=============" << endl;

    outStr << "File: " << endl << inFile.section("/",-1,-1) << endl;
    outStr << "loaded at: " << QTime::currentTime().toString() << endl << endl;

    outStr << "Bed Dimensions, X       : " << photon->getHeader().bedX << endl;
    outStr << "Bed Dimensions, Y       : " << photon->getHeader().bedY << endl;
    outStr << "Bed Dimensions, Z       : " << photon->getHeader().bedZ << endl;
    outStr << "Layer Height            : " << photon->getHeader().layerHeight << endl;
    outStr << "Exposure Time           : " << photon->getHeader().expTime << endl;
    outStr << "Bottom Exposure Time    : " << photon->getHeader().expBottom << endl;
    outStr << "Off Time                : " << photon->getHeader().offTime << endl;
    outStr << "#Bottom Layers          : " << photon->getHeader().bottomLayers << endl;
    outStr << "Total #Layers           : " << photon->getHeader().nrLayers << endl;
    outStr << endl;

    //std::cout << *photon.get();   // get() is a member of unique pointer and returns a pointer to the contained object.

    ui->txtInfo->appendPlainText(tempStr);
    ui->lblNrLayers->setText(QString::number(photon->getHeader().nrLayers) + " Layers");

    ui->verticalScrollBar->setMaximum(photon->getHeader().nrLayers-1);
    ui->verticalScrollBar->setEnabled(true);
    photon->decodeImageFromRaw(0, showBuf);
    ui->txtCurrentLayer->setText("0");

    ui->widget->repaint();

    //fclose(fp);
}

void MainWindow::on_actionSave_triggered() {

    if (photon==nullptr) {
        QMessageBox::information(this, "Cannot do this now", "You need to load a file first.", QMessageBox::Ok);
        return;
    }
/*
    outFile = QFileDialog::getSaveFileName(this, tr("Select file to Save to"),
                                          QDir::homePath(),
                                          tr("Photon Files (*.photon)"),nullptr,QFileDialog::DontUseNativeDialog);
*/
    outFile = QFileDialog::getSaveFileName(this, tr("Select file to Save to"),
                                          QDir::homePath(),
                                          tr("Photon Files (*.photon)"));

    if (outFile=="") return;

    FILE *wp=fopen(outFile.toStdString().c_str(),"wb");
    photon->writeFile(outFile.toStdString());
    fclose(wp);

    QString tempStr;
    QTextStream outStr(&tempStr);
    outStr << "File: " << endl << outFile.section("/",-1,-1) << endl;
    outStr << "saved at: " << QTime::currentTime().toString() << endl << endl;
    ui->txtInfo->appendPlainText(tempStr);
}

void MainWindow::on_verticalScrollBar_valueChanged(int value)
{
    photon->decodeImageFromRaw(static_cast<unsigned long>(value), showBuf);
    ui->txtCurrentLayer->setText(QString::number(value));
    ui->widget->repaint();
}

void MainWindow::on_txtCurrentLayer_textChanged(const QString &arg1)
{
    if (arg1.toInt()<photon->getHeader().nrLayers-1) {
        photon->decodeImageFromRaw(arg1.toULong(), showBuf);
        ui->verticalScrollBar->setValue(arg1.toInt());
    } else {
        photon->decodeImageFromRaw(static_cast<unsigned long>(photon->getHeader().nrLayers-1), showBuf);
        ui->verticalScrollBar->setValue(photon->getHeader().nrLayers-1);
    }
}

void MainWindow::on_btnDecrease_clicked() {

    if (photon->getHeader().nrLayers==0) {
        QMessageBox::information(this, "Cannot do this now", "You need to load a file first.", QMessageBox::Ok);
        return;
    }

    QProgressBar *progressBar = new QProgressBar;
    progressBar->setRange(0,photon->getHeader().nrLayers-1);
    QLabel *label= new QLabel;
    label->setText("Trimming Pixels: ");
    ui->statusBar->addPermanentWidget(label);
    ui->statusBar->addPermanentWidget(progressBar);

    progressBar->setAlignment(Qt::AlignRight);

    ui->txtInfo->appendPlainText("Starting decreasing at: " + QTime::currentTime().toString());

    for (int i=0; i<photon->getHeader().nrLayers; i++) {
        progressBar->setValue(static_cast<int>(i));
        photon->decreasePixel(static_cast<unsigned long>(i));
        QCoreApplication::processEvents();              // This prevents the loop from blocking the GUI.
    }

    ui->statusBar->removeWidget(progressBar);
    ui->statusBar->removeWidget(label);
    delete progressBar;
    delete label;
    photon->decodeImageFromRaw(ui->txtCurrentLayer->text().toULong(), showBuf);
    ui->widget->repaint();
    ui->txtInfo->appendPlainText("Finished at: " + QTime::currentTime().toString());
    ui->txtInfo->appendPlainText("");

}
