#ifndef XQRUBBERBAND_H
#define XQRUBBERBAND_H

#include <QPainter>
#include <QWidget>
#include <QMouseEvent>

class XQRubberBand
{
    float startX, startY;
    float endX, endY;

    bool haveSelection;

public:
    XQRubberBand();

    void begin(float startX, float startY);
    void begin(QMouseEvent *event, QWidget *parent);
    void move(float endX, float endY);
    // Also takes into account shift-down
    void move(QMouseEvent *event, QWidget *parent);
    void finish();
    bool empty();

    QRectF getSelection();

    void paint(QPainter &painter, QWidget *parent);
};

#endif // XQRUBBERBAND_H
