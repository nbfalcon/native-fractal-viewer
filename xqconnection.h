#ifndef XQCONNECTION_H
#define XQCONNECTION_H

#include <QObject>
#include <QPointer>

// A QPointer-wrapper to allow posting messages to another QObject from another thread without lifetime issues.
// The other object may die unexpectedly, but since this a QObject, the thread it lived in cannot, meaning we can post a message
// and check if the connection is still ok.
//
// Use deleteLater after imvoke.
class XQConnection : public QObject
{
    Q_OBJECT

    QPointer<QObject> connected;

public:
    explicit XQConnection(QObject *connectWith);

    void invoke(std::function<void(QObject *)> postToTarget);
};

#endif // XQCONNECTION_H
