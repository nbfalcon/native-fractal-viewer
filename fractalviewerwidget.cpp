#include "fractalviewerwidget.h"
#include "fractals.h"
#include "xqconnection.h"
#include <cmath>
#include <optional>
#include <QPainter>
#include <QAction>
#include <QKeySequence>
#include <QWheelEvent>
#include <QtConcurrent/QtConcurrent>
// #include <iostream>

struct ImageSlice {
    QRectF srcSlice;
    QRectF targetSlice;
};

static std::optional<ImageSlice> getViewPortSlice(const FractalViewport &drawViewPort, const FractalViewport &imageViewport, int imWidth, int imHeight) {
    // The current image, relative to the draw surface; this is where we'll draw it
    FractalViewport imageInDraw = imageViewport.relativeTo(drawViewPort).clip(0.0, 0.0, 1.0, 1.0);
    // The draw surface, within the image; this is the actual visible part of the image that we will select (and draw)
    FractalViewport drawInImage = drawViewPort.relativeTo(imageViewport).clip(0.0, 0.0, 1.0, 1.0);

    return ImageSlice{
        .srcSlice = drawInImage.asQRectF(imWidth, imHeight),
        .targetSlice = imageInDraw.asQRectF(imWidth, imHeight)
    };
}

FractalViewerWidget::FractalViewerWidget()
    : viewPort(DEFAULT_VP)
{
    uiRenderPool.setMaxThreadCount(8);
    createActions();
}

void FractalViewerWidget::createActions() {
    QAction *panUp = new QAction(tr("Pan &Up"), this);
    connect(panUp, &QAction::triggered, this, [this](){ shiftBy(0.0, -0.1); });
    panUp->setShortcut(Qt::Key_Up);

    QAction *panDown = new QAction(tr("Pan &Down"), this);
    connect(panDown, &QAction::triggered, this, [this](){ shiftBy(0.0, 0.1); });
    panDown->setShortcut(Qt::Key_Down);

    QAction *panLeft = new QAction(tr("Pan &Left"), this);
    connect(panLeft, &QAction::triggered, this, [this](){ shiftBy(-0.1, 0.0); });
    panLeft->setShortcut(Qt::Key_Left);

    QAction *panRight = new QAction(tr("Pan &Right"), this);
    connect(panRight, &QAction::triggered, this, [this](){ shiftBy(+0.1, 0.0); });
    panRight->setShortcut(Qt::Key_Right);

    QAction *zoomIn = new QAction(tr("Zoom &In"));
    connect(zoomIn, &QAction::triggered, this, &FractalViewerWidget::zoomIn);
    zoomIn->setShortcuts(QList<QKeySequence>{QKeySequence::StandardKey::ZoomIn, Qt::Key_Plus});

    QAction *zoomOut = new QAction(tr("Zoom &Out"));
    connect(zoomOut, &QAction::triggered, this, &FractalViewerWidget::zoomOut);
    zoomOut->setShortcuts(QList<QKeySequence>{QKeySequence::StandardKey::ZoomOut, Qt::Key_Minus});

    addAction(panUp);
    addAction(panDown);
    addAction(panLeft);
    addAction(panRight);
    addAction(zoomIn);
    addAction(zoomOut);
}

void FractalViewerWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (currentFractal.rendering) {
        if (lastContinuousZoomTimeStamp.isValid()) {
            updateForContinuousZoom(continuousZoomX, continuousZoomY);
        }
        std::optional<ImageSlice> theSliceM;
        if (currentFractal.actualViewPort == viewPort
                || !(theSliceM = getViewPortSlice(currentFractal.actualViewPort, viewPort, currentFractal.rendering->width(), currentFractal.rendering->height()))) {
            painter.drawImage(QPoint(), *currentFractal.rendering);
        }
        else {
            const ImageSlice &theSlice = *theSliceM;
            painter.drawImage(theSlice.srcSlice, *currentFractal.rendering, theSlice.targetSlice);
        }

        selection.paint(painter, this);
    }
    else {
        painter.drawText(rect(), Qt::AlignCenter, tr("Rendering fractal, please wait..."));
    }
}

