---
layout: default
title: Use Case Catalog
---

# AntScopeZ use-case catalog (working doc)

Not end-user documentation -- that's `docs/user-guide.md`, organized by
*feature*. This is organized by *goal* ("I want to accomplish X"),
specifically to drive feature planning: for each real thing a user
might be trying to do, what's the procedure today, and does AntScopeZ
actually cover it yet. Pulled together 2026-08-19 as prep work before
planning S22 exposure, so gaps get spotted deliberately instead of
being discovered one bug report at a time.

Status legend: ✅ works today -- ⚠️ partially/awkwardly -- ❌ not
possible today.

## Antenna / one-port measurement

| # | Use case | How today | Charts/tools | Status |
|---|---|---|---|---|
| 1 | Check SWR/return loss at my operating frequency | Set Start/Stop, Single scan, read the dip/marker | SWR, RL | ✅ |
| 2 | Find an antenna's actual resonant frequency | Read the SWR dip, or where X crosses zero | SWR, Z=R+jX | ✅ |
| 3 | Trim an antenna to a target frequency | Place Current + Target markers, read suggested trim | Tools > Marker Comparison | ✅ |
| 4 | Design a matching network for a mismatched antenna | Read R/X (series) or Rp/Xp (parallel) at target frequency, or trace shape on Smith | Z=R+jX, Z=R‖jX, Smith | ✅ |
| 5 | Determine antenna Q / usable bandwidth | Marker Comparison's Q field, or eyeball Phase-crossing steepness | Phase, Marker Comparison | ✅ |
| 6 | Compare two antennas/configurations side by side | Check multiple Measurements rows visible on one chart | any chart, Multi | ✅ |
| 7 | Watch a match improve in real time while physically adjusting it | Continuous scan mode | SWR (or any chart) | ✅ |
| 8 | Log/export a measurement for a report or later reference | Export (CSV/NWL/Touchstone), Save (.asd), Print, Save Screenshot | File menu | ✅ |
| 9 | Retrieve scans already stored on the analyzer itself | Data from AA | File > Data from AA | ✅ |
| 10 | Correct for the analyzer's own measurement error | OSL calibration, then check the Calibration box | Settings > OSL Calibration | ✅ |
| 11 | Target scans to a specific ham band quickly | Band selector fills Start/Stop | Presets panel | ✅ |
| 12 | Find a cable fault or verify cable integrity | Run a TDR scan, read the reflection (distance + impedance in Ohms, not just open/short) | TDR, Tools > TDR Measurement | ✅ distance/open-short reliable; ⚠️ the Ohms figure is a rough estimate, not precision -- confirmed 2026-08-21 against a real open 13ft cable (101Ω, then 771Ω after a real bug fix, still not fully convincing) -- real cable loss, the settling-point calculation, and window-function choice all affect the number independently. See [[tdr-scan-rework-plan]]. |
| 13 | Determine an unknown cable's length or velocity factor | Reverse-solve calculator (known length -> velocity factor) | Tools > TDR Measurement | ✅ |
| 14 | Figure out if bad SWR is the antenna or the feedline | Enter the known feedline length -- automatically flags "peak is short of that length" (possible fault partway along) vs. it just being the far end | Tools > TDR Measurement | ✅ (automated 2026-08-21 -- was a manual eyeball comparison before) |
| 15 | Share/document a chart as an image or PDF | Print, Save Screenshot, Screenshot from AA | File menu | ✅ |
| 16 | Overlay markers directly on the TDR (distance) chart | -- | -- | ❌ deferred -- see [[tdr-analysis-roadmap-deferred]] |
| 17 | Pick a TDR window function (Hamming/Hann/etc.) instead of the fixed default | Window combobox (Rectangular/Hamming/Hann/Blackman/Kaiser), live re-plot, no rescan | Tools > TDR Measurement | ✅ (shipped 2026-08-21) |
| 39 | See every significant reflection in a scan, not just the single strongest one (e.g. two connectors plus the far end) | -- | -- | ❌ feature suggestion, 2026-08-21, not scoped -- needs real peak-finding (local maxima above the noise floor), not just a global max. See [[tdr-scan-rework-plan]]. |
| 40 | Compare a TDR scan against an earlier one of the same cable (did a repair actually change anything) | Select multiple Measurements rows, eyeball both traces on the same chart | TDR (multi-select already works) | ⚠️ possible today by overlaying rows manually; no dedicated before/after diff (distance/amplitude deltas called out explicitly). Feature suggestion, 2026-08-21, not scoped -- would need a second-measurement picker UI. See [[tdr-scan-rework-plan]]. |
| 18 | Type "6ft"/"1.5M" into a field instead of the field's fixed unit | -- | -- | ❌ planned, not started (see the unit-shorthand-input plan) |
| 29 | Scan a range wider than one sweep/device supports (segmented/stitched sweep) | Settings > General: set "Analyzer maximum number of points" to the device's real limit; requesting more points than that transparently splits the scan into several sequential sweeps and concatenates them into one continuous dataset | any 1-port chart | ✅ (shipped -- `g_analyzerMaxPoints`, `AnalyzerPro::buildStitchSegments()`; confirmed 2026-08-18 against a real RigExpert Match at 1000/2000/10000 points) |
| 36 | Watch SWR/Z settle at one specific frequency live, without a full sweep redrawing around it | Set Start=Stop (or Range=0), Single or Continuous | Smith chart tracer + a floating readout box | ✅ (undocumented until 2026-08-20 despite being reachable all along; three real bugs -- a crash, a silent restart loop, disabled panel buttons -- found and fixed the same day) |
| 37 | Get a large, glanceable SWR readout visible from a distance (e.g. up a ladder, tweaking an antenna, reading the screen across a yard) | -- | -- | ❌ row 36's floating box is small and packs 11 technical fields into it -- not remotely readable at a distance. A `OneFqDisplayStyle::BigReadout` stub exists (`onefqwidget.h`) but nothing renders it yet. This is the actual use case a "tuning aid" needs; row 36 doesn't serve it despite being adjacent. |

