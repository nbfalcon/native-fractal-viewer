#ifndef FRACTALS_H
#define FRACTALS_H

#include "xfuture.h"
#include <memory>
#include <cmath>
#include <QImage>
#include <QThreadPool>

struct FractalViewport {
    double x1, y1;
    double x2, y2;

    void shiftByPercent(double dxP, double dyP) {
        double dx = width() * dxP;
        double dy = height() * dyP;
        x1 += dx;
        x2 += dx;
        y1 += dy;
        y2 += dy;
    }

    void zoomIn(double byFactor) {
        byFactor = std::sqrt(byFactor);
        double dw = (width() - width() / byFactor) / 2, dh = (height() - height() / byFactor) / 2;
        x1 += dw; x2 -= dw;
        y1 += dh; y2 -= dh;
    }

    void zoomOut(double byFactor) {
        byFactor = std::sqrt(byFactor);
        double dw = (width() - width() * byFactor) / 2, dh = (height() - height() * byFactor) / 2;
        x1 += dw; x2 -= dw;
        y1 += dh; y2 -= dh;
    }

    double width() const {
        return x2 - x1;
    }

    double height() const {
        return y2 - y1;
    }

    FractalViewport relativeTo(const FractalViewport &other) const {
        return FractalViewport{
            .x1 = (x1 - other.x1) / other.width(),
            .y1 = (y1 - other.y1) / other.height(),
            .x2 = (x2 - other.x1) / other.width(),
            .y2 = (y2 - other.y1) / other.height()
        };
    }

    FractalViewport slice(const QRectF &theSlice) const {
        return FractalViewport {
            .x1 = x1 + width() * theSlice.x(),
            .y1 = y1 + height() * theSlice.y(),
            .x2 = x1 + width() * (theSlice.x() + theSlice.width()),
            .y2 = y1  + height() * (theSlice.y() + theSlice.height())
        };
    }

    bool operator ==(const FractalViewport &other) const {
        return x1 == other.x1 && y1 == other.y1 && x2 == other.x2 && y2 == other.y2;
    }
};
const static FractalViewport DEFAULT_VP = {-2.0, -2.0, 2.0, 2.0};

std::shared_ptr<XPromise<QImage *> > render_mandelbrot(FractalViewport vp, int width, int height, QThreadPool &pool, int nThreads);

#endif // FRACTALS_H
