# LATEST: CPU opponent
See ARENA_CPU_NOTES.md and TEST_ROM_RUNBOOK.md. Older contact-auto-KO/B-only details below are superseded by CPU combat; doorway/pit geometry and stack fixes remain.

# LATEST UPDATE: doorway / pit return
See ARENA_DOORWAY_NOTES.md and TEST_ROM_RUNBOOK.md. Older duck-to-BIOS directions below are superseded: duck/pit now resets the fight LOCALLY; the far-right arch leads to UNT_TITLE. Prior combat fixes below are unchanged.

# Arena 1v1 combat fix

## Scene
**Arena** (`project/scenes/arena`, id `b9f61af9-…`, PLATFORM). BG `230913-inanna-one1 (2)`.

## Screenshot map
| Asset | Content | Maps to |
|---|---|---|
| `37ccb03…jpg` (bug report) | Glitched Arena combat | Arena mid-fight |
| `ae9c01f…jpg` (bug report) | KERNEL PANIC | Duck POP without PUSH (fixed) |
| `3db79174…png` (GUI retest) | Player punch at range, smaller foe, duck below | Arena; punch anim OK at distance |

## Root causes (evidenced)

### A. Contact KO / close-range glitch (this pass)
Opponent On Interact ran `EVENT_ACTOR_SET_FRAME` **frame=2** after `SET_STATE DEAD`.
DEAD anim is **1 frame** (`frame_start == frame_end` in compiled sheet).
Engine `actor_set_frame_offset` does `offset % (frame_end - frame_start)` → **modulo / divide by zero** on contact collision (platform runs actor `script` with param 0 on group collision — see `actors_handle_player_collision`).
Matches GUI: close approach glitches player pose; A-near-foe does not confirm KO; contact path unsafe.

**Fix:** removed SET_FRAME from foe interact; DEAD + `EVENT_ACTOR_COLLISIONS_DISABLE`; dropped commented leftover distance block.

### B. B had no hitbox
B only set `punch`→wait→`STANCE`. No damage/distance. Fundamental — not a hitbox.

**Fix (narrow):** on B: punch; if distance to foe **≤ 3** tiles: DEC var **31** (Script 5 sets HP=3), flicker+SFX+shake; if 31≤0 → DEAD + collisions disable; wait; return to **default** state (not STANCE).

### C. Perpetual STANCE near foe
Init locked **STANCE** (7 OBJ tiles). Close to foe (2+ tiles) stresses GB 10-sprites/line; blank/small look. Compiler already mirrors STANCE frames, so empty-dir theory alone insufficient; OBJ pressure + contact script fault together explain GUI.

**Fix:** init + post-punch use **default** (4-tile walk), punch only during B.

### D. Prior fixes (still in tree)
- Removed update proximity→DEAD.
- PUSH before every Arena SWITCH (BIOS / CAUGHT / Script 8).
- Scene 39 menu freeze (`actor_3` LOOP off On Init).

### E. Sprite tile budget (exact)
`colorMode=color`, `spriteMode=8x16`, Arena BG `n_tiles=27` → `maxSpriteTilesForBackgroundTilesLength` = **192**. Usage 40+27+4=**71≤192**.

### F. playerHit1 / L3
playerHit1 runs on group 1 (`COLLISION_GROUP_1=0x2`) → shake + DEC **L3**. L3 was never initialized; now init sets **L3=5**. No win/lose UI on L3=0 (limitation).

## Not claimed fixed in GUI
Parent must retest. Contact KO and B 3-hit KO are compile-verified only.

## Artifacts
| File | Role |
|---|---|
| `build-out/gbm-alltime-menu-fix.gbc` | FINAL (Scene 39 start) |
| `build-out/gbm-alltime-arena-combat-fix.gbc` | Same FINAL bytes, combat name |
| `build-out-test/gbm-alltime-arena-test.gbc` | TEST (BIOS start) |
| `gbm-alltime-arena-fix-patch.zip` | Changed resources + notes |
| `gbm-alltime-arena-fix.zip` | Full project snapshot |

## Patch apply
Unzip `gbm-alltime-arena-fix-patch.zip` into project root (folder with `gbm-alltime.gbsproj`). Includes Scene 39 `actor_3` menu fix.