## 2-port / transmission measurement (`.s2p` import only -- no live 2-port hardware owned)

| # | Use case | How today | Charts/tools | Status |
|---|---|---|---|---|
| 19 | Verify a filter/attenuator's passband or insertion loss vs frequency | Import a `.s2p`, read S21 magnitude | S21 tab | ✅ (shipped 2026-08-19) |
| 20 | Sanity-check S21≈S12 (reciprocity) for a passive device | Both traced on the same S21 tab (dashed/solid) | S21 tab | ✅ |
| 21 | Track a frequency point's S21/S12 across the Markers table / cursor hover | Marker Comparison table columns, Cursor Details | S21 tab, Markers | ✅ (shipped 2026-08-19) |
| 22 | Check the DUT's *output*-port match (S22) | -- | -- | ❌ deferred pending live 2-port hardware -- same gap as row 28, no live-capture device to build/test S22 against |
| 23 | Sanity-check a device is electrically symmetric (S11≈S22) | -- | -- | ❌ same gap as above |
| 24 | Design a matching network for a 2-port device's output side | -- | -- | ❌ same gap |
| 25 | Read group delay / phase linearity through a device | -- | -- | ❌ explicitly deferred in the 2-port plan |
| 26 | Compare S21 alongside another chart in Multi view | -- | -- | ❌ S21 is explicitly excluded from Multi's join menu (`mainwindow_multitab.cpp`) |
| 27 | Export imported 2-port data back out (S11/S21/S12/S22), in RI, MA, or DB | Export dialog's S2P RI/MA/DB buttons (only shown for a 2-port measurement) | File > Export | ✅ (shipped 2026-08-19, `exportSParamData()`) |
| 28 | Capture 2-port data live from real hardware | -- | -- | ❌ dead end on this user's hardware -- `FDB` (S21) *and* `EFRX` (User Defined tab) both return "Error.Not recognized" on the only device tested (RigExpert Match RFE, confirmed 2026-08-19/20). See [[s21-and-user-defined-live-capture-deferred]]. (Row 29's stitching should extend here once hardware unblocks this -- `on_measureUser()`/EFRX already routes through `startStitchedMeasure()` same as a normal scan; `on_measureS21()`/FDB doesn't yet, calls `startMeasure()` directly. Worth fixing when this gets picked back up.) |
| 35 | Import a 2-port *Z-parameter* Touchstone file (`# ... Z RI ...`, 9-value rows) correctly | -- | -- | ❌ real gap found 2026-08-19 -- Z21/Z12/Z22 are a different quantity than S21/S12/S22 (would need actual Z-to-S 2-port matrix conversion); guarded against silent mislabeling (skipped, not shown as wrong data) rather than fixed. S-parameter 2-port files (the overwhelmingly common case) are unaffected. |
| 34 | Tell, at a glance, which Measurements-list rows are 2-port (have real S21/S12/S22) vs. plain 1-port | Points column now reads e.g. "100 (s1p)"/"300 (s2p)" | Measurements panel | ✅ (shipped 2026-08-19; column-width formatting for large point counts still to be revisited) |

