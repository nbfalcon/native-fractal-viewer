#ifndef FRACTALS_H
#define FRACTALS_H

#include <QImage>
#include <QThreadPool>

void render_mandelbrot(int width, int height, QThreadPool &pool, int nThreads, std::function<void(QImage *result)> onResult);

#endif // FRACTALS_H
