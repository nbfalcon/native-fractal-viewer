#include "xqconnection.h"

XQConnection::XQConnection(QObject *connectWith)
    : connected(connectWith)
{
}

void XQConnection::invoke(std::function<void(QObject *)> cb) {
    QMetaObject::invokeMethod(this, [this, cb = std::move(cb)](){
        if (!connected.isNull()) {
            cb(connected);
        }
    }, Qt::QueuedConnection);
}
