# Fresh-upload-only: hold B run + CPU update

**Supersedes every previous run-cpu-v2/cumulative patch. Do not overlay those old patches.**
This source delta applies ONLY to the fresh uploaded ZIP `88ec09f64ebac2dc547efda56d0e217f9e5d0787448853d67b9e8ecd192c5f7a.zip` (uploaded renamed project), not the original/stale upload. It is not an IPS/BPS ROM patch.

## Apply on Mac
Close GB Studio, back up the current project, and use the exact fresh uploaded project snapshot. If the ZIP and project are both in your default Downloads folder:

```sh
unzip -o "$HOME/Downloads/gbm-alltime-renamesafe-run-cpu-patch.zip" -d "$HOME/Downloads/gbm-alltime"
```

Destination must be the folder directly containing `gbm-alltime.gbsproj` and `project/`, not its parent. Adjust only `-d` if yours lives elsewhere. Reopen `gbm-alltime.gbsproj` and rebuild. If edited since this fresh ZIP, compare/merge first; the manifest contains per-file BEFORE/AFTER SHA256s. This patch adds no old generic scene directories and contains NO settings file. A new output ROM can also be played directly without rebuilding.

## Exact source scope / preservation
- Fresh snapshot preserved separately at `/workspace/gbm-renamesafe-fresh/gbm-alltime`; working source at `/workspace/gbm-renamesafe-work/gbm-alltime`.
- **56 of56 scene IDs/names/paths unchanged**. Fresh upload contains27 renamed scene names vs stale upload. See `SCENE_IDENTITY_AUDIT.md` for every current name/ID/path.
- **58 existing files modified +1 new script**, zero removed. Exactly56 scene entry files receive speed-reset/setup prefixes; Arena init additionally gets2 requested numeric changes. One CPU actor's14 existing event arguments tuned. Existing variables preserved in order with their fresh shortened symbols; append unused variable228 only.
- **6,500 unrelated files byte-identical**. All73 actor resources except the CPU actor unchanged (72 unchanged); all24 existing custom scripts unchanged. All assets/sprite metadata, collisions, scene fields, fresh settings and project metadata unchanged. User's nested `2/` folder left untouched. No old patch overlaid, no Git operations, pushes, repo auth or engine modifications.
- Existing menu fix is in **METRO-CITY** (`metro-city`, stable old internal symbol `scene_39`); inventory55/56 is in **MENU** (`menu`, internal `scene_40`). Their actors stay byte-identical. Existing doorway, pit/duck, player combat handlers and title input/stack cleanup also remain byte-identical.
- FINAL start is the fresh setting **METRO-CITY**, ID `ded6baad-2400-4c26-9a79-b416093dafef`. Settings remain byte-for-byte fresh. TEST differs only in its separate throwaway copy's startSceneId = BIOS.

## Running
Reusable custom event `project/scripts/hold_b_run_topdown.gbsres` starts a yielding background poll, reads physically held B (no B binding replacement), sets base1/run2. **37 TOPDOWN scenes** enabled by stable scene ID, preserving fresh names. TOPDOWN **LOADING-SCREEN-1**, **BOOT**, **BOOT2** excluded (existing menu/intro behavior). Non-TOPDOWN Arena/MENU/title excluded. Every scene starts by restoring base1, preventing speed leaks; returning to exploration polls current held state. Fresh source has no scene-specific player-speed overrides. A interactions/Start/B inventory handling unchanged. Variable228 = TOPDOWN_RUN_THREAD.

## CPU tuning
Fresh CPU script semantics exactly matched prior pre-v2 CPU; no conflicting user combat edits were overwritten. Kept fresh actor fields/IDs and changed requested event arguments only:
- step1->2px; explicit update wait4->1frame; native x136->120 in30frames (~32px/sec).
- chase height follows CPUY-56..CPUY+8, rather than absolute Y66..78. Tracks jumping over same platform, no complex navigation.
- CPU HP3->5; player HP remains5. Initial grace15->12 ticks, recovery15->9; existing6-tick windup retained. Variables31 HP,32 playerHP,215 phase,216 timer,217 iframes,218 B cooldown,219 foe hit cooldown,220..227 scratch.
- Damage still requires correct facing, <=24px horizontal, <=6px vertical alignment, no active player invulnerability. B punches remain attacks; no contact auto-KO/drain. CPU stops on KO; loss reloads Arena locally. Door is never win-gated.
- Earlier reported total CPU immobility was NOT reproduced in the old local ROM: it moved only ~8-9px/sec and stopped tracking jumps outside narrow Y band. This update addresses that measured behavior; does not pretend to know which independent user build caused complete immobility.

## Actual fresh-ROM verification
Both new FINAL and TEST compiled successfully. Fresh native **PyBoy** execution, generated linker-map scene identity lookup and live memory observations were used (no RAM pokes). **27 native/preservation assertions passed**, plus7,725 preservation checks:
- Actual16-frame travel16px walk/32px run; live speed32/64, release32.
- Three MENU cycles entering with B held, closing with B, holding into METRO-CITY, releasing; no leak/freeze.
- Actual CPU movement, jumping pursuit,5HP, no contact auto-KO, real CPU hit gaps55frames; loss returns healthy at tile(12,9).
- Actual B defeats CPU; no CPU movement/damage over180 further frames. Arena B speed stays32.
- Walk through door WITHOUT winning -> real UNT_TITLE; B -> real UNT_CURB at tile(4,13).
- Pit/duck resets locally, no old encounter-trigger loop.
Evidence: `FRESH_NATIVE_RESULTS.json`, `PRESERVATION_RESULTS.json`, screenshots, and saved development harness at `/workspace/gbm-renamesafe-work/runtime-evidence/`. All probe data generated from these new ROMs, not reused old evidence. Human GUI animation/difficulty feel and all37 exploration scenes individually still need review. Basic platform-edge AI does not jump/pathfind.

## ROMs / GUI route
FINAL: `build-final/gbm-alltime-renamesafe-run-cpu-final.gbc`
TEST: `build-test/gbm-alltime-renamesafe-run-cpu-test.gbc`

FINAL boots METRO-CITY using uploaded start settings. Hold B+direction, release to walk; verify A interaction and MENU inventory as usual.
TEST BIOS starts **L0=0**. Down twice -> Advanced (L0=2), A -> Arena. A jump/B punch. Let CPU approach, dodge/jump and fight; five accepted B hits KO it. Go right off central platform to lower-right ledge, then walk right through doorway trigger **x29..31,y12..15** -> UNT_TITLE. B/A/Start begins UNT_CURB. Falling/duck interaction respawns locally at(12,9).
Use fresh ROM filenames, reset emulator instead of loading stale save states. These builds use no external server/deployment.
