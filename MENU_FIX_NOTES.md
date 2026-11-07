# Scene 39 start-menu freeze fix

## Root cause (evidenced)
GB Studio merges every actor **On Init** (`startScript`) into the scene init script, which runs under `VM_LOCK`. While locked, `state_update()` does not run, so the player cannot move.

Scene 39 actor_3 (patrol NPC) had `EVENT_LOOP_WHILE faCoins (var 69) >= 1` in **On Init**.

- **First boot:** scene init runs actor startScripts *before* the scene script that sets `faCoins=200`. `faCoins` is still 0, so the while-body is skipped, init finishes, lock clears, movement works.
- **After Start menu / inventory return:** `EVENT_SCENE_POP_STATE` reloads Scene 39 with init again, but globals persist. `faCoins` is still >= 1, so the while loop never exits under `VM_LOCK` → freeze.

This hits every return path (menu B/Start/Select pop, and pic1/pic2/pic3 pops) because all of them re-enter Scene 39 init.

## Fix (narrow)
Moved actor_3's patrol `LOOP_WHILE` from `startScript` → `updateScript` (compiled `lock:false`, `loop:true`). No map/asset/menu script changes.

## Test status
- Compile: **PASS** (`gb-studio-cli make:rom`, 1.0M CGB-only ROM).
- Runtime emulator verification: **NOT run here**. Parent should dispatch computerUse against `/workspace/gbm-alltime-work/build-out/gbm-alltime.gbc` with scenarios below.

## Emulator scenarios for parent
1. Boot Scene 39 → walk with D-pad.
2. Press Start → Scene 40 menu → press B → confirm walk restored at prior tile.
3. Repeat open/close Start 3×.
4. Start → click unlocked thumbnail → item scene → A/B/Start/Select back → walk restored.
5. Confirm actor_3 still patrols after return.
