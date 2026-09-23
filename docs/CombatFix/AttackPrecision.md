# Attack movement and hit precision follow-up

The previous suite proved repeated health loss but did not test movement during recovery or false-positive/false-negative hits. The follow-up covers those gaps.

## Confirmed causes and repair

- The ability ended at the last skill step, while its montage continued after its task was detached. The BT resumed pursuit during the remaining animation. Monster abilities now keep their attack lock until both execution and montage completion; cancellation stops the montage and pending hit tasks. Chase and aim tasks respect the lock, while scripted root-motion charges remain available. Pool return cancels outstanding attacks.
- Melee used a single instant overlap and a center-only cone check. Targets on the edge could miss the one sample. Melee steps now support a short active window, sampled by ability-owned tasks, with each actor damaged at most once per step. The cone accounts for the target capsule, queries exclude mesh-only overlaps and friendly units, and solid world geometry blocks damage. Overlap-only portal/room triggers are excluded from obstruction checks; the actual portal has a Pawn overlap response and exposed this distinction during validation. The attack origin/facing remain committed throughout each window.
- BladeGuard used a two-swing montage for one hit. Bomber and Spitter used a three-segment hit-reaction montage. They now use dedicated single-attack montages from existing compatible sequences.
- Zombie damage at 0.45 seconds preceded its forward strike; sampled hand poses showed the right arm still winding back at 0.65 seconds. Its active window now begins at 1.02 seconds. Warden/DualBlade combo windows align with the two montage segments (approximately 0.4 and 1.5 seconds). Charge movement previously started after most/all of its 0.83-second montage had elapsed; Charger/Warden charge starts at 0.2 seconds, with impact following the scripted movement.
- Zombie, Raptor and BladeGuard melee reach is reduced. Their BT attack-selection distances are reduced to match, including the Charger's shared close attack; Charger pursues to 180 cm when closing for melee. Defaults for player step hit-window duration remain zero.

Asset edits are performed through native Unreal MCP. `refine_monster_attack_native.py` records touched skill rows before/after and validates preservation of untouched table values.

## Verification

- Development Editor build succeeded (15.97 seconds), followed by editor restart.
- `ARPG.Combat.MonsterAttacksPIE`: existing 15 real-AI cases, extended to check movement during non-charge montages.
- `ARPG.Combat.MonsterAttackPrecisionPIE`: 16 controlled cases covering front/back/side/range/wall/overlap-only trigger, wind-up dodge, entering/leaving the active window, interruption, chase recovery, zombie/raptor contact, two-hit boss combo and charge movement.

Final native automation result: **2 suites passed, 0 failed, 31 cases**, in 151.29 seconds. All controlled non-charge cases measured **0.00 cm** attack displacement, no damage before the scheduled hit, and correct expected hit counts. The chase-recovery case resumed movement after the attack. Interruptions produced no delayed hit, the boss combo hit twice, and scripted charge still moved and hit. Full results: `precision-run.json`; per-case logs: `precision-cases.json`.

All 10 edited/new assets are saved. PIE is stopped, the editor remains open, and no content or map packages are dirty. Existing unrelated project warnings about legacy classes/item-option tags remain in the native report.

These tests use temporary PIE fixtures and restore player attributes without saving gameplay state. They validate functional timing, range and movement; they do not establish final animation polish or balance.
