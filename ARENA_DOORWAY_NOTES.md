# LATEST: CPU opponent
See ARENA_CPU_NOTES.md and TEST_ROM_RUNBOOK.md. Older contact-auto-KO/B-only details below are superseded by CPU combat; doorway/pit geometry and stack fixes remain.

# Arena doorway -> existing UNTITLED title (latest cumulative delivery)

## Changes / evidence
- Existing `UNT_TITLE` resource ID **903be6a4-0888-41ea-a6ee-77392dd86174** (`project/scenes/unt_title/scene.gbsres`); no invented scene ID.
- Arena's old trigger was EMPTY at (29,8), size 1x2. Door art is lower, at the far-right arch. Repositioned this existing trigger to **(29,12), size 3x4**, covering x29..31/y12..15. All these collision tiles are clear, with solid ground at y16. Walking into it changes scene; no A needed.
- Trigger clears fight variable17, detaches B, resets runtime scene stack, SWITCHes to UNT_TITLE (0,0; LOGO scene has no physical player spawn).
- Title init defensively detaches all eight old input mappings, resets runtime scene stack, clears fight flag17. Original title music, 0.5s delay and A/B/Start await retained. Title then enters **UNT_CURB** (`515cb9f3-59d6-4ce3-94a2-332dd1a12dda`) at original **(4,13), facing right**; collision-free spawn. UNT_CURB supplies its own story player sprite and Start/status binding.
- No SRAM save deletion/reset, no broad story/inventory-variable reset. Only transient scene-return stack is discarded when entering storyline. Existing storyline progress flags remain as before.

## Return-loop root cause / chosen consequence
`1f/triggers/trigger_6.gbsres` covers (38,2), size 5x2; it calls CAUGHT. CAUGHT PUSH saves the player's exact position while INSIDE this Arena-entry trigger. Duck On Interact previously POPped that exact state. Return to 1F instantly activated the same trigger -> Arena again. In BIOS test, POP merely returned to BIOS, masking the 1F loop.

Duck/gator contact now shakes and **reloads Arena locally at (12,9)** with no PUSH and no POP. Existing Arena init restores platform position/camera, player health and opponent/fight state. Falling therefore still costs the current fight's progress; it does not return to the entry trigger. Added lower pit catch trigger **(6,18), size 24x4** so missing the moving duck also resets locally before the bottom checkerboard. Arena platform (12,9) is clear over solid floor y10. Repeated deaths do not grow/pop the saved stack; doorway discards it.

## Preserved exactly
Compared bytes against prior cumulative combat patch: Arena scene + foe actor and Scene39/menu actor3 unchanged. Compared thumbnails against `gbm-alltime-thumb-flag-fix-patch.zip`: scene40 pic1/pic2 actors identical. No collision-map edits, no fight/input redesign.

## Files changed this pass
- project/scenes/arena/triggers/trigger.gbsres
- project/scenes/arena/triggers/gator_pit_respawn.gbsres (new)
- project/scenes/arena/actors/actor.gbsres (duck return)
- project/scenes/unt_title/scene.gbsres (runtime input/stack cleanup)

## Cumulative source patch — apply to ORIGINAL uploaded project
`gbm-alltime-arena-doorway-patch.zip` includes all prior menu/Arena fixes, thumbnail flag actors, current doorway/pit fixes, and final settings (Scene39). Close GB Studio, back up your project, then extract **INSIDE the folder containing gbm-alltime.gbsproj** and overwrite matching `project/` resources. This is project source, NOT an IPS/BPS ROM patch. Do not extract into `project/` itself. Open gbm-alltime.gbsproj and rebuild normally.

Full archive: `gbm-alltime-arena-doorway-full.zip` contains top-level `gbm-alltime/` project; extract separately and open its gbsproj. It is the FINAL Scene39 project, not TEST.

## ROMs
- FINAL (Scene39 start): build-out/gbm-alltime-arena-doorway-final.gbc
- TEST (throwaway BIOS start): build-out-test/gbm-alltime-arena-doorway-test.gbc

## Runtime validation
Not run in emulator by executor. Parent must GUI-verify walk-through doorway -> title -> A/B/Start -> UNT_CURB; B/Start afterwards must not return to Arena/old menu. Also repeat pit/duck death and verify local Arena respawn, not 1F/BIOS re-entry. Prior combat still needs its previously documented GUI checks.
