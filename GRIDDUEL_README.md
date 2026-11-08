# Gridduel — playable GB Color baseline (7 November 2026, ET)

This is a complete small-deck duel loop, not a full Duelists of the Roses clone or a promise to fit future-console ideas onto GB. It adapts the unfinished mini Gridduel reference to the existing7×7 board. All new assignments/tuning are provisional. Class/attribute/type and UNO metadata remain undecided/unimplemented.

## Enter / leave

FINAL retains the upload's METRO-CITY start and every setting. Walk into GRIDSTORE at city tile(55,24), size2×2: it already routes to `board_attempt_1` (ID36b29559-ddf3-4ab9-9162-21efb20a0e3b). From the safe return anchor(54,27), walk north one16-pixel step to re-enter. No A interaction is required at the store entrance. Coordinates here are GB Studio's zero-based8-pixel tiles.

Choose BOTS or HASHTAGZ; CPU gets the other theme. HOW TO PLAY explains the controls. Both themes share the same provisional24-card gameplay pool, with different existing-art skins/leader cards, rather than pretending there are two finished canonical card sets. No permanent rewards, ante losses, XP changes or level resets are applied to world data.

Win, loss and confirmed quit return to METRO-CITY anchor(54,27), below/outside GRIDSTORE. The existing TOPDOWN engine uses16-pixel grid snapping; the return is deliberately aligned. Inputs and scene stack are cleared, player shown under fade, then the world's own menu/run initialization resumes. Walking north again intentionally starts another fresh duel.

## Controls

- D-pad: board cursor or list selection; held directions repeat.
- SELECT cycles BOARD → HAND → PIECES → BOARD.
- HAND: A chooses a card, then A on a legal board tile plays it. B opens full card information.
- PIECES: A selects the friendly piece on the board; B inspects it. M/A indicate used move/attack.
- BOARD: A picks your piece; A on an empty legal square moves, or on an enemy in its attack mask attacks. Green marks legal movement/placement; red marks an occupied legal attack target. Yellow is cursor.
- B cancels a selected card/piece. With no selection, B inspects a visible piece/own unoccupied trap; B on empty ground opens quit confirmation. A confirms / B cancels. Hidden enemy traps behave identically to empty ground.
- INFO: A or B returns. Separate MOVE and ATTACK diagrams use O as origin. Leader info explicitly shows no attack.
- START ends your turn. If a piece/card is still selected, first press cancels it; press again to end. No world menu is opened in the duel.
- Outcome: A/B returns to Metro. Holding B through an exit correctly resumes world running; release restores base speed.

## Small-deck rules

-24 playable individual instances plus a leader; hand5, up to5 monsters plus leader. Timing-based shuffles vary fresh matches; CPU decisions are deterministic from the resulting state.
- One card placement/use per turn, beside your own leader for monsters/traps. Spells can be played on any cursor tile. Equipment targets an own piece; equip a monster for useful attack power, as leaders cannot attack.
- Each piece may move once and attack once per turn (separate flags); a newly deployed monster can act. Leader moves orthogonally one cell and never attacks.
- Monster combat compares ATK plus equipment and prototype shape advantage (+500): Circle > Triangle > Square > Circle. Winner survives, tie removes both. Attacks do not move the attacker into the target square. DEF is shown for continuity with the reference but is not used in this first ruleset.
- Hitting a leader reduces its4000 LP by attacker's ATK. Reduce LP to0 or orthogonally surround the opposing leader at its turn start to win. Board edges reduce the number of neighbors required.
- BOLT damages opposing LP800; BALM heals800, capped4000. BLADE/CROWN add500/800 ATK, cumulative cap1500. These magnitudes come from editable card data.
- SNARE removes an entering monster (not a leader); MINE damages its owner's LP1000. A trap is consumed/revealed on activation. Owner knows its traps; enemy does not see position/type in graphics, info, movement markers or occupancy. A friendly piece temporarily covers its own trap marker. No invented adjacent-trigger aura.
- Walks check every actual traversed square: for longer non-diagonal travel, horizontal then vertical. Adjacent diagonal steps visit their destination, not side squares. Jump moves check landing only. Arrows/ranged attacks do not traverse floor tiles or trigger traps, and can pass over intervening pieces.
- A hidden trap is not an occupancy blocker. CPU planning never reads enemy trap slots; activation is resolved only after choosing/performing an action. Enemy face-down monster stats are not consulted for CPU battle scoring.
- An empty deck causes500 LP fatigue at each own turn start, preventing a leaders-only exhausted game from getting stuck. This is provisional tuning and has no permanent consequence.

