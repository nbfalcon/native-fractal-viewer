#include "fractals.h"
#include <complex>

QImage *render_mandelbrot(int width, int height) {
    std::complex<double> start{-2.0, -2.0}, end{2.0, 2.0};

    QImage *result = new QImage(width, height, QImage::Format_RGB888);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double xF = start.real() + (end.real() - start.real()) / (double)width * (double)x;
            double yF = start.imag() + (end.imag() - start.imag()) / (double)height * (double)y;

            std::complex<double> z{0.0, 0.0};
            std::complex<double> c{xF, yF};

            int nIter = 0;
            while (z.real() * z.real() + z.imag() * z.imag() <= 4 && nIter <= 255) {
                z = z * z + c;
                nIter++;
            }

            result->setPixel(x, y, qRgb(nIter, nIter, nIter));
        }
    }
    return result;
}
