#ifndef MXZMQ_H
#define MXZMQ_H

#include <QThread>
#include <zmqpp/zmqpp.hpp>
#include <QJsonObject>
#include "defines.h"

class MXZmq : public QThread
{
    Q_OBJECT
public:
    explicit MXZmq(QObject *parent, QString topic, zmqpp::endpoint_t endpoint);
    ~MXZmq();

    // Methods
    void run() override;
    void connect();
    void disconnect();
    QJsonObject payloadData();
    QJsonDocument payloadDocument();    // can contain multiple Json objects
    bool validateEndpoint(const QString &endpoint);

    // Accessors
    bool isConnected() { return _connected; }
    QString payload() { return QString::fromStdString(_payload); } // convert std string to QString
    QString endpoint() { return QString::fromStdString(_endpoint); }

    // Pubblic properties
    QString format = "unknown";
    QString topic;

public slots:
    bool setEndpoint(const QString &text);
    bool setEndpoint(const zmqpp::endpoint_t &text);

signals:
    void gotNewMessage(const QJsonObject obj);
    void gotInvalidPayload(const std::invalid_argument &ex, const QString &payload);
    void gotWrongMessage(int parts);
    void gotNoMessage();

private:
    // use _ for the global variable (is just a convention)
    zmqpp::context *_context;
    zmqpp::socket *_socket;
    zmqpp::endpoint_t _endpoint;
    std::string _payload = "";          // zmqpp use std::string, then we will convert for Qt application
    bool _connected = false;
};

#endif // MXZMQ_H
