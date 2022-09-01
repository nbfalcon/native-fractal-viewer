#include "fractalviewerwidget.h"
#include "fractals.h"
#include <QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

FractalViewerWidget::FractalViewerWidget()
    : uiRenderPool{}
{
    uiRenderPool.setMaxThreadCount(1);
}

void FractalViewerWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (currentFractal) {
        painter.drawImage(QPoint(), *currentFractal);
    }
    else {
        painter.drawText(rect(), Qt::AlignCenter, tr("Rendering fractal, please wait..."));
    }
}

void FractalViewerWidget::resizeEvent(QResizeEvent *) {
    queueUpdate();
}

void FractalViewerWidget::queueUpdate() {
    if (lastCancelHandle) {
        lastCancelHandle->store(true);
    }
    this->lastCancelHandle = std::make_shared<std::atomic_bool>(false);

    FractalRenderJob *render = new FractalRenderJob(width(), height(), lastCancelHandle);
    connect(render, &FractalRenderJob::haveResult, this, &FractalViewerWidget::updateImage, Qt::QueuedConnection);
    uiRenderPool.start(render);
}

void FractalViewerWidget::updateImage(QImage *newIm) {
    this->currentFractal = std::unique_ptr<QImage>(newIm);
    update();
}

FractalRenderJob::FractalRenderJob(int width, int height, std::shared_ptr<std::atomic_bool> cancelHandle)
    : width(width), height(height), cancelHandle(std::move(cancelHandle))
{}

void FractalRenderJob::run() {
    if (!cancelHandle->load()) {
        QImage *r = render_mandelbrot(width, height);
        emit haveResult(r);
    }
}
