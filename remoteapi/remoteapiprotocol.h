#ifndef REMOTEAPIPROTOCOL_H
#define REMOTEAPIPROTOCOL_H

// Shared NDJSON wire-protocol constants for RemoteApiServer/
// RemoteApiConnection -- one JSON object per line, both directions. See
// the json-tcp-api branch's plan doc for the full command/response
// reference; this header is just the single source of truth for the
// literal strings so command handlers (spread across remoteapiconnection.cpp
// as this grows) can't drift from each other via a typo.

namespace RemoteApiProtocol {

// Request "cmd" values (client -> server).
constexpr const char* CMD_STATUS = "status";
constexpr const char* CMD_DEVICES = "devices";
constexpr const char* CMD_CONNECT = "connect";
constexpr const char* CMD_DISCONNECT = "disconnect";

// Common JSON keys, both directions.
constexpr const char* KEY_CMD = "cmd";
constexpr const char* KEY_ID = "id";
constexpr const char* KEY_OK = "ok";
constexpr const char* KEY_ERROR = "error";
// Named KEY_EVENT_NAME, not KEY_EVENT -- windows.h's wincon.h #defines
// KEY_EVENT to 1 (a console INPUT_RECORD.EventType constant), which
// silently mangles any other identifier named exactly that anywhere it's
// transitively included (this repo's devinfo/redeviceinfo.h -> ftdi/
// ftdiinfo.h -> <windows.h> chain reaches it even from files that never
// include windows.h themselves) -- confirmed by the actual compiler error
// this produced before the rename.
constexpr const char* KEY_EVENT_NAME = "event";

} // namespace RemoteApiProtocol

#endif // REMOTEAPIPROTOCOL_H
