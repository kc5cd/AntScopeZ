---
layout: default
title: Windows Port Audit
---

# Windows Port Audit

Porting backlog for AntScopeZ's Linux→Windows work (branch
`windows-port`). A separate team owns Linux compatibility — nothing here
is about breaking or fixing Linux.

## How to re-run this audit

Re-scan periodically as the port progresses, instead of re-deriving
findings from scratch. For each category below, search first-party code
only — skip vendored/third-party trees
(`analyzer/usbhid/hidapi/{linux,mac,windows}/`, `ftdi/*.dll|*.lib|*.h`,
`src/qcustomplot.*`):

1. POSIX headers/APIs: `pthread_*`, `<unistd.h>`, `<sys/*.h>`, `dlopen`/
   `dlsym`, `fork`/`exec*`, `mmap`, `<termios.h>`, raw BSD sockets.
2. Path handling: hardcoded `/tmp`, forward-slash path literals used as
   separators (not URLs), `getenv("HOME")`-style assumptions.
3. Linux-only logging/system facilities: `syslog.h`, `<execinfo.h>`.
4. Signal handling: `signal.h`, `sigaction`, `SIGxxx` (excluding Qt's
   `SIGNAL()`/`SLOT()` macros, which are unrelated and portable).
5. Existing platform `#ifdef`s: `Q_OS_WIN`, `Q_OS_LINUX`, `Q_OS_MAC`,
   `_WIN32`, `__linux__` — catalog what's already branched, not just
   what isn't, to tell finished scaffolding from a real gap.
6. Threading/sync primitives: confirm `QThread`/`QMutex` usage stays
   consistent; flag any new raw `std::thread`/`pthread_*`.
7. Compiler extensions: `__attribute__`, `__builtin_*`, inline asm.
8. Build-system/packaging: `CMakeLists.txt`'s `WIN32` branch staying
   current as sources/dependencies are added elsewhere; the
   `windows-mingw` `CMakePresets.json` preset still resolving the right
   Qt kit/toolchain paths; each `find_package(Qt6 6.2 REQUIRED COMPONENTS
   ...)` module actually present under the kit's `lib/cmake/`; whether a
   `windeployqt`-equivalent packaging step exists yet; native-library
   linkage (link vs. `QLibrary` dynamic-load) and its MinGW/MSVC ABI
   implications; OpenSSL/TLS necessity vs. Qt's default Schannel
   backend.

For each hit, record file:line, the construct, a suggested Windows/
Qt-portable replacement (prefer `QDir`, `QStandardPaths`,
`QLoggingCategory`, `QThread`, etc. over hand-rolled `#ifdef WIN32`
branches where Qt already solves the problem), and a rough risk/size
estimate (trivial/small/moderate/large). Append new findings under a
dated `## Scan — <date>` heading rather than overwriting prior scans, and
mark previously-listed items resolved (strike through, or move to a
"Resolved" section) instead of deleting them, so this file keeps a record
of what's actually been fixed.

## Why this reads differently than a typical POSIX-port audit

A first source-level scan (2026-08-30) found almost none of the classic
Linux-blocker categories: no raw `pthread_*`/`unistd.h`/`sys/*.h`,
`dlopen`, `fork`/`exec`, raw BSD sockets, `syslog`, POSIX signal handling,
or GCC-specific extensions anywhere in first-party code. Threading and
networking already go through `QThread`/`QMutex` and
`QUdpSocket`/`QTcpSocket` throughout. Existing `Q_OS_WIN`/`Q_OS_LINUX`/
`Q_OS_MAC` scaffolding already covers most platform differences (config/
data paths via `QStandardPaths`, `.asd` file association via the Windows
registry, `WM_DEVICECHANGE` device-change notifications, a fully
implemented FTDI driver path via `QLibrary`). So this backlog is weighted
toward **build-system and packaging gaps**, and a **verification
checklist** for existing-but-never-actually-tested Windows code — not a
sweep for POSIX API replacements, because that sweep came up mostly
empty. `BUILDINFO.md` states outright: *"This project has not been tested
on Windows."*

## Scan — 2026-08-30

