#include "fractalviewerwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    FractalViewerWidget viewer{};

    viewer.resize(800, 600);
    viewer.setWindowTitle("Fractal Viewer - Mandelbrot");

    viewer.show();

    return app.exec();
}
