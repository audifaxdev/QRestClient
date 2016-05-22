//Author : Kevin Barreau
//Async rest client
//Making outrageous use of c++11 lamdas
//Particulary useful in a QML use case thanks to QJson* -> QVariant Map/List conversion
//One would wrap an instance into a global object
//which would expose Rest api's results through properties
//License WTFPL

#ifndef QRESTCLIENT_H
#define QRESTCLIENT_H

#include <QtNetWork/QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <functional>

typedef std::function<void(QJsonDocument)> jsonPayloadFn;
//typedef std::function<void(QByteArray)> byteArrayPayloadFn;

class QRestClient : public QObject
{
    Q_OBJECT
public:
    enum AuthenticationMethod{
        NoMethod,
        OAuth2
    };

    enum ContentType{
        NoContentType,
        Json,
        ImageJpeg,
        Multipart
    };

    QRestClient(QObject*);

    //setRootUri
    void setRootUrl(QByteArray);

    //Use oAuth2 client credentials
    void oAuthLogin(QVariant, QVariant, jsonPayloadFn, jsonPayloadFn);

    AuthenticationMethod authMethod() const;

    //Blocking call
    QByteArray byteArrayCall(QByteArray method, QByteArray uri, QByteArray body = QByteArray());

    //Async
    void asyncJsonCall(QByteArray method, QByteArray uri, QJsonDocument body = QJsonDocument(),
              jsonPayloadFn successCb = {}, jsonPayloadFn errCb = {});

private:
    void setAuthMethod(AuthenticationMethod m);

    QNetworkRequest buildRequest(QUrl, ContentType = NoContentType);
    QNetworkReply* call(QByteArray method, QByteArray uri, QByteArray body = QByteArray(), ContentType = NoContentType);
    QJsonDocument buildJsonError(QString, QByteArray);

    //For debuging
    void displayHtml(QByteArray html);

    AuthenticationMethod m_authMethod;

    QByteArray m_rootUrl;
    QByteArray m_oAuthToken;
    QByteArray m_tenantId;

    QNetworkAccessManager *m_qnam;

};

#endif // QRESTCLIENT_H
