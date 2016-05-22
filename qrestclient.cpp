#include "qrestclient.h"
#include <QNetworkRequest>
#include <QIODevice>
#include  <QUrl>

#include <QtWebEngineWidgets>

#undef QRESTCLIENT_DEBUG
#ifdef QRESTCLIENT_DEBUG
#include <QDebug>
#endif

QRestClient::QRestClient(QObject* parent) :
    QObject(parent),
    m_authMethod(NoMethod),
    m_qnam(new QNetworkAccessManager(this))
{

}

void QRestClient::setRootUrl(QByteArray rootUrl)
{
    this->m_rootUrl = rootUrl;
}

void QRestClient::oAuthLogin(
        QVariant id,
        QVariant secret,
        jsonPayloadFn successCb,
        jsonPayloadFn errCb)
{
    QJsonObject clientCredentials
    {
        {"client_id", QJsonValue::fromVariant(id)},
        {"client_secret", QJsonValue::fromVariant(secret)},
        {"grant_type", QJsonValue::fromVariant("client_credentials")}
    };

    this->asyncJsonCall("post", "oauth/access_token",
        QJsonDocument(clientCredentials),
        //Success callback
        [this, successCb, errCb](QJsonDocument payload){
            if(!payload.isObject() || !payload.object().contains("access_token")) {
                //Custom oAuth Error
                if(errCb)
                    errCb(buildJsonError("OAuth2: Error while getting bearer token", payload.toJson()));
                return;
            }

            this->m_oAuthToken = payload.object()["access_token"].toVariant().toByteArray();
            this->setAuthMethod(OAuth2);
            if(successCb)
                successCb(QJsonDocument());
        //For other errors
        },
        errCb
    );
}

QByteArray QRestClient::byteArrayCall(
        QByteArray method,
        QByteArray uri,
        QByteArray body)
{
    QEventLoop eventLoop;
    QNetworkReply *rep = call(method, uri, body);
    connect(rep, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QByteArray data = rep->readAll();
    rep->deleteLater();
    return data;
}


void QRestClient::asyncJsonCall(
        QByteArray method,
        QByteArray uri,
        QJsonDocument body,
        jsonPayloadFn successCb,
        jsonPayloadFn errCb)
{
    QNetworkReply *rep = call(method, uri, body.toJson(), Json);
    connect(rep, &QNetworkReply::finished,
            this, [this, successCb, errCb](){

                QNetworkReply *rep = dynamic_cast<QNetworkReply*>(sender());
                if(!rep) return;

                QJsonDocument body = QJsonDocument::fromJson(QString(rep->readAll()).toUtf8());
#ifdef QRESTCLIENT_DEBUG
                qDebug() << __PRETTY_FUNCTION__ << body.toJson();
#endif
                if(rep->error() == QNetworkReply::NoError && successCb){
                    successCb(body);
                }else if(errCb){
                    errCb(buildJsonError(rep->errorString(), body.toJson()));
                }
                rep->deleteLater();
    });
}

QNetworkReply* QRestClient::call(
        QByteArray method,
        QByteArray uri,
        QByteArray body,
        ContentType cType)
{
#ifdef QRESTCLIENT_DEBUG
    qDebug() << __PRETTY_FUNCTION__ << method << uri << body;
#endif
    QNetworkRequest req = buildRequest(QUrl(m_rootUrl + uri), cType);
    QNetworkReply *rep = nullptr;

    if (method == "get") {
        rep = m_qnam->get(req);
    }else if (method == "post") {
        rep = m_qnam->post(req, body);
    }else if (method == "put") {
        rep = m_qnam->put(req, body);
    }else if (method == "delete") {
        rep = m_qnam->deleteResource(req);
    }
    return rep;
}

QRestClient::AuthenticationMethod QRestClient::authMethod() const
{
    return m_authMethod;
}

void QRestClient::setAuthMethod(AuthenticationMethod m)
{
    m_authMethod = m;
}

QNetworkRequest QRestClient::buildRequest(QUrl url, ContentType cType)
{
#ifdef QRESTCLIENT_DEBUG
    qDebug() << __PRETTY_FUNCTION__ << url;
#endif
    QNetworkRequest req = QNetworkRequest(url);

    switch(cType){
    case Json:
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        break;
    case Multipart:
        break;
    default:
        break;
    }

    switch(this->authMethod()) {
    case OAuth2:
        req.setRawHeader("Authorization", "Bearer "+this->m_oAuthToken);
        break;
    default:
        break;
    }
#ifdef QRESTCLIENT_DEBUG
    foreach(QByteArray headerName, req.rawHeaderList())
        qDebug() << __PRETTY_FUNCTION__ << "HTTP HEADER : " << headerName << req.rawHeader(headerName);
#endif
    return req;
}

QJsonDocument QRestClient::buildJsonError(QString errStr, QByteArray body)
{
    QJsonDocument doc = QJsonDocument::fromJson(body);
    QJsonObject rootObj = doc.object();
    QJsonValue val = QJsonValue::fromVariant(errStr);
#ifdef QRESTCLIENT_DEBUG
    qDebug() << __PRETTY_FUNCTION__ << errStr << doc.toJson();
#endif

    if(rootObj.isEmpty()) {
        rootObj = QJsonObject{ {"errStr",val} };
    }else{
        rootObj.insert("errStr", val);
    }
    return QJsonDocument(rootObj);
}

void QRestClient::displayHtml(QByteArray html)
{
    QWebEngineView *view = new QWebEngineView();
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->setHtml(html);
    view->show();
}
