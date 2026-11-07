# Arena CPU opponent — latest cumulative build

## Implemented (source + compilation/static model; GUI verification pending)
The opponent is now a CPU-controlled fighter, not an instant contact KO prop. CPU lives in the foe actor's **On Update**. No engine changes. No init loop, blocking Move To, AI thread per attack, sprite hiding, or invalid SET_FRAME offsets.

### Behavior
- Initial ~1.25-second recovery/grace, then approaches player horizontally by **1 pixel per update pass** when player is on the central shared platform.
- CPU origin stays within **x88..160 px** (tiles 11..20), fixed y72 px (tile 9). Player must be in x80..168/y66..78 px for pursuit; this protects the pit/door route.
- Stops approaching inside **24 px horizontal range**, faces the player and starts a **six-tick telegraph**. Facing is frozen until impact, so jumping away or crossing behind can dodge.
- At impact re-reads positions and checks foe/player alive, **abs horizontal anchor difference <=24 px**, **abs vertical anchor difference <=6 px**, correct facing side, and player invulnerability==0. Exactly **1 damage** per swing. Recovery **15 ticks** follows a hit OR miss.
- CPU uses the existing sdaction **piunching** art in a new metadata-only **CPU_PUNCH** state with horizontal mirroring. The original state was fixed-direction; original five sprite states and PNG are unchanged. CPU art is the project's existing small stick fighter, not newly drawn art. Basic approach slides the idle sprite; no new walk art was invented.
- CPU stops moving/attacking when opponent HP reaches 0. Player can leave via doorway at any time, with no win gate.
- Player **B** still punches; now checks facing AND the same vertical/horizontal range. Three successful hits KO the opponent. Hit sound + damage shake provide feedback; no new HP bar is added.
- Contact alone causes neither automatic opponent KO nor player HP drain. Normal actor collision remains enabled while alive. Collision is disabled only for a defeated opponent, so it cannot block the exit.
- Player starts with **5 HP**; opponent with **3 HP**. Player at 0 briefly pauses (~0.5s), then reloads the Arena at safe (12,9). Pit/duck also keeps the prior local reset consequence. A reload resets both fighters/AI counters. No caller POP or new stack PUSH occurs.

### Timing and knobs (GB Studio resource events)
Compiler wraps actor On Update with an extra idle: four explicit WAIT frames + one compiler idle means a nominal **5-frame tick** (script execution can lengthen timing). Values below are ticks, not raw frame counts.
- Global **31 / ARENA_CPU_HP**: 3 on each Arena init.
- Global **32 / ARENA_PLAYER_HP**: 5 on each Arena init; replaces previously uninitialized/context-local L3 hit counter. Script 5 executes before explicit health init.
- **215 ARENA_CPU_PHASE**: 0 seek, 1 telegraph, 2 recover.
- **216 ARENA_CPU_TIMER**: grace/recovery 15 ticks (~75f), windup 6 ticks (~30f).
- **217 ARENA_PLAYER_IFRAMES_TICKS**: set 10 ticks (~50f) on CPU hit; blocks another CPU hit.
- **218 ARENA_PLAYER_ATTACK_COOLDOWN**: set 8 ticks (~40f) per accepted B; B visual lasts 18f.
- **219 ARENA_CPU_HIT_COOLDOWN**: set 5 ticks (~25f) on player hit, prevents duplicate damage.
- **220..223** CPU-only position scratch; **224..227** B-only scratch (no cross-script overwrites).
- Range 24px, height tolerance 6px, step bounds and timing constants are editable in `arena/actors/actor_2.gbsres` and `arena/scene.gbsres`.

## Preserved
- Scene39/menu On Init freeze fix.
- Inventory thumbnail 55/56 corrections (scene40 pic1/pic2 resources).
- No invalid DEAD frame offset; no contact auto-KO.
- Far-right walk-through door (29,12), size3x4 -> real UNT_TITLE; input bindings + runtime return stack cleanup retained.
- Existing title A/B/Start -> UNT_CURB (4,13), story flags and saves preserved.
- Pit/duck local Arena reset and lower catch trigger retained.
All untouched prior-fix resources were byte-compared to the prior doorway full archive. Engine files and original sprite states also compared unchanged.

## Scope / limitations
No jumping AI, ladder/pathfinding, platform-edge traversal, retaliation outside central platform, new HUD or full fighting-game combo system. CPU stops at the safe arena edges and waits when player leaves/jumps. It does not retreat if the player deliberately overlaps it. Punch timing/visibility, perceived reach and difficulty still need the parent's emulator review. Timing figures are approximate; static model checks prove the event decisions, not engine movement/rendering or real input responsiveness.

## Deliverables (use these NEW filenames; older ROMs were not overwritten)
- FINAL Scene39: `build-out/gbm-alltime-arena-cpu-final.gbc`
- TEST BIOS: `build-out-test/gbm-alltime-arena-cpu-test.gbc`
- Cumulative original-project source patch: `gbm-alltime-arena-cpu-patch.zip`
- Full FINAL project snapshot: `gbm-alltime-arena-cpu-full.zip`
- Static test source: `test_arena_cpu_static.py`; report `ARENA_CPU_STATIC_TESTS.txt`.
- Build logs: `build-out/arena-cpu-final-build.log`, `build-out-test/arena-cpu-test-build.log`.

## Apply source patch
Close GB Studio and back up project. Extract `gbm-alltime-arena-cpu-patch.zip` **inside the folder containing gbm-alltime.gbsproj**; overwrite matching `project/` AND `assets/` resources. It includes ALL previous menu/combat/door/pit/inventory fixes against the original uploaded project, final Scene39 settings, variable definitions and the sprite metadata needed for mirrored CPU punch. This is not an IPS/BPS ROM patch. Do not extract into project/ itself.
Full archive extracts to its own `gbm-alltime/` folder; open its gbsproj separately. TEST settings are not included in the patch/full archive.
