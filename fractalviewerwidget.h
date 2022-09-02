#ifndef FRACTALVIEWERWIDGET_H
#define FRACTALVIEWERWIDGET_H

#include "fractals.h"
#include "xfuture.h"
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

    void queueUpdate();
    void shiftBy(double dx, double dy);
public:
    FractalViewerWidget();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};

#endif // FRACTALVIEWERWIDGET_H