### Build system / packaging

- **`CMakePresets.json` had no Windows/MinGW preset.** Fixed this
  session: added `windows-mingw` (native build against the Qt Online
  Installer's Qt 6.11.2 MinGW 13.1.0 kit under `C:\Qt`, using Ninja).
  Risk/size: small, mechanical addition — didn't touch the existing
  Linux/macOS presets.
- **Qt 6.11.2 MinGW kit was missing `Qt6SerialPort` and `Qt6Bluetooth`**,
  both required by `find_package(Qt6 6.2 REQUIRED COMPONENTS ...)`
  (`CMakeLists.txt`). Installed via `C:\Qt\MaintenanceTool.exe install
  qt.qt6.6112.addons.qtserialport qt.qt6.6112.addons.qtconnectivity`
  (Bluetooth ships under the "Connectivity" package, per `BUILDINFO.md`'s
  own note that Debian packages it as `qt6-connectivity-dev`). Resolved
  this session; noting the exact package IDs here since they aren't
  obvious from the display names in the Maintenance Tool GUI. Risk/size:
  trivial once you know the package name.
- **No `windeployqt` (or equivalent) step exists for Windows packaging —
  confirmed blocking, not just theoretical.** `CMakeLists.txt`'s
  install-rules section only installs license/doc files flat into
  `CMAKE_INSTALL_BINDIR` for `WIN32` (~line 783, ~827); the actual
  Qt-bundling logic (`qt6_generate_deploy_script()`, ~line 845 onward) is
  gated `elseif(NOT APPLE)`, i.e. Linux-only. `THIRD-PARTY-LICENSES.md`
  independently flags this same gap. **Confirmed 2026-08-30:** running
  the freshly-built `AntScopeZ.exe` directly fails with "the code
  execution cannot proceed because Qt6Core.dll/Qt6Gui.dll/
  Qt6Network.dll/Qt6Bluetooth.dll was not found" — exactly as predicted.
  **Not fixed yet** — needs a `WIN32` branch mirroring the Linux
  deploy-script pattern, or a `windeployqt` post-build/install step.
  Risk/size: moderate — this is packaging, not source, but touches the
  shared install-rules section; check in before implementing per the
  structural change threshold in `CLAUDE.md`.
- **OpenSSL linkage on Windows — investigated 2026-08-30, resolved: not
  needed.** `app/libeay32.dll`/`app/ssleay32.dll` (legacy OpenSSL 1.0.x)
  sit in the tree unreferenced by any CMake rule, leftover from the old
  qmake build. `CMakeLists.txt`'s `WIN32` block has a standing comment
  that the old `.pro` file's `win64`-scope OpenSSL link flags were
  deliberately not carried over, "add them only once it is confirmed
  Windows needs them." Findings:
  - `analyzer/updater/downloader.cpp`'s `QSslConfiguration`/HTTPS code is
    entirely `#if 0`'d out (see that file's own comment: disabled because
    it phones home to RigExpert with device/OS telemetry, and its only
    caller is also disabled) — not a live network path.
  - `src/licenseagent.cpp` **does** make live HTTPS requests via
    `QNetworkAccessManager`/`QSslConfiguration` (license verification) —
    a real, active TLS consumer, unlike `downloader.cpp`.
  - Per Qt's own docs (`ssl.html`, verified against the Qt 6.11
    reference): Qt Online Installer builds for Windows ship a native
    Schannel TLS backend by default — OpenSSL is only linked in if a
    *source* build of Qt explicitly opts in with `-openssl-linked`, which
    the prebuilt installer kit does not. Confirmed on this machine: `C:\Qt\
    6.11.2\mingw_64\plugins\tls\` has both `qopensslbackend.dll` and
    `qschannelbackend.dll`, but no OpenSSL 3.x runtime DLLs
    (`libssl-3-x64.dll`/`libcrypto-3-x64.dll`) exist anywhere in the Qt
    install. Qt tries backends in order (OpenSSL first) and falls through
    to the next on load failure — so with no OpenSSL 3.x DLLs present,
    it silently falls through to Schannel, which works standalone.
  - `app/libeay32.dll`/`ssleay32.dll` are OpenSSL **1.0.x** (pre-3.0
    naming scheme) — even if something did try to load them, Qt 6.11's
    OpenSSL backend requires OpenSSL 3 at runtime, so these specific
    files couldn't satisfy it regardless. They're not just unreferenced,
    they're the wrong major version.
  - **Conclusion: no OpenSSL linkage needed.** `licenseagent.cpp`'s HTTPS
    calls will work via Schannel with zero extra packaging. `app/
    libeay32.dll`/`ssleay32.dll` are dead weight from the old build and
    can be deleted — small, mechanical cleanup, safe to do without
    check-in.
- **First native build baseline (this session):** see the
  `## Build baseline` section below once the first `windows-mingw`
  build finishes.

