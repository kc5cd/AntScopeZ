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

// Common JSON keys, both directions.
constexpr const char* KEY_CMD = "cmd";
constexpr const char* KEY_ID = "id";
constexpr const char* KEY_OK = "ok";
constexpr const char* KEY_ERROR = "error";
constexpr const char* KEY_EVENT = "event";

} // namespace RemoteApiProtocol

#endif // REMOTEAPIPROTOCOL_H
