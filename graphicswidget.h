#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include <QWidget>
#include <QtCore>
#include <QtGui>
#include "fileutils.h"

class graphicsWidget : public QWidget
{
    Q_OBJECT
public:


    explicit graphicsWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
signals:

public slots:

private:

};

#endif // GRAPHICSWIDGET_H