### Native library linkage

- `ftdi/ftdiinfo.cpp` loads `ftd2xx.dll` **dynamically via `QLibrary`**
  and resolves `FT_*` entry points at runtime — it does not link against
  `ftdi/windows/win32|win64/ftd2xx.lib`. No MinGW/MSVC import-library ABI
  concern for this path. `ftdi/windows/*/ftd2xx.lib` appear unused;
  worth confirming nothing else references them before considering
  removal (out of scope for this pass).
- `hidapi` Windows backend (`analyzer/usbhid/hidapi/windows/hid.c`) is
  already selected by `CMakeLists.txt`'s `if(WIN32)` block and links
  `setupapi` directly — no dynamic-load path here, so this one does need
  its MinGW build actually verified (see verification checklist below).

### Verification checklist — existing Windows code, never actually run

None of the following have been executed/tested on Windows before this
port. Each needs a real run, not just a clean compile:

- [ ] `.ico` resource / `WIN32_EXECUTABLE` — icon shows correctly on the
      built `.exe`.
- [ ] `.asd` file-association registration via the Windows registry
      (`src/mainwindow.cpp:606`, `QSettings::NativeFormat` +
      `"HKEY_CLASSES_ROOT"`) — actually registers and double-clicking a
      `.asd` file opens AntScopeZ.
- [ ] `WM_DEVICECHANGE` device-change notifications (`src/main.cpp`,
      `<dbt.h>`) — connecting/disconnecting a real analyzer is detected.
- [ ] hidapi Windows backend + `setupapi` — a real RigExpert unit is
      enumerated and communicates correctly over HID.
- [ ] FTDI dynamic load via `QLibrary` against the bundled
      `ftdi/amd64/ftd2xx.dll` — resolves and works for FTDI-based units.
- [ ] Config/data path resolution via `QStandardPaths` on Windows
      (`src/settings.cpp`, six `Q_OS_WIN`/`Q_OS_LINUX` pairs) — settings
      persist to the expected per-user location.

### Out of scope / noted only

- `ftdi/ftdiinfo.cpp`'s `#else` (non-Windows/non-Mac) branch is a stub
  that returns an empty device list — i.e. Linux-side FTDI detection
  isn't implemented. This is the **opposite** of what you'd expect from
  an app "originally" targeting Linux, but it's explicitly the other
  team's problem (Linux compatibility), not this port's. Noted here only
  so it isn't later mistaken for a Windows gap.

## Build baseline

First `windows-mingw` configure+build attempt (2026-08-30, Debug config,
after installing the missing Qt Serial Port/Bluetooth components above):
**succeeded cleanly, zero compiler warnings**, all 93 translation units.
`AntScopeZ.exe` linked successfully; the `if(WIN32)` post-build step
correctly copied `ftdi/amd64/ftd2xx.dll` next to it.

This is a compile/link-only baseline — the resulting `.exe` has **not**
been run yet (no `windeployqt` step means it likely can't find Qt's DLLs
without the Qt `bin` directory on `PATH`; see the packaging gap above),
and none of the "Verification checklist" items above have been exercised.
Don't read a clean build as "the port works" — it means the source
compiles against MinGW without issue, which given how little
Linux-specific code existed in the first place (see the top of this
document) isn't a large surprise. The real unknowns are all in the
checklist and the packaging gap, not the compile step.
