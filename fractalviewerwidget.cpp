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

struct ImageSlice {
    QRectF source;
    QRectF target;
};

// Adapted from my java fractal viewer; I don't want to derive this fancy algorithm again
static std::optional<ImageSlice> getViewPortSlice(const FractalViewport &actualViewport, const FractalViewport &currentViewport, int imWidth, int imHeight) {
    FractalViewport whichSlice = actualViewport.relativeTo(currentViewport);

    // Without round, the image ends up jumping by a pixel once the fractal re-renders at the new
    // viewport. This is ever so *slightly* jarring.
    int rawX = (int) std::round(whichSlice.x1 * imWidth);
    int rawY = (int) std::round(whichSlice.y1 * imHeight);
    int x = std::max(0, rawX), y = std::max(0, rawY);

    int rawWidth = (int) std::round(whichSlice.width() * imWidth);
    int maxWidth = imWidth - x;
    int width = std::min(maxWidth, rawWidth);
    int rawHeight = (int) std::round(whichSlice.height() * imHeight);
    int maxHeight = imHeight - y;
    int height = std::min(maxHeight, rawHeight);

    // width & height > 0 implies x & y are in bounds, due to the min above
    if (width <= 0 || height <= 0) return std::nullopt;

    return ImageSlice{
        .source = QRectF(x, y, width, height),
        .target = QRectF(-std::min(0, rawX), -std::min(0, rawY),
                        imWidth - std::max(0, rawWidth - maxWidth),
                        imHeight - std::max(0, rawHeight - maxHeight))
    };
//    return new SliceImageResult(
//            sliceMe.getSubimage(x, y, width, height),
//            -Math.min(0, rawX), -Math.min(0, rawY),
//            // Negative if we had to clip on rhs
//            Math.max(0, rawWidth - maxWidth),
//            Math.max(0, rawHeight - maxHeight));
}

FractalViewerWidget::FractalViewerWidget()
    : viewPort(DEFAULT_VP)
{
    uiRenderPool.setMaxThreadCount(8);
    createActions();
}

void FractalViewerWidget::createActions() {
    QAction *panUp = new QAction(tr("Pan Up"), this);
    connect(panUp, &QAction::triggered, this, [this](){ shiftBy(0.0, -0.1); });
    panUp->setShortcut(Qt::Key_Up);

    QAction *panDown = new QAction(tr("Pan Down"), this);
    connect(panDown, &QAction::triggered, this, [this](){ shiftBy(0.0, 0.1); });
    panDown->setShortcut(Qt::Key_Down);

    QAction *panLeft = new QAction(tr("Pan Left"), this);
    connect(panLeft, &QAction::triggered, this, [this](){ shiftBy(-0.1, 0.0); });
    panLeft->setShortcut(Qt::Key_Left);

    QAction *panRight = new QAction(tr("Pan Right"), this);
    connect(panRight, &QAction::triggered, this, [this](){ shiftBy(+0.1, 0.0); });
    panRight->setShortcut(Qt::Key_Right);

    addAction(panUp);
    addAction(panDown);
    addAction(panLeft);
    addAction(panRight);
}

void FractalViewerWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (currentFractal.rendering) {
        std::optional<ImageSlice> theSliceM;
        if (currentFractal.actualViewPort == viewPort
                || !(theSliceM = getViewPortSlice(currentFractal.actualViewPort, viewPort, currentFractal.rendering->width(), currentFractal.rendering->height()))) {
            painter.drawImage(QPoint(), *currentFractal.rendering);
        }
        else {
            const ImageSlice &theSlice = *theSliceM;
            painter.drawImage(theSlice.source, *currentFractal.rendering, theSlice.target);
        }
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
    viewPort.shiftBy(dx, dy);
    update();
    queueUpdate();
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
