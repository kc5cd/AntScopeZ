#include "remoteapiconnection.h"
#include "remoteapiprotocol.h"

#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSerialPortInfo>

#include "mainwindow.h"
#include "devinfo/redeviceinfo.h"
#include "nanovna_analyzer.h"
#include "selectdevicedialog.h"
#include "analyzerparameters.h"
#include "analyzer/analyzerpro.h"

using namespace RemoteApiProtocol;

RemoteApiConnection::RemoteApiConnection(QTcpSocket* socket, MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_mainWindow(mainWindow)
{
    m_socket->setParent(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &RemoteApiConnection::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &RemoteApiConnection::onDisconnected);

    // Every open connection observes connect/disconnect, not just whichever
    // one (if any) issued the command that caused it -- matches the
    // "any client can watch, not just the one in control" design used for
    // the point-stream subscription too (added a later phase).
    connect(m_mainWindow->analyzer(), &AnalyzerPro::analyzerFound, this, &RemoteApiConnection::onAnalyzerFound);
    connect(m_mainWindow->analyzer(), &AnalyzerPro::deviceDisconnected, this, &RemoteApiConnection::onDeviceDisconnected);
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

void RemoteApiConnection::onAnalyzerFound(int /*index*/)
{
    QJsonObject fields;
    fields.insert("device", deviceStatusObject());
    sendEvent("connected", fields);
}

void RemoteApiConnection::onDeviceDisconnected()
{
    sendEvent("disconnected", QJsonObject());
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
    } else if (cmd == QLatin1String(CMD_CONNECT)) {
        QString error;
        result = cmdConnect(request, &error);
        if (!error.isEmpty()) {
            sendError(id, error);
            return;
        }
    } else if (cmd == QLatin1String(CMD_DISCONNECT)) {
        result = cmdDisconnect();
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

void RemoteApiConnection::sendEvent(const QString& eventName, const QJsonObject& fields)
{
    QJsonObject out = fields;
    out.insert(QLatin1String(KEY_EVENT_NAME), eventName);

    m_socket->write(QJsonDocument(out).toJson(QJsonDocument::Compact));
    m_socket->write("\n");
}

QJsonObject RemoteApiConnection::deviceStatusObject() const
{
    QJsonObject device;
    device.insert("name", m_mainWindow->connectedDeviceName());
    device.insert("serial", m_mainWindow->analyzer()->getSerialNumber());
    return device;
}

QJsonObject RemoteApiConnection::cmdStatus() const
{
    QJsonObject result;
    bool connected = m_mainWindow->isAnalyzerConnected();
    result.insert("connected", connected);
    result.insert("measuring", m_mainWindow->isMeasuring());
    result.insert("device", connected ? QJsonValue(deviceStatusObject()) : QJsonValue());
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

namespace {
// hid/serial/nano only -- the same set devices() enumerates (see its own
// comment on why BLE isn't part of that either).
ReDeviceInfo::InterfaceType deviceTypeFromString(const QString& type)
{
    if (type == QLatin1String("hid"))
        return ReDeviceInfo::HID;
    if (type == QLatin1String("serial"))
        return ReDeviceInfo::Serial;
    if (type == QLatin1String("nano"))
        return ReDeviceInfo::NANO;
    return ReDeviceInfo::WRONG;
}
} // namespace

QJsonObject RemoteApiConnection::cmdConnect(const QJsonObject& request, QString* error)
{
    if (m_mainWindow->isAnalyzerConnected()) {
        *error = QStringLiteral("already connected; disconnect first");
        return QJsonObject();
    }

    QString name = request.value("name").toString();
    ReDeviceInfo::InterfaceType type = deviceTypeFromString(request.value("type").toString());
    if (type == ReDeviceInfo::WRONG || name.isEmpty()) {
        *error = QStringLiteral("invalid or unsupported \"type\" (must be hid/serial/nano) or empty \"name\"");
        return QJsonObject();
    }

    // Headless: constructed with silent=true and never show()n/exec()'d.
    // connectSilent() (src/selectdevicedialog.cpp) is the same
    // UI-independent helper this class design deliberately requires --
    // see remoteapiconnection.h's own comment on why (must not depend on
    // any particular dialog/widget being open, unlike the abandoned
    // OneFqWidget UDP bridge).
    SelectDeviceDialog dlg(true, m_mainWindow);
    if (!dlg.connectSilent(static_cast<int>(type), name)) {
        *error = QStringLiteral("device not found: %1 \"%2\"").arg(request.value("type").toString(), name);
        return QJsonObject();
    }
    AnalyzerParameters* selected = AnalyzerParameters::current();
    if (selected == nullptr) {
        *error = QStringLiteral("device not found: %1 \"%2\"").arg(request.value("type").toString(), name);
        return QJsonObject();
    }

    // Mirrors MainWindow::on_selectDeviceDialog()'s accept branch exactly
    // (src/mainwindow_analyzer.cpp) -- analyzerFound is emitted manually
    // here, synchronously, rather than waited on as a separate async
    // confirmation from the device itself. By the time this function
    // returns, m_analyzerConnected/m_connectedDeviceName are already
    // updated (same-thread signal/slot), so the response below already
    // reflects real state -- the "connected" event every open connection
    // also receives (onAnalyzerFound()) is a broadcast for observers, not
    // this response's own source of truth.
    m_mainWindow->analyzer()->on_connectDevice(dlg.analyzer());
    emit m_mainWindow->analyzer()->analyzerFound(selected->index());

    QJsonObject result;
    result.insert("connected", m_mainWindow->isAnalyzerConnected());
    return result;
}

QJsonObject RemoteApiConnection::cmdDisconnect()
{
    m_mainWindow->analyzer()->on_disconnectDevice();
    QJsonObject result;
    result.insert("connected", false);
    return result;
}
