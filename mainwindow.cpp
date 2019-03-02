#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include <QFileInfo>
#include <QString>

pixelType pixelBuf[1440][2560];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->geometry().width(),this->geometry().height());
    ui->verticalScrollBar->setEnabled(FALSE);
    memset(&header,0, sizeof(header));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionOpen_triggered() {
    inFile = QFileDialog::getOpenFileName(this, "Select file to decrease Pixels from",
                                          QDir::homePath(),
                                          "Photon Files (*.photon)",nullptr,QFileDialog::DontUseNativeDialog);

    if (inFile=="") return;

    FILE *fp=fopen(inFile.toStdString().c_str(), "rb");
    if (fp==nullptr) {
        qDebug() << endl << "Error! File: " << inFile << " does not exist!" << endl;
        return;
    }

    setWindowTitle("decreasePixel: " + inFile);

    if (header.preview0Addr!=0) {
        free(preview1.imageData);
        free(preview2.imageData);
        rawData.clear();
        vector <vector <unsigned char> >().swap(rawData);
        vector<layerDefType>().swap(layerDefs);
    }


    getHeader(&header, fp);
    getPreview( header.preview0Addr, &preview1, fp );
    getPreview( header.preview1Addr, &preview2, fp );
    getLayerDefs( header.layerDefsAddr, header.nrLayers, &layerDefs, fp );
    getImages(&rawData, &layerDefs, header.nrLayers,fp);

    QString tempStr;
    QTextStream outStr(&tempStr);
    outStr << "=============" << endl << "=============" << endl;

    outStr << "File: " << endl << inFile.section("/",-1,-1) << endl;
    outStr << "loaded at: " << QTime::currentTime().toString() << endl << endl;

    outStr << "Bed Dimensions (X, Y, Z): " << header.bedX << ", " << header.bedY << ", " << header.bedZ << endl;
    outStr << "Layer Height            : " << header.layerHeight << endl;
    outStr << "Exposure Time           : " << header.expTime << endl;
    outStr << "Bottom Exposure Time    : " << header.expBottom << endl;
    outStr << "Off Time                : " << header.offTime << endl;
    outStr << "#Bottom Layers          : " << header.bottomLayers << endl;
    outStr << "Total #Layers           : " << header.nrLayers << endl;
    outStr << endl;

    ui->txtInfo->appendPlainText(tempStr);
    ui->lblNrLayers->setText(QString::number(header.nrLayers) + " Layers");

    ui->verticalScrollBar->setMaximum(header.nrLayers-1);
    ui->verticalScrollBar->setEnabled(TRUE);
    decodeImageFromRaw(rawData[0], pixelBuf);
    ui->txtCurrentLayer->setText("0");

    ui->widget->repaint();

    fclose(fp);
}

void MainWindow::on_actionSave_triggered() {

    if (header.nrLayers==0) {
        QMessageBox::information(this, "Cannot do this now", "You need to load a file first.", QMessageBox::Ok);
        return;
    }

    outFile = QFileDialog::getSaveFileName(this, tr("Select file to Save to"),
                                          QDir::homePath(),
                                          tr("Photon Files (*.photon)"),nullptr,QFileDialog::DontUseNativeDialog);
    if (outFile=="") return;

    int nextFp = sizeof(headerType);
    header.preview0Addr = nextFp;
    nextFp += sizeof(previewType::padding) +
            sizeof(previewType::dataLength) +
            sizeof(previewType::resolutionX) +
            sizeof(previewType::resolutionY) +
            sizeof(previewType::imageAddress);
    preview1.imageAddress = nextFp;
    nextFp += static_cast<unsigned long>(preview1.dataLength);
    header.preview1Addr = nextFp;
    nextFp += sizeof(previewType::padding) +
            sizeof(previewType::dataLength) +
            sizeof(previewType::resolutionX) +
            sizeof(previewType::resolutionY) +
            sizeof(previewType::imageAddress);
    preview2.imageAddress = nextFp;
    nextFp += static_cast<unsigned long>(preview2.dataLength);
    header.layerDefsAddr = nextFp;
    nextFp += static_cast<int>(sizeof(layerDefType)) * header.nrLayers;
    for (unsigned long i=0; i<static_cast<unsigned long>(header.nrLayers); i++) {
        layerDefs.at(i).imageAddress=nextFp;
        nextFp+=rawData.at(i).size();
    }

    FILE *wp=fopen(outFile.toStdString().c_str(),"wb");
    writeHeader(&header,wp);
    writePreview(&preview1,wp);
    writePreview(&preview2,wp);
    writeLayerDefs(header.nrLayers, &layerDefs, wp);
    writeImages(&rawData, header.nrLayers,wp);


    QString tempStr;
    QTextStream outStr(&tempStr);
    outStr << "File: " << endl << outFile.section("/",-1,-1) << endl;
    outStr << "saved at: " << QTime::currentTime().toString() << endl << endl;
    ui->txtInfo->appendPlainText(tempStr);
}

void MainWindow::on_verticalScrollBar_valueChanged(int value)
{
    decodeImageFromRaw(rawData[static_cast<unsigned long>(value)], pixelBuf);
    ui->txtCurrentLayer->setText(QString::number(value));
    ui->widget->repaint();
}

void MainWindow::on_txtCurrentLayer_textChanged(const QString &arg1)
{
    if (arg1.toInt()<header.nrLayers-1) {
        decodeImageFromRaw(rawData[static_cast<unsigned long>(arg1.toInt())], pixelBuf);
        ui->verticalScrollBar->setValue(arg1.toInt());
    } else {
        decodeImageFromRaw(rawData[static_cast<unsigned long>(header.nrLayers-1)], pixelBuf);
        ui->verticalScrollBar->setValue(header.nrLayers-1);
    }
}



void MainWindow::on_btnDecrease_clicked() {

    if (header.nrLayers==0) {
        QMessageBox::information(this, "Cannot do this now", "You need to load a file first.", QMessageBox::Ok);
        return;
    }

    QProgressBar *progressBar = new QProgressBar;
    progressBar->setRange(0,header.nrLayers-1);
    QLabel *label= new QLabel;
    label->setText("Trimming Pixels: ");
    ui->statusBar->addPermanentWidget(label);
    ui->statusBar->addPermanentWidget(progressBar);

    progressBar->setAlignment(Qt::AlignRight);

    ui->txtInfo->appendPlainText("Starting decreasing at: " + QTime::currentTime().toString());


    int nextAddr=layerDefs[0].imageAddress;
    for (unsigned long i=0; i<static_cast<unsigned long>(header.nrLayers); i++) {
        progressBar->setValue(static_cast<int>(i));
        layerDefs[i].imageAddress=nextAddr;
        decodeImageFromRaw(rawData[i], pixelBuf);
        decreasePixel(pixelBuf);
        encodeImageToRaw(&rawData[i], pixelBuf);
        layerDefs[i].dataLength=static_cast<int>(rawData[i].size());
        nextAddr+=rawData[i].size();
    }
    ui->statusBar->removeWidget(progressBar);
    ui->statusBar->removeWidget(label);
    delete progressBar;
    delete label;
    decodeImageFromRaw(rawData[static_cast<unsigned long>(ui->txtCurrentLayer->text().toInt())], pixelBuf);
    ui->widget->repaint();
    ui->txtInfo->appendPlainText("Finished at: " + QTime::currentTime().toString());
    ui->txtInfo->appendPlainText("");

}
