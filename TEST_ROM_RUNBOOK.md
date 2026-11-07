# Arena CPU TEST runtime checklist (parent GUI)

ROM: `/workspace/gbm-alltime-work/build-out-test/gbm-alltime-arena-cpu-test.gbc`
FINAL: `/workspace/gbm-alltime-work/build-out/gbm-alltime-arena-cpu-final.gbc` (Scene39 start)

BIOS starts L0=0. Down/Right TWICE -> Advanced (L0=2), A -> Arena. In Dojo emulator: arrows move; X=A/jump, Z=B/punch, Enter=Start. Fresh ROM name avoids old battery save/state confusion; do not restore an old emulator save state.

1. Wait still at spawn (12,9): opponent should face left, close distance, stop near you and visibly wind up before one HP damage. It should recover, not drain HP every frame. Damage is SFX + shake (no HP bar).
2. During visible windup, jump A and/or move across/away. Swing should miss when height gap >6px, horizontal gap >24px, or player crosses behind the frozen facing.
3. Walk to the other side on central platform: CPU should face right and approach; punch art should mirror. It must stay on platform, not follow into pit/door.
4. Face opponent at close range and tap B with pauses (~0.7s). Three legal hits should KO it; it stops moving/attacking. Contact by itself must NOT auto-KO either fighter. B from behind or a different height must miss.
5. Let CPU land five deliberate attacks: expect short pause then local Arena reset at (12,9), both healthy, no crash/1F/BIOS retrigger. Repeat.
6. Leave without beating CPU: go right off central platform, land on lower-right ledge at feet y15, walk Right into door x29..31/y12..15 -> UNT_TITLE. Release buttons; press A/B/Start -> UNT_CURB. No old punch/menu POP should carry over.
7. Pit/duck: falling into center pit should reset locally, not return to old Arena-entry trigger. Repeat twice, then take doorway.
8. FINAL regression: Scene39 menu opens/closes repeatedly; inventory thumbnails honor flags55/56. These resources were preserved byte-for-byte, but check GUI when practical.

Executor ran compilation and static event-model checks only. Runtime gameplay/feel, animation visibility, responsiveness, enemy KO, defeat and doorway/title sequence remain for parent verification. See ARENA_CPU_NOTES.md for behavior knobs/limitations.
