#include "qmlrest.h"
#include <QJsonDocument>
#include <QDebug>

using namespace QmlRest;

Client::Client(QObject *parent) :
    QObject(parent),
    QQuickImageProvider(
        QQuickImageProvider::Image,
        QQuickImageProvider::ForceAsynchronousImageLoading
    ),
    m_isLoading(false),
    m_restClient(new QRestClient(this))
{
    setRootUrl("https://dev.your.api/api/");

    handleError= [this](QJsonDocument payload) {
        qDebug() << payload.toJson();
        this->update_isLoading(false);
        this->update_networkError(payload.object().toVariantMap());
    };
}

QRestClient* Client::restClient()
{
    return m_restClient;
}

void Client::setRootUrl(QByteArray rootUrl)
{
    restClient()->setRootUrl(rootUrl);
}

void Client::login(QVariant id, QVariant secret)
{
    this->update_isLoading(true);
    restClient()->oAuthLogin(
        id, secret,
        [this](QJsonDocument payload){
            Q_UNUSED(payload)
            this->update_isLoading(false);
            emit loggedIn();
        },
        handleError
    );
}

void Client::refreshProducts()
{
    restClient()->asyncJsonCall(
        "get", "products",
        QJsonDocument(),
        [this](QJsonDocument payload){
            this->update_products(payload.object()["data"].toArray().toVariantList());
        },
        handleError
    );
}

QImage Client::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
    qDebug() << __PRETTY_FUNCTION__ << id << requestedSize;
    return QImage::fromData(
        restClient()->byteArrayCall("get", id.toUtf8(), QByteArray())
    );
}

QPixmap Client::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)
//    qDebug() << __PRETTY_FUNCTION__ << id << requestedSize;
    return QPixmap::fromImage(
        QImage::fromData(
            restClient()->byteArrayCall("get", id.toUtf8(), QByteArray())
        )
    );
}

