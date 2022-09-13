#ifndef FRACTALVIEWERWIDGET_H
#define FRACTALVIEWERWIDGET_H

#include "fractals.h"
#include "xfuture.h"
#include "xqrubberband.h"
#include <memory>
#include <QWidget>
#include <QRunnable>
#include <QThreadPool>

struct FractalRendering {
    std::unique_ptr<QImage> rendering;
    FractalViewport actualViewPort;
};

class FractalViewerWidget : public QWidget
{
    Q_OBJECT

    FractalViewport viewPort;

    FractalRendering currentFractal;
    std::shared_ptr<XPromise<QImage *>> lastCancelHandle;

    QThreadPool uiRenderPool;

    // Bonus widgets
    XQRubberBand selection;

    void queueUpdate();
    void shiftBy(double dx, double dy);
    void createActions();
    void update2();
public:
    FractalViewerWidget();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    // Rubber-band
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void zoomIn();
    void zoomOut();
};

#endif // FRACTALVIEWERWIDGET_H
