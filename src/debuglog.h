#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <QByteArray>

// Raw TX/RX logging for the analyzer interfaces (Serial/COM, USB/HID, BLE,
// NanoVNA), gated per-interface by Settings' Developer tab "Debug Logging"
// checkboxes. Deliberately session-only: the enable flags here are plain
// in-memory statics, never persisted to the ini, so logging always starts
// off and has to be turned on again each run (see the checkboxes' own
// comment in settings.cpp for why).
//
// Output is a single shared file per calendar day,
// <UserDataDir>/Debug-yyyyMMdd.log, appended to across the app's lifetime
// (and across restarts on the same day) rather than one file per
// interface -- interleaved TX/RX across all four is more useful for
// understanding what actually happened in what order than separate logs
// would be. Every entry is flushed immediately: this is a diagnostic log,
// meant to still be useful if the app crashes right after writing it.
//
// NanoVNA is its own checkbox/tag rather than folding into Serial/COM --
// it's a genuinely separate device family (its own QSerialPort, own
// ASCII shell protocol, not ComAnalyzer's binary one) even though both
// happen to ride over a QSerialPort underneath.
class DebugLog
{
public:
    static void setSerialEnabled(bool enabled);
    static void setUsbHidEnabled(bool enabled);
    static void setBleEnabled(bool enabled);
    static void setNanovnaEnabled(bool enabled);

    // BLE's once-a-second keepalive ping (BleAnalyzer::sendPing(),
    // BLE_PING_CMD) is legitimate traffic but drowns out real work in a
    // long capture. Off by itself among the four interfaces: BLE's ping is
    // a fixed single-byte marker on a self-contained packet, cleanly
    // classifiable by the caller at the same point it already logs
    // (see ble_analyzer.cpp's write()/dataReceived()). Serial/COM's ping
    // has no such marker -- its response is indistinguishable, in both
    // shape and the app's own parsing, from a genuine one-time
    // identification reply -- so it's deliberately NOT filtered: Serial
    // TX and RX are always logged in full.
    static void setBleShowPings(bool show);

    // "Report Detailed Errors" (Settings > Developer > Error Reporting &
    // Logging), same session-only/off-by-default convention as the Debug
    // Logging checkboxes above -- gates whether BleAnalyzer::setError()'s
    // (ble_analyzer.cpp) fairly technical, frequent BLE-stack-level errors
    // reach the user-facing analyzer error dialog (MainWindow::
    // onAnalyzerError()) at all. Off by default: those messages used to be
    // compiled out entirely outside a #ifdef _DEBUG build; this replaces
    // that compile-time gate with a runtime one now that they have a real
    // dialog to appear in, without making them appear for every user by
    // default. Doesn't affect the small set of always-shown messages
    // (analyzer busy/unreachable) -- see onAnalyzerError()'s own comment
    // for why those aren't considered "detailed".
    static void setDetailedErrorsEnabled(bool enabled);
    static bool detailedErrorsEnabled();

    // Raw bytes only -- no protocol interpretation. Each call is a no-op
    // (cheap early-return) unless the matching interface's logging is
    // enabled, so call sites don't need their own enabled-check. bleTx()/
    // bleRx()'s isPing lets the caller (which already knows BLE_PING_CMD)
    // mark a packet as ping traffic, suppressed when setBleShowPings(false).
    static void serialTx(const QByteArray &data);
    static void serialRx(const QByteArray &data);
    static void usbHidTx(const QByteArray &data);
    static void usbHidRx(const QByteArray &data);
    static void bleTx(const QByteArray &data, bool isPing = false);
    static void bleRx(const QByteArray &data, bool isPing = false);
    static void nanovnaTx(const QByteArray &data);
    static void nanovnaRx(const QByteArray &data);
};

#endif // DEBUGLOG_H
