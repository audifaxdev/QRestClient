
#ifndef QMLREST_H
#define QMLREST_H

#include "qqmlautopropertyhelpers.h"
//http://gitlab.unique-conception.org/qt-qml-tricks/qt-supermacros/

#include <QQuickImageProvider>
#include <QObject>

#include "qrestclient.h"

namespace QmlRest {

    class Client : public QObject, public QQuickImageProvider
    {
        Q_OBJECT
    public:
        explicit Client(QObject *parent = 0);

        //setRootUri
        void setRootUrl(QByteArray);

        //QML Invokable
        //Use oAuth2 client credentials
        Q_INVOKABLE void login(QVariant, QVariant);

        //REST
        Q_INVOKABLE void refreshProducts(QString);

        QML_READONLY_AUTO_PROPERTY(QVariantMap, networkError)
        QML_READONLY_AUTO_PROPERTY(bool, isLoading)

        QML_READONLY_AUTO_PROPERTY(QVariantList, products)

        QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
        QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);

    signals:
         //Important to leave the name of params for qml to grab them
        void loggedIn();

    private:

        jsonPayloadFn handleError;

        QRestClient* restClient();

        QRestClient *m_restClient;
    };

}
#endif // QMLREST_H

