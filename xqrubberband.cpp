#include "xqrubberband.h"
#include <cmath>
#include <QColor>

XQRubberBand::XQRubberBand()
    : haveSelection(false)
{
}

void XQRubberBand::begin(float startX, float startY) {
    haveSelection = true;
    this->startX = endX = startX;
    this->startY = endY = startY;
}

void XQRubberBand::begin(QMouseEvent *event, QWidget *parent) {
    float x = (float)event->pos().x() / parent->width(),
            y = (float)event->pos().y() / parent->height();
    begin(x, y);
}

void XQRubberBand::move(float endX, float endY) {
    this->endX = endX;
    this->endY = endY;
}

void XQRubberBand::move(QMouseEvent *event, QWidget *parent) {
    float x = (float)event->pos().x() / parent->width(),
            y = (float)event->pos().y() / parent->height();
    // Shift = square selection
    if (event->modifiers() & Qt::ControlModifier) {
        float dx = x - startX, dy = y - startY;
        float delta = std::max(std::abs(dx), std::abs(dy));
        float dxM = dx < 0 ? -delta : delta, dyM = dy < 0 ? -delta : delta;
        x = startX + dxM;
        y = startY + dyM;
    }
    move(x, y);
}

void XQRubberBand::finish() {
    haveSelection = false;
}

bool XQRubberBand::empty() {
    return !haveSelection || startX == endX || startY == endY;
}

QRectF XQRubberBand::getSelection() {
    double x1 = std::min(startX, endX), x2 = std::max(startX, endX);
    double y1 = std::min(startY, endY), y2 = std::max(startY, endY);
    return QRectF(x1, y1, x2 - x1, y2 - y1);
}

// FIXME: optimize repaints
// FIXME: use a real QRubberBand
void XQRubberBand::paint(QPainter &painter, QWidget *parent) {
    if (haveSelection) {
        double x1 = std::min(startX, endX), x2 = std::max(startX, endX);
        double y1 = std::min(startY, endY), y2 = std::max(startY, endY);
        QRectF area = QRectF(QPoint(x1 * parent->width(), y1 * parent->height()), QPoint(x2 * parent->width(), y2 * parent->height()));

//        QBrush brush = QBrush(QColorConstants::Blue);
//        painter.fillRect(area, brush);
        painter.setPen(QColorConstants::Blue);
        painter.drawRect(area);
    }
}
