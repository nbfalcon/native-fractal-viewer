#ifndef FRACTALVIEWERWIDGET_H
#define FRACTALVIEWERWIDGET_H

#include <memory>
#include <QWidget>
#include <QRunnable>
#include <QThreadPool>

class FractalViewerWidget : public QWidget
{
    Q_OBJECT

    std::unique_ptr<QImage> currentFractal;
    std::shared_ptr<std::atomic_bool> lastCancelHandle;

    QThreadPool uiRenderPool;

    void queueUpdate();

public:
    FractalViewerWidget();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateImage(QImage *newPm);
};

#endif // FRACTALVIEWERWIDGET_H
