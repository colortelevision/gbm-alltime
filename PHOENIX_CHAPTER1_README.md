# THE PHOENIX — Chapter 1 / GBM, November 8, 2026

A playable local chapter adaptation, not a website deployment. No repository operations were used.

## Exact baseline — important

Apply this chapter's patch ONLY to the exact full Gridduel delivery:
`GBM-Gridduel-7x7-FULL-PROJECT-Nov07.zip`

Archive SHA-256:
`fa53b469181c7c387f7cb9ce83aed1b153f6af5682a778c828a6284a3d5ba605`

The extracted project has 6,691 files. Its canonical sorted path→SHA256 manifest fingerprint is:
`beb750d4ff0f9d86079ceec461b262900a348c69ad815bf51ac7e60e6f8b5380`

The installer verifies every baseline file and every payload file before writing; unexpected extra project files are refused too. It refuses a raw earlier upload, a phone/CPU-only project, missing Gridduel changes, or edited baseline files. It is idempotent. Use a copy; do not overlay older cumulative patches. Point it at the directory containing `gbm-alltime.gbsproj`, not at a ROM folder. A guarded full project is supplied as an alternative.

Mac example after unzipping the patch in Downloads (adjust only the project directory):

```sh
python3 "$HOME/Downloads/GBM-Phoenix-Chapter1-PATCH-Nov08/apply_patch.py" "$HOME/Downloads/gbm-alltime" --check
python3 "$HOME/Downloads/GBM-Phoenix-Chapter1-PATCH-Nov08/apply_patch.py" "$HOME/Downloads/gbm-alltime"
```

Original changed files are backed up to a new sibling directory before installation. Hash mismatches cause a refusal, not a forced merge.

## ROMs / entry

- FINAL retains the original Metro start/settings. No debug shortcut or fixture is added to FINAL.
- TEST starts at the chapter's existing `UNT_TITLE`. Press A, then A to advance the begin prompt.
- In FINAL, the existing route is Metro → B-ILLUSTRATED → 1F → bottom exit → BIOS → Advanced → Arena → far-right doorway → chapter title. BIOS begins at L0=0: RIGHT twice selects Advanced (L0=2), then A.
- Exact tested short route from the fresh Metro position (tile coordinates): `(6,15) → (6,27) → (10,27)`, UP into B-ILLUSTRATED. Allow its existing walk-in sequence/dialogue to finish. Walk toward `(21,34)` and the bottom exit into BIOS. In Arena, move right and jump over gaps; the far-right door is the existing rectangle x29–31/y12–15. Winning Arena is not required. The chapter title's B returns to safe Metro `(54,27)` with an empty scene stack.
- The opening card deliberately remains UNTITLED / CHAPTER 1; the Phoenix name/reveal is held for the ending.

## Controls / chapter route

- Exploration: D-pad moves, A talks/examines, hold B runs. START gives contextual hints in the main exploration rooms; this chapter does not open the main-world inventory.
- Beach: talk to Conner/Ami if desired; walk east into the ocean. The red moon and rain are visible in the fixed beach view.
- Ocean: D-pad swims, tap A for a short stronger stroke. B does not enable land running. Follow the voice north to the boat.
- Bedroom: Conner's wake-up splash is staged through his visible actor, sound and dialogue. Leave through the bottom door.
- Chase: D-pad moves, A jumps. Follow Conner; jump/vault over Parker's collision-backed car and reach the bus at the right. Scoped jump settings are restored on boarding.
- Classroom: optional band/crush/student conversations; talk to Maxwell near the front, then leave at the bottom. In the hall take the right-hand door.
- Church: central aisle then the open right-side aisle leads to the stairs. In the cellar approach the cross. A brief escape window lets you move; it cannot become a consent choice or successful escape.
- Conner: examine the room if desired, leave at the bottom, approach Parker, then run east to the church.
- Parker: D-pad moves, A attacks, B dodges, hold SELECT defends. Hearts are Parker's health; diamonds are stamina. In-range, correctly faced punches stagger/interrupt Maxwell's wind-up. Dodge grants a short step/invulnerability; guard spends stamina to block a strike. Attacks cannot kill her. Idle, guarding, active play and repeated well-timed close punches all end in defeat; skill changes only the timing (hard story finisher at native frame1080).
- Conner rescue: reach Matt, press A near the bindings to try pulling. Sound/camera feedback acknowledges the attempt. Repeated attempts and doing nothing have the same fixed failed outcome.
- After the rescue window, death, the musical cut, silence, Phoenix reveal, rebirth and the complete card are automatic. No button/score/secret victory triggers resurrection.
- Complete card: A returns to title, B returns safely to Metro. Replaying clears only chapter-local journey state; the completion flag and unrelated world progress are not reset.

