#ifndef FRACTALS_H
#define FRACTALS_H

#include "xfuture.h"
#include <memory>
#include <QImage>
#include <QThreadPool>

std::shared_ptr<XPromise<QImage *> > render_mandelbrot(int width, int height, QThreadPool &pool, int nThreads);

#endif // FRACTALS_H
