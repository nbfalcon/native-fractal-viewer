#include "fractalviewerwidget.h"
#include "fractals.h"
#include "xqconnection.h"
#include <QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

FractalViewerWidget::FractalViewerWidget()
{
    uiRenderPool.setMaxThreadCount(8);
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
        lastCancelHandle->cancel();
    }

    XQConnection *connToWidget = new XQConnection(this);
    std::shared_ptr<XPromise<QImage *>> imagePromise = render_mandelbrot(width(), height(), uiRenderPool, 8);
    imagePromise->onComplete([connToWidget](QImage *result){
        connToWidget->invoke([result](QObject *actuallyTheViewer){
            static_cast<FractalViewerWidget*>(actuallyTheViewer)->updateImage(result);
        });
        connToWidget->deleteLater();
    });
    lastCancelHandle = imagePromise;
}

void FractalViewerWidget::updateImage(QImage *newIm) {
    this->currentFractal = std::unique_ptr<QImage>(newIm);
    update();
}