## Implementation / editing

The eight original UNT scene IDs, names and paths are retained and reworked. Eight additive scenes link the beach, ocean, platform chase, boss, rescue and ending cards. Original unrelated scenes, character resources, run/menu/phone/Arena/Gridduel logic and settings are preserved byte-for-byte. Superseded chapter actor/trigger scripts are disabled in place rather than deleting their identities. Legacy chapter custom-event definitions are retained but no longer drive this route.

Story/dialogue/cutscenes/transitions are ordinary editable GB Studio events. New PNGs and `.gbsres` sprite states are editable. Maxwell has a derived silver ponytail/glasses sheet, distinct from the untouched shared janitor asset. Character palettes follow Matt brown/white, Conner blond/red, Ami orange/cap, Parker black-haired, Maxwell silver. GB's three visible sprite colors mean some skin and clothing tones must be shared.

`plugins/Phoenix/events/eventPhoenix.js` connects the small additive banked `assets/engine/src/core/phoenix.c` controller. No existing engine implementation or memory limit was replaced. It handles swimming strokes, bounded escape/rescue timers and Parker's timed fight; all narrative remains in scene events. Its small state borrows compiler-unallocated VM heap after `MAX_GLOBAL_VARS`, guarded at compile time, and is mutually exclusive with Gridduel's existing scratch use. Do not launch both native minigames simultaneously.

New variables243–259 are PH_STAGE, PH_MODE, PH_RESULT, PH_ACTION, PH_HP, PH_STAM, PH_HUD, PH_FOE, PH_TELL, PH_CLOCK, PH_ATTACKS, PH_DODGES, PH_GUARDS, PH_PULLS, PH_DONE, PH_VISIT, PH_INVITED. Existing inventory/quest variables are not reused. No real telephone service is involved.

Rebuild using GB Studio4.3 with the supplied project engine/plugins, as for the Gridduel baseline. No art-generation scripts or external service are required to rebuild.

## Coverage and honest limits

Actual PyBoy inputs cover the complete TEST chapter and FINAL's physical Metro/phone/Gridduel/shop/BIOS/Arena entry plus the complete chapter. Tests exercise swimming, actual platform jumping, switches, failed escape, idle/guarded/active/skilled fixed boss outcomes, visible rescue attempts, no-input finale, actual silent audio samples, end-card return, B-run release and replay. FINAL's phone pickup, incomplete/wrong/correct dialing, backspace, inventory return and Gridduel entry/quit are exercised before the chapter; phone/contact progress is checked afterward. Two complete normal Gridduel matches pass: one fresh and one after chapter completion, verifying shared scratch initialization in both directions. Monitor contact gating also passes before/after the correct call. The supplied four runtime suites pass all 193 assertions (TEST41, FINAL83, fresh Gridduel35, post-chapter Gridduel34). Test checkpoints are real-input emulator save states, not RAM-variable fabrication.

This is a compact first playable adaptation. Dialogue is concise authored dialogue, not a claim of a verbatim screenplay. Classroom background students reuse placeholder silhouettes. Maps, rain, props, Maxwell, combat poses and Phoenix/rebirth cards are simple editable derived/placeholder pixel art; the reveal is a timed silhouette/card sequence, not elaborate animation. Swimming is top-down water traversal with a stroke boost, not a separate underwater physics simulation. The chase is one short platform lane. Balance and subjective pacing still need manual handheld/GUI playtesting.

There is no mid-chapter save/continue UI. Re-entry and PH_DONE persistence across ordinary scene transitions/replay are tested, not power-cycle restoration of a chapter checkpoint. Do not reuse old ROM save states or assume older SRAM compatibility: adding/removing used script globals changes GB Studio's compiled save layout. Keep old saves backed up and start these newly named ROMs fresh. No automatic world save, permanent rewards, ante, mythology explanation, early Matt fire ability, hidden boss victory or player-triggered resurrection is added.
