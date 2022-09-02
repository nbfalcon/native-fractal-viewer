#ifndef FRACTALS_H
#define FRACTALS_H

#include "xfuture.h"
#include <memory>
#include <QImage>
#include <QThreadPool>

struct FractalViewport {
    double x1, y1;
    double x2, y2;

    void shiftBy(double dx, double dy) {
        x1 += dx;
        x2 += dx;
        y1 += dy;
        y2 += dy;
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

    bool operator ==(FractalViewport &other) {
        return x1 == other.x1 && y1 == other.y1 && x2 == other.x2 && y2 == other.y2;
    }
};
const static FractalViewport DEFAULT_VP = {-2.0, -2.0, 2.0, 2.0};

std::shared_ptr<XPromise<QImage *> > render_mandelbrot(FractalViewport vp, int width, int height, QThreadPool &pool, int nThreads);

#endif // FRACTALS_H