## App reliability / diagnostics

| # | Use case | How today | Charts/tools | Status |
|---|---|---|---|---|
| 30 | Understand why a scan is slow or appears hung | -- | -- | ❌ watchdog root-caused but no UI yet |
| 31 | Reliable auto-reconnect after a dropped/power-cycled analyzer | -- | -- | ⚠️ known gap, tracked alongside the deferred 2-port hardware work |
| 38 | Get remote help from a more experienced ham -- let them drive your analyzer software over the network while you handle the physical antenna/hardware end | -- | -- | ❌ parked idea (2026-08-20), not scoped or designed -- see [[remote-network-control-idea]]. Would supersede, not extend, an existing but abandoned narrow UDP bridge (see [[s21-and-user-defined-live-capture-deferred]]-adjacent findings in `BUILDINFO.md`). |

## Visual / UI

| # | Use case | How today | Charts/tools | Status |
|---|---|---|---|---|
| 32 | Tell overlapping measurement traces apart at a glance | `getColor()` palette | any multi-trace chart | ⚠️ known collision bug, task #11 -- "not ready to fix it" |
| 33 | Keep the Markers table visible without it floating awkwardly over the chart | -- | -- | ⚠️ unsolved placement problem (parked, not forgotten) |

## What this surfaced for S22 specifically (2026-08-19)

Rows 22-24 are the actual gap this doc was written to clarify before
planning. Nothing lets a user look at the output port of a 2-port
device at all -- not a "clunky but possible" workaround, a flat ❌. That
confirms the earlier use-case reasoning (verify output match, check
reciprocity, design an output-side matching network) was the right set
to design against, and that it's genuinely new surface, not a rough
edge on something that already half-works.

Row 27 (2-port export) and row 26 (Multi view) are adjacent gaps worth
keeping in view while shaping S22's design, even though they're not the
immediate target -- e.g. whatever data model S22 uses should be export-
and Multi-view-compatible later without another rework.

Row 34 (spotted 2026-08-19, reviewing this doc) turned out to be a real
prerequisite for row 27, not just adjacent to it -- both shipped the same
session: the Points-column tag (row 34) is what `Export::updateDetails()`
reads to decide whether to show the S2P button at all (row 27). Neither
needed S22 to exist first -- `dataSParam` already had everything
`exportSParamData()` needed, straight passthrough, no fabrication.

## What actually happened next (2026-08-20)

S22 itself wasn't picked up. The next session went a different direction
entirely -- an audit of everything gated behind the `-developer`
command-line flag (`g_developerMode`), triggered by investigating a User
Defined tab crash. That surfaced rows 28, 36, 37, and 38 above, plus a
`.deb`-user-reachable "One Fq" bug fix (row 36) that had nothing to do
with 2-port work at all. Full technical detail lives in `BUILDINFO.md`'s
"Compile-time feature gates" section, not duplicated here. S22 (rows
22-24) remains exactly as much of a gap as it was on 2026-08-19 --
nothing below touched it, this is just an honest record that the very
next session's actual priority turned out to be something this doc
didn't anticipate.

## What actually happened next (2026-08-21)

S22 still wasn't picked up -- this session went to TDR instead, starting
from row 17 (window function, deferred) and growing into a full rework:
a dedicated `Tools > TDR Measurement` dialog (`TdrScanPanel`/
`TdrScanDialog`) replacing the old tab-implicit "switch to TDR, click
Single" trigger; a live unambiguous-range/resolution preview before
scanning; the window-function picker itself (row 17, shipped); and --
found along the way, not planned -- `TDRAnalysisDialog` merged into the
same dialog (its own `Tools > TDR Analysis` menu entry retired), an
automatic fault-vs-known-cable-length flag (row 14), and an impedance
(Ohms) readout (row 12) whose accuracy turned out to be a real, still-open
question rather than a quick win. Full history, including two confirmed
bugs found via real hardware testing (a scan-trigger reentrancy hang, and
the impedance reading itself), in the `tdr-scan-rework-plan` memory --
not duplicated here. Continuous mode was added for TDR mid-session, then
deliberately removed again -- every bug found was Continuous-specific, and
TDR's real use cases never needed it (see the same memory).
