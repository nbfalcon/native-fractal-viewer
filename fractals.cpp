#include "fractals.h"
#include "xfuture.h"
#include <complex>

std::shared_ptr<XPromise<QImage *>> render_mandelbrot(FractalViewport viewPort, int width, int height, QThreadPool &pool, int nThreads) {
    std::complex<double> start{viewPort.x1, viewPort.y1}, end{viewPort.x2, viewPort.y2};

    std::atomic_int *countDown = new std::atomic_int(nThreads);
    std::shared_ptr<XPromise<QImage *>> resultPromise = std::make_shared<XPromise<QImage *> >();

    QImage *result = new QImage(width, height, QImage::Format_RGB32);
    for (int threadNo = 0; threadNo < nThreads; threadNo++) {
        pool.start([resultPromise, countDown, threadNo, nThreads, result, width, height, start, end](){
            // FIXME: do we care about synchronization? Also, can we just use weak?
            if (!resultPromise->started()) {
                if (resultPromise->cancelled()) return;
                resultPromise->start();
            }

            for (int y = threadNo; y < height; y += nThreads) {
                uint *writeLine = reinterpret_cast<uint*>(result->scanLine(y));
                for (int x = 0; x < width; x++) {
                    double xF = start.real() + (end.real() - start.real()) / (double)width * (double)x;
                    double yF = start.imag() + (end.imag() - start.imag()) / (double)height * (double)y;

                    std::complex<double> z{0.0, 0.0};
                    std::complex<double> c{xF, yF};

                    int nIter = 0;
                    while (z.real() * z.real() + z.imag() * z.imag() <= 4 && nIter < 255) {
                        z = z * z + c;
                        nIter++;
                    }

                    writeLine[x] = qRgb(nIter, nIter, nIter);
                }
            }

            // Last thread -> we're done
            if (countDown->fetch_sub(1, std::memory_order_acq_rel) == 1) {
                delete countDown;
                resultPromise->complete(result);
            }
        });
    }

    return resultPromise;
}
