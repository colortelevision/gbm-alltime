# Phone keypad — newest-upload-only source delta

## Deliverables / apply
FINAL: `build-final/gbm-alltime-phone-final.gbc` (uploaded start/settings retained).
TEST: `build-test/gbm-alltime-phone-test.gbc` (separate test-only shortcut hub; not FINAL source).
Patch: `gbm-alltime-phone-patch.zip`. Optional complete FINAL project: `gbm-alltime-phone-full.zip`.

**Only apply to the newest uploaded project ZIP `b85eb9056c21770bba166264e7981ed9427a72799290465ae993b9ff8beecd80.zip`.** Older cumulative/renamesafe patches are superseded for this upload; do not overlay them. Close GB Studio, back up your project, extract inside the folder directly containing `gbm-alltime.gbsproj`, merging `project/` and `assets/`, then reopen/rebuild. This is source, not an IPS/BPS ROM patch. If ZIP/project are in Downloads on Mac:

```sh
unzip -o "$HOME/Downloads/gbm-alltime-phone-patch.zip" -d "$HOME/Downloads/gbm-alltime"
```

Adjust destination if elsewhere. If modified since upload, compare first: manifest includes BEFORE/AFTER SHA256s for all15 source files. No settings file is in the patch.

## Gameplay / controls
Pick up the existing phone in METRO-CITY (actor at tile53,35). Start opens existing MENU; select its PHONE item (tile8,16). The existing PHONE custom event now pushes one MENU snapshot and switches to **PHONE DIALER**.
- D-pad moves/wraps highlighted key; **A** picks a digit or DEL/CALL.
- **B** deletes one digit. **B on empty** cancels to MENU. **SELECT** is instant cancel.
- **START** or bottom-right **CALL** submits. Exactly7 digits required. An eighth digit is ignored with NUMBER FULL feedback.
- Incomplete: NEED7DIGITS; wrong: WRONG NUMBER; remain in dialer to edit. Neither clears an earlier successful contact.
- Correct4206967 sets existing **cinemaContacted, global59**. Original four-page phone conversation is preserved. After its A-dismissed dialogue, return to MENU; B returns to original world. Digits clear on each dialer entry, contact never resets there.
- No note prerequisite: knowledge puzzle. UI does not reveal target number. Actual dog capture awards existing **DOGNOTE77**, activating existing inventory note (tile13,16), whose original text already reads `call4206967`.

## Implementation / exact scope
- Uses11 previously unused signed16-bit globals:229 PHONE_LENGTH;230..236 PHONE_DIGIT_1..7;237 PHONE_ROW;238 PHONE_COL;239 PHONE_SELECTED_DIGIT. Only length0..7 and digit0..9 are stored; compare all7 slots independently, NEVER store4206967 in a16-bit integer. Unregistered fresh variable references were also audited for collisions.
- Original PHONE custom event ID `22fe14e5-a9c9-4690-94a2-015ff9c46700` retained; removed unused V0 argument/overflowing comparison, opens new scene. Inventory PHONE actor untouched.
- Existing Monitor female actor ID `32e01e6e-52f3-42d4-a0cd-4bc546d854c1`: init activates iff59 true; otherwise deactivates (disabled, removed from active/render/collision list, not just invisible). Existing interaction untouched. Calling from Monitor returns through MENU and reloads Monitor, immediately reevaluating gate. Added Monitor Start-to-MENU binding so MENU Pop binding cannot leak into world/underflow after return.
- Existing dog actor ID `09ec8350-b5c5-439e-b97a-f2778829ff3d`: original dialogue and77 award retained. Added a one-shot tail (localL0=0, deactivate) and previously-captured init guard; otherwise its unchanged OnUpdate condition remained1 and would reopen capture dialogue indefinitely. No other Metro actor or Metro scene event changed.
- Dialer POINTNCLICK scene has finite init/input callbacks, no OnInit loops. Hide its unused cursor only while dialing; exit shows cursor/player, clears temporary inputs, pops exactly one snapshot. MENU retains caller underneath. World B-run/A interactions/Start and title controls restored/verified.
- Locally composed5x7 pixel glyphs and minimal faceplate. Background **87 tiles**, glyph atlas48.37 independently unique mutable background cells prevent tile aliasing. Dedicated placeholders in PNG are intentionally replaced before fade; do not duplicate/merge those tiles when editing art. GB Studio Replace Tile events remain editable. No engine change.
- Initial rolling Text Draw design was rejected after actual screenshots showed tile-buffer wrapping. Delivered version uses fixed BG replacement, not rolling font buffer;100 redraws and wrong/correct screens retain exact header/key/help pixels.

## Preservation
Newest untouched snapshot: `/workspace/gbm-phone-fresh/gbm-alltime`.
Working FINAL source: `/workspace/gbm-phone-work/gbm-alltime`.
**All57 existing scene IDs/names/paths retained**, one PHONE DIALER added (58 total). All existing scene fields retained; only Monitor init gets appended inventory binding. Fresh settings/start byte-identical; FINAL still METRO-CITY as configured. **6,624 unrelated files byte-identical, zero removals.** Five existing files edited (PHONE script, variables, Monitor scene/actor, dog actor); ten new files (scene, five editable custom events, background+metadata, tileset+metadata). All111 existing variable definitions preserved, append11. Existing inventory phone/note, new Metro events, other actors, Arena/CPU/door/pit, run logic, title and assets untouched. No pushes, auth, repo operations or external calls/uploads.

## Actual runtime verification
New FINAL and TEST compiled successfully; ROM checksums verified. **149 native PyBoy assertions** (including navigation/state assertions) passed using actual key input, fresh symbols and read-only observations, NOT RAM patches. Test hub only teleports near real existing items/scenes, never sets quest flags. Save-state branches independently test knowledge-without-note and real capture-note route.
Verified: missing/six-digit/wrong numbers; cap7; B/DEL/backspace/empty cancel/SELECT; Start and on-screen CALL; repeat entry3cycles; prior contact preserved on wrong retry; actual dog capture+note display; NPC absent before/present after, including calling from inside Monitor; scene stack0world/1MENU/2dialer with no leak; visible world/cursor; B-run/release after both Metro and Monitor return; actual FINAL2x distance; title B->UNT_CURB intact.100 redraw rendered-pixel stability checks and screenshots inspected.
Evidence in `runtime-evidence/PHONE_NATIVE_RESULTS.json/.txt`; native development harness retained locally in same folder. Human GUI animation/feel and exhaustive whole-game playthrough still not claimed.

## TEST ROM route (FINAL has none of these shortcuts)
Boot PHONE TEST HUB:
- A: teleport below real dog, then A to capture; dismiss original3pages with A. Start -> MENU -> note reveals number.
- B: teleport below real phone pickup; A collects it. Start -> MENU, select PHONE.
- START: MONITOR before/after contact. Start there opens MENU normally.
- SELECT: normal METRO-CITY start vicinity. RIGHT: existing UNT_TITLE.
- SELECT from METRO-CITY/MONITOR returns test hub only. In phone SELECT still cancels to MENU; in MENU SELECT keeps original return behavior.
Use a fresh reset, not an old emulator save. Test fixture changes/settings are NOT included in the source patch/full FINAL project.

Screenshots: dialer-empty/incomplete/wrong/full/correct/stress; note-reveal; dog-capture-dialogue; monitor-before/after; title-to-story. Standalone `phone-dialer-preview.png` is a4x nearest-neighbor copy of actual empty-keypad emulator screenshot.
