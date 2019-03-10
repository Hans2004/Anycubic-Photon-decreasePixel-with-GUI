#include "graphicswidget.h"
#include "mainwindow.h"
//#include <QDebug>
#include <QStyleOption>

graphicsWidget::graphicsWidget(QWidget *parent) : QWidget(parent)
{

}


void graphicsWidget::paintEvent([[gnu::unused]] QPaintEvent *event) {
    QPainter painter(this);
    QPen pen;
    unsigned char c=0;

    // Next three lines needed to apply the background style defined in mainwindow.ui
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    int xStep=1440/painter.device()->width();
    int yStep=2560/painter.device()->height();
    for (int y=0; y<2560; y+=yStep) {
        for (int x=0; x<1440; x+=xStep) {
           c=static_cast<unsigned char>(showBuf[x][y].color*255);
           pen.setColor(QColor(c,c,c));
           painter.setPen(pen);
           painter.drawPoint(x/xStep,y/yStep);
        }
    }
    painter.end();
}
