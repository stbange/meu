# Local Journey Saves, Format 1

M0e uses the same portable binary snapshot on Windows, Linux and the web. Native and browser adapters publish it through their own durable storage APIs. Save is explicit; changing settings, closing menus, backgrounding or quitting does not silently save.

## Player controls

On the web, pause and open Saved journeys. Save journey includes settings. A supported parked unit and a settled lift are required; close and resume to let braking or a lift ride finish, then save. Load latest and Recover previous leave the game paused. Export downloads a portable .meu file; Import validates and replaces only the paused running journey. Save explicitly after importing to keep it in this browser/device.

On native desktop, F2 opens/closes Saved journeys. F5 saves, F9 loads latest, F8 recovers previous, F6 exports through the file picker and F7 imports. Closing the menu leaves the game paused; Enter resumes. A valid latest save restores automatically at startup, without initializing sound or advancing simulation until that explicit gesture.

## Record layout

Schema 2 records are exactly 256 bytes. Legacy schema 1 records of 128 bytes remain readable; migration initializes power to safe defaults and the original source remains unchanged. Integers and IEEE 754 binary32/binary64 numbers are encoded explicitly in little-endian order; C struct layout/padding is never written. Format/schema version and authored-world compatibility are separate from the runtime build/content version. World ID 1 names the current authored home/causeway geometry. A future incompatible layout must change the world compatibility policy rather than silently loading into unrelated geometry.

| Byte offset | Field |
| --- | --- |
| 0–7 | Magic: MEUSAVE followed by a zero byte |
| 8 | u32 format/schema, currently 2 |
| 12 | u32 authored-world compatibility ID, currently 2 |
| 16 | Nonzero u32 generation, local to the journal |
| 20 | u32 record size, 256 |
| 24, 32, 40 | f64 robot world x, z and yaw |
| 48, 52, 56, 60, 64 | f32 Commander robot-local x, y, z, yaw and pitch |
| 68 | f32 settled lift height |
| 72, 76 | u32 lift landing and current interior room |
| 80 | u64 simulation ticks (bounded to JavaScript's exact-integer range) |
| 88 | u32 visited-space bit mask |
| 92 | u32 flags: quarters light 1, inspection light 2, invert drag 4, walking bob 8, machine sway 16, parked 32 |
| 96, 100 | f32 sensitivity and volume |
| 104–123 | Reserved zero bytes |
| 124–127 | CRC-32 over bytes 0–123 |
| 128–147 | u32 main energy, heat, daylight clock, subsecond phase, power flags |
| 148–251 | reserved for expedition systems; zero until defined |
| 252–255 | CRC-32 over bytes 0–251 |

The checksum detects accidental damage. Geometry and semantic checks remain independent: reject unsupported schema/world, non-finite or out-of-range values, inconsistent room/lift metadata, reserved flags, solid intersections, unsupported player footing and invalid robot hull/foot support. A staged decode touches neither output nor live state on failure. The schema-2 reference `tests/fixtures/save-v2.meu` is checked byte-for-byte; `save-v1.meu` is retained for legacy migration in native and WebAssembly tests. It has generation 7 and tick 12345.

Only a paused live state can be replaced. A restored journey uses the authored geometry, zero travel speed/turn rate, parked state, cleared helm/held input, reset interpolation/accumulators and explicit resume. Current menu ownership is preserved so import/recovery cannot restart gameplay behind a dialog. Camera bob/sway phase is cosmetic and restarts; physical player coordinates do not change.

## Native journal

The normal directory comes from `SDL_GetPrefPath("MEU", "MechanizedExplorationUnit")`, independent of other games. Typical locations are under the user's AppData/Roaming on Windows or XDG data directory on Linux. The log prints the exact path. `--save-dir <directory>` selects a separate directory; automated rendered replays use `build/native/persistence-replay`.

The journal has latest.meu and previous.meu. Saving takes an inter-process lock, reads and validates existing generations, encodes a candidate with a new generation, preserves malformed source files as damaged-latest.meu/damaged-previous.meu, and keeps the last valid latest as previous. Each replacement writes a pending file, flushes it, publishes within the same directory and confirms the platform's durability operation. Windows uses UTF-8-to-wide file APIs, FlushFileBuffers and write-through replacement; Linux uses fsync on the file and containing directory. Success is reported only after these steps return successfully.

An interrupted save may leave both slots holding the last complete generation. That is valid recovery data; a pending file never loads as a committed save. If an error occurs after replacement but before durability confirmation, no Saved message is shown and the prior confirmed generation remains in the other slot. OS/process locks release when the process exits. The most recent malformed source from each slot is retained separately when the user explicitly saves a repair; those diagnostic slots are replaced only by a later malformed source from that slot.

File dialogs can complete on another thread. Their callback queues the selected UTF-8 path back to the SDL main loop; decoding and live-state replacement occur there. Native tests inject write/flush/replace failures and an interruption after the backup, plus failure after quarantining damaged latest data. Separate rendered processes save, restore at startup, export, change position, import, and load again.

## Browser journal

Database: meu-local-saves, database version 1, object store snapshots. The keys are latest, previous, damaged-latest and damaged-previous. Snapshot values are canonical 512-character hex encodings of the same 256 bytes (legacy 256-character encodings are accepted). The host checks record length before passing a string to WebAssembly; the C decoder also bounds parsing. Stored source data can be retained/exported even if it fails that format.

One strict-durability IndexedDB read/write transaction reads the old generations, validates/captures the candidate, rotates previous, retains damaged source records and publishes latest. There is no asynchronous gap between generation inspection and queuing writes within that transaction. Concurrent tabs are serialized by IndexedDB. Only `transaction.oncomplete` produces Saved; request/put success does not. Abort/quota/storage denial reports failure and leaves the previously committed transaction intact. Navigation can abort or complete a pending transaction; either outcome must preserve a complete confirmed generation rather than partial journal updates.

A valid latest record restores into pause on startup. If latest is damaged or missing, the player chooses Recover previous. Recovery itself never rewrites the stored source. An explicit later save preserves the most recent damaged data before repairing latest. Import does not publish or change local generations; it remains possible when IndexedDB is unavailable. Export reports download start rather than claiming a durable local save. Browser profile/origin storage is separate from native saves and other browsers; exporting provides a separate copy.

Storage completion is the platform confirmation boundary, not a claim that browser data cannot be cleared or evicted. There is no cloud account, background save or network synchronization. Tests cover quota/denial, put success before transaction completion, page interruption, corrupt/latest recovery, oversized damaged records, incompatible imports and two-tab publication through real IndexedDB operations.
