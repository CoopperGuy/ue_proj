# Monster combat repair

For the subsequent movement/recovery and hit precision repair, see [AttackPrecision.md](AttackPrecision.md). The results below describe the earlier health-loss repair.

## Confirmed problems

- Natural AI tests also reproduced missing target acquisition after pool activation, despite a visible player. Earlier forced-target probes had masked this.
- Effect contexts pointed at the AI controller/player state, while the ASC is on the character. Source attack power was therefore absent from melee damage calculations. With the test player's defense of 8, zombie base damage 10 rounded to zero.
- Raptor rows had no granted skills while their shared tree requested zombie skill 3001.
- Raptor and zombie animation graphs had no montage slot. Zombie skill 3001 referenced the bomber montage instead of the zombie attack sequence.
- Bomber skill 3002 had a 100-second cooldown. Its tree mixed early ability activation, movement and a second ability-wait task.
- PlayGAS returned InProgress for an already active ability without tracking completion. Failed activation left ActionRequest set. The action-request decorators could then block recovery on cooldown.
- Melee queries used a blocking channel sweep, and measured the cone from an offset point using a 3D angle. Close contact and crowded combat could exclude otherwise valid pawn targets.
- Retained DualBlade boss charge 2003 contained movement only, with no damage step.

## Repair

- AI activation refreshes perception and acquires a nearby player only with clear line of sight; a 0.25-second fallback retries when no target exists. Blackboard target values are refreshed even when the actor pointer is unchanged.
- The project ASC repairs effect contexts when their instigator cannot resolve back to that ASC, using the character avatar. Ability and variant-effect code retains this context instead of overwriting it with the controller. This also repairs source stats and attribution for player attacks using the same path.
- Raptor bite 3003, its own behavior tree and BiteAttack_A montage; zombie uses Zombie_Attack and a 220 cm melee step. Both AnimBPs include DefaultSlot. Raptor locomotion transition rules are connected to Speed.
- Bomber uses a sequential chase/face/attack tree, a 260 cm radial hit, a separate cosmetic explosion, and a 4.5-second cooldown. Zombie cooldown is 2.4 seconds.
- PlayGAS observes ability-spec activity, handles synchronous completion, clears completed/failed requests and cancels on abort. Request decorators route all pending requests to this task; cooldown eligibility remains in skill selection and activation.
- Both melee paths query overlapping pawns, deduplicate actors, and measure horizontal facing from the attacker. The damage formula and stat values are unchanged; source stats now reach that formula correctly.
- Old boss charge has an explicit melee impact step.

## Validation

Asset changes use native Unreal MCP exclusively. DataTable writes are individual requests, with readback and preservation checks for untouched rows and fields. Animation blueprints and the cosmetic explosion blueprint compile with warnings treated as errors, and all edited assets are saved.

`ARPG.Combat.MonsterAttacksPIE` opens a disposable PIE session and uses the real pool, AI perception, behavior trees, montages and GAS damage. It tests ten monster IDs plus stale-request recovery, nearby allies, Warden phase two at two ranges, and a moving target. Each case requires at least two observed health decreases and an active attack montage; source-ASC context identity is checked for every monster and the player. The suite restores player attributes and closes PIE. It does not write save slots.

Run through the native AutomationTestToolset after DiscoverTests. Open the dungeon TestRoom map first. Results and inspected table snapshots are stored in this folder.

Final verification on 2026-09-23:

- ARPGEditor Win64 Development build succeeded (18.39 seconds); editor restarted with the resulting binary.
- `ARPG.Combat.MonsterAttacksPIE` succeeded in 94.71 seconds: all 15 combat cases passed, no test errors. Per-case measurements are in `automation-cases.json`; the native result is in `automation-run.json`.
- Tested Raptor and Zombie normal/elite, Bomber, BladeGuard, Spitter, Charger, Warden and the retained DualBlade boss. Additional cases covered stale requests, nearby allies, Warden phase two from close/far starts and Charger versus a moving target.
- Zombie normal now dealt 4 total damage over two observed hits; elite dealt 6 over two hits. Before the source-context fix, both dealt zero damage over 22 seconds despite playing montages. These figures use the test player's defense of 8, without changing monster attack stats or the damage formula.
- All 16 edited/new assets exist and are saved. PIE stopped normally, with no dirty content or map packages. The editor remains open.

The native result also records existing project warnings about missing legacy Rig/LevelComponent classes and invalid item-option tags. This repair does not resolve those unrelated warnings. The combat suite checks functional attacks and health loss; it does not assess combat balance or animation polish.
