#include "remoteapiconnection.h"
#include "remoteapiprotocol.h"

#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSerialPortInfo>

#include "mainwindow.h"
#include "devinfo/redeviceinfo.h"
#include "nanovna_analyzer.h"

using namespace RemoteApiProtocol;

RemoteApiConnection::RemoteApiConnection(QTcpSocket* socket, MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_mainWindow(mainWindow)
{
    m_socket->setParent(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &RemoteApiConnection::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &RemoteApiConnection::onDisconnected);
}

void RemoteApiConnection::onReadyRead()
{
    m_lineBuffer.append(m_socket->readAll());

    int newlineIndex;
    while ((newlineIndex = m_lineBuffer.indexOf('\n')) != -1) {
        QByteArray line = m_lineBuffer.left(newlineIndex);
        m_lineBuffer.remove(0, newlineIndex + 1);
        handleLine(line);
    }
}

void RemoteApiConnection::onDisconnected()
{
    emit closed(this);
}

void RemoteApiConnection::handleLine(const QByteArray& line)
{
    QByteArray trimmed = line.trimmed();
    if (trimmed.isEmpty())
        return;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(trimmed, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        sendError(QJsonValue(), QStringLiteral("invalid JSON: %1").arg(parseError.errorString()));
        return;
    }

    dispatch(doc.object());
}

void RemoteApiConnection::dispatch(const QJsonObject& request)
{
    QJsonValue id = request.value(QLatin1String(KEY_ID));
    QString cmd = request.value(QLatin1String(KEY_CMD)).toString();

    QJsonObject result;
    if (cmd == QLatin1String(CMD_STATUS)) {
        result = cmdStatus();
    } else if (cmd == QLatin1String(CMD_DEVICES)) {
        result = cmdDevices();
    } else {
        sendError(id, QStringLiteral("unknown command: %1").arg(cmd));
        return;
    }

    sendResponse(result, id);
}

void RemoteApiConnection::sendResponse(const QJsonObject& response, const QJsonValue& id)
{
    QJsonObject out = response;
    if (!id.isUndefined())
        out.insert(QLatin1String(KEY_ID), id);
    out.insert(QLatin1String(KEY_OK), true);

    m_socket->write(QJsonDocument(out).toJson(QJsonDocument::Compact));
    m_socket->write("\n");
}

void RemoteApiConnection::sendError(const QJsonValue& id, const QString& message)
{
    QJsonObject out;
    if (!id.isUndefined())
        out.insert(QLatin1String(KEY_ID), id);
    out.insert(QLatin1String(KEY_OK), false);
    out.insert(QLatin1String(KEY_ERROR), message);

    m_socket->write(QJsonDocument(out).toJson(QJsonDocument::Compact));
    m_socket->write("\n");
}

QJsonObject RemoteApiConnection::cmdStatus() const
{
    QJsonObject result;
    bool connected = m_mainWindow->isAnalyzerConnected();
    result.insert("connected", connected);
    result.insert("measuring", m_mainWindow->isMeasuring());
    if (connected) {
        QJsonObject device;
        device.insert("name", m_mainWindow->connectedDeviceName());
        device.insert("serial", m_mainWindow->analyzer()->getSerialNumber());
        result.insert("device", device);
    } else {
        result.insert("device", QJsonValue());
    }
    return result;
}

QJsonObject RemoteApiConnection::cmdDevices() const
{
    // Mirrors SelectDeviceDialog::onScan()'s HID + Serial/NanoVNA
    // enumeration (src/selectdevicedialog.cpp), minus the table-widget
    // population. BLE is deliberately out of scope for v1 -- it's an
    // async scan-then-callback flow (BleAnalyzer::devicesChanged), not a
    // synchronous "list what's there right now" query like the others.
    QJsonArray devices;

    foreach (const ReDeviceInfo& info, ReDeviceInfo::availableDevices(ReDeviceInfo::HID)) {
        QString serial = info.serial().trimmed();
        if (serial.mid(0, 4) == "5001") // skip REAMP, same exclusion as onScan()
            continue;
        QJsonObject dev;
        dev.insert("type", "hid");
        dev.insert("name", info.systemName().replace("Analyzer", "", Qt::CaseInsensitive).trimmed());
        dev.insert("serial", serial);
        devices.append(dev);
    }

    foreach (const ReDeviceInfo& info, ReDeviceInfo::availableDevices(ReDeviceInfo::Serial)) {
        QJsonObject dev;
        dev.insert("type", "serial");
        dev.insert("name", info.deviceName(info).replace("Analyzer", "", Qt::CaseInsensitive).trimmed());
        dev.insert("port", info.portName().trimmed());
        devices.append(dev);
    }

    NanovnaAnalyzer::detectPorts();
    foreach (const QSerialPortInfo& info, NanovnaAnalyzer::availablePorts()) {
        QJsonObject dev;
        dev.insert("type", "nano");
        dev.insert("name", "NanoVNA");
        dev.insert("port", info.portName().trimmed());
        devices.append(dev);
    }

    QJsonObject result;
    result.insert("devices", devices);
    return result;
}
