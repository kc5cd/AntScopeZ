#ifndef REMOTEAPICONNECTION_H
#define REMOTEAPICONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>

class QTcpSocket;
class MainWindow;

// One instance per accepted QTcpSocket. Owns that socket's NDJSON framing
// (readyRead() can deliver a partial line, or several lines at once -- both
// need buffering across calls) and dispatches each complete line as one
// request. Deliberately reaches AnalyzerPro directly via MainWindow, the
// same way Measurements does, rather than through a specific GUI widget --
// see the json-tcp-api plan's Context section for why (the abandoned
// OneFqWidget UDP bridge only worked while its own dialog happened to be
// open, which is the opposite of what this needs).
class RemoteApiConnection : public QObject
{
    Q_OBJECT
public:
    explicit RemoteApiConnection(QTcpSocket* socket, MainWindow* mainWindow, QObject* parent = nullptr);

signals:
    void closed(RemoteApiConnection* self);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void handleLine(const QByteArray& line);
    void dispatch(const QJsonObject& request);
    void sendResponse(const QJsonObject& response, const QJsonValue& id);
    void sendError(const QJsonValue& id, const QString& message);

    QJsonObject cmdStatus() const;
    QJsonObject cmdDevices() const;

    QTcpSocket* m_socket;
    MainWindow* m_mainWindow;
    QByteArray m_lineBuffer;
};

#endif // REMOTEAPICONNECTION_H