void FractalViewerWidget::resizeEvent(QResizeEvent *) {
    queueUpdate();
}

void FractalViewerWidget::wheelEvent(QWheelEvent *event) {
    shiftBy(-(double)event->pixelDelta().x() / 96.0, -(double)event->pixelDelta().y() / 96.0);
}

void FractalViewerWidget::shiftBy(double dx, double dy) {
    viewPort.shiftByPercent(dx, dy);
    update2();
}

void FractalViewerWidget::queueUpdate() {
    if (lastCancelHandle) {
        lastCancelHandle->cancel();
    }

    XQConnection *connToWidget = new XQConnection(this);
    FractalViewport viewPortForRender = viewPort;
    std::shared_ptr<XPromise<QImage *>> imagePromise = render_mandelbrot(viewPortForRender, width(), height(), uiRenderPool, 8);
    imagePromise->onComplete([connToWidget, viewPortForRender](QImage *result){
        connToWidget->invoke([result, viewPortForRender](QObject *actuallyTheViewer){
            FractalViewerWidget *self = static_cast<FractalViewerWidget*>(actuallyTheViewer);
            self->currentFractal = FractalRendering{.rendering = std::unique_ptr<QImage>(result), .actualViewPort = viewPortForRender};
            self->update();
        });
        connToWidget->deleteLater();
    });
    lastCancelHandle = imagePromise;
}

void FractalViewerWidget::update2() {
    queueUpdate();
    update();
}

// Rubber-band
void FractalViewerWidget::mousePressEvent(QMouseEvent *event) {
    if (event->modifiers() & Qt::AltModifier) {
        continuousZoomX = (float)event->x() / width();
        continuousZoomY = (float)event->y() / height();
        lastContinuousZoomTimeStamp.start();
    }
    else {
        selection.begin(event, this);
        // FIXME: optimize: update real rectangle
        update();
    }
}

void FractalViewerWidget::mouseMoveEvent(QMouseEvent *event) {
    selection.move(event, this);

    continuousZoomX = (float)event->x() / width();
    continuousZoomY = (float)event->y() / height();
    // std::cout << "Mouse move: " << event->x() << ":" << event->y() << std::endl;

    update();
}

void FractalViewerWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (lastContinuousZoomTimeStamp.isValid()) {
        updateForContinuousZoom((float)event->x() / width(), (float)event->y() / height());
        lastContinuousZoomTimeStamp.invalidate();
    }
    else {
        selection.finish();

        if (!selection.empty()) {
            viewPort = viewPort.slice(selection.getSelection());
            update2();
        }
        else if ((event->modifiers() & Qt::ControlModifier)
                 && (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)) {
            // On Click, recenter and zoom in
            viewPort.centerInOnPoint((double)event->x() / width(), (double)event->y() / height(), 1.0);
            if (event->button() == Qt::LeftButton) {
                viewPort.zoomIn(2);
            } else {
                viewPort.zoomOut(2);
            }
            update2();
        }
    }
}

// "Slots"
void FractalViewerWidget::zoomIn() {
    viewPort.zoomIn(2);
    update2();
}

void FractalViewerWidget::zoomOut() {
    viewPort.zoomOut(2);
    update2();
}

void FractalViewerWidget::updateForContinuousZoom(float x, float y) {
    qint64 deltaT = lastContinuousZoomTimeStamp.restart();
    // std::cout << "Center: " << x << ":" << y << std::endl;
    viewPort.centerInOnPoint((double)x, (double)y, 0.1 * (double)deltaT / 1000);

    double zoomCoeff = 1.0 + (double)deltaT / 1000 * 0.2;
    // std::cout << "Zoom: " << zoomCoeff << std::endl;
    viewPort.zoomIn(zoomCoeff);
    update2();
}
