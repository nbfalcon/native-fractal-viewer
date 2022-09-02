#include "fractalviewerwidget.h"
#include "fractals.h"
#include "xqconnection.h"
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
    std::shared_ptr<std::atomic_bool> newCancelHandle = std::make_shared<std::atomic_bool>(false);
    lastCancelHandle = newCancelHandle;

    XQConnection *connToWidget = new XQConnection(this);
    int w = width(), h = height();
    uiRenderPool.start([newCancelHandle = std::move(newCancelHandle), connToWidget, w, h](){
        if (!newCancelHandle->load()) {
            QImage *result = render_mandelbrot(w, h);
            connToWidget->invoke([result](QObject *actuallyTheViewer){
                static_cast<FractalViewerWidget*>(actuallyTheViewer)->updateImage(result);
            });
        }
        // You die anyway
        connToWidget->deleteLater();
    });
}

void FractalViewerWidget::updateImage(QImage *newIm) {
    this->currentFractal = std::unique_ptr<QImage>(newIm);
    update();
}