## Exact movement data / provisional roles

The original `assets/backgrounds/movement-grid-icon-tileset.png` contains11 actual masks. The first7 match mini Gridduel's strip; its engine had not wired them to individual cards. We decoded the existing pixels, not invented universal ranges. Separated cells are destinations: explicit `move_mode=jump` or an independent `attack_mask`, never forced travel through every thumbnail mark.

|Base ID|BOTS / HASHTAGZ skin|Move|Mode|Attack|
|--|--|--|--|--|
|1|CBOT / HASH A|1|walk|1|
|2|BBOT / HASH B|1|walk|1|
|3|DBOT / HASH C|1|walk|5 ranged|
|4|GENBOT / HASH D|2|walk|2|
|5|MILTON / HASH E|1|walk|2|
|6|LBOT / HASH F|7|jump, diagonal1–2|1|
|7|MBOT / HASH G|5|walk, orthogonal1–2|1|
|8|ROEBOT / HASH H|2|walk|2|

These are provisional roles, NOT decided canonical classes/types. LONGBOW replaces only ATTACK with actual mask4. Advanced bow definitions using8/10/11 exist in catalog/tests but are not in starter decks. Movement never expands just because attack range does.

## Editing / rebuild compatibility

- Existing source names, scene IDs/paths, settings and all unrelated files are retained. The only modified original files are board scene init and the variable registry (two appended named globals:240 exit request;242 deck choice). Original actors remain unmodified; a new hidden, resource-valid controller supplies finite Actor On Update callbacks.
- New files are additive `assets/engine/src/core/gridduel_*.c`, `assets/engine/include/gridduel*.h` and an ordinary `plugins/Gridduel/events` plugin. No existing engine implementation/limit was replaced. GB Studio's standard local-engine build compiles these banked files. Use the supplied generated headers; ordinary rebuilding needs no Python or Node scripts.
- Editable card values, independent masks/modes, deck lists and reserved metadata: `plugins/Gridduel/data/catalog.json`. After changing it, run from the project root: `node plugins/Gridduel/tools/build_catalog.js .`, then rebuild in GB Studio. It validates supported kinds/effects and a conservative24-definition budget; shipped catalog has18 definitions (15 in the starter pool,3 advanced test-only bows). New effects still need explicit rule code; this is not an unlimited scripting framework.
- Existing themed art provenance: `plugins/Gridduel/data/art_sources.json`. Source PNGs are untouched. Generated tile header is included. Optional art regeneration: `python3 plugins/Gridduel/tools/build_art.py .` (requires Pillow). It honors actual sprite metasprite slices, keys sprite-green and uses nearest-neighbor fitting. New unmapped monster art falls back to a visible labeled token, never an invisible piece.
- Individual instance ID, level1, XP0, equipment and attack override are explicit fields. XP awards, a leveling curve and a persistent saved roster are NOT implemented; these are per-duel scaffolding only. No UNO mode is implemented.
- Native duel state occupies644 bytes of compiler-unallocated VM heap beginning AFTER `MAX_GLOBAL_VARS`; it never aliases original quest globals or context stacks. Compile-time guards enforce structure size and heap bounds. The native catalog remains bounded WRAM data. This avoids increasing any existing engine memory limit.
- Card/piece presentation uses fixed background tiles, not49 allocated actor sprites or GB Studio's rolling dialogue glyph buffer. Normal scene loading restores graphics after exit.

## Deliberate first-entry limits

No terrain, fusions, deck editor, persistent collection/leveling, RPG quest reward integration, special counter-deck taxonomy or real-time Archon combat. CPU is a simple legal-action heuristic, not a deep search AI. Diagonal walkers may use a single adjacent diagonal edge; longer walks use the documented deterministic path. Advanced equipment replaces an attack mask rather than stacking masks. Existing art is consistently palette-adapted but not newly drawn illustration. This is intended for playable tuning, not a claim of final balance.
