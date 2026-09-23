"""Check recorded native-MCP PIE evidence for the Warden encounter."""
import json
import math
from pathlib import Path

DATA = Path(__file__).resolve().parents[2] / "docs" / "Bosses"


def read(name):
    return json.loads((DATA / name).read_text(encoding="utf-8"))


checks = {}


def check(name, condition):
    checks[name] = bool(condition)
    assert condition, name


spawned = json.loads(read("pie-spawned.json")["output"])
check("boss_room_spawn", any(m["name"] == "BP_WardenBoss_C_0" and not m["hidden"] for m in spawned["monsters"]))
cut, charge = read("pie-crosscut.json"), read("pie-charge.json")
for name, record in [("crosscut", cut), ("charge", charge)]:
    check(name + "_activation", record["activation"]["activated"])
    check(name + "_damage", record["health_after"] < record["health_before"])
    check(name + "_finished", all(not a["bIsActive"] for a in record["abilities_after"]))
distance = math.dist(charge["samples"][0]["location"], charge["samples"][-1]["location"])
check("charge_travel_and_impact", 400 < distance < 510)
interrupted = read("pie-interrupted-combo.json")
check("interrupted_combo_only_first_hit", interrupted["interrupted"] and
      interrupted["health_before"] - interrupted["health_after"] == (cut["health_before"] - cut["health_after"]) / 2)
check("interrupted_combo_finished", all(not a["bIsActive"] for a in interrupted["abilities_after"]))
phase1 = read("pie-phase1-ai.json")
check("phase1_selection", {s["phase"] for s in phase1["samples"]} == {1} and
      3201 in {s["action_request"] for s in phase1["samples"]})
changes = []
previous = phase1["health_before"]
for sample in phase1["samples"]:
    if sample["player_health"] < previous:
        changes.append(sample["game_time"])
    previous = sample["player_health"]
check("combo_hits_separated", len(changes) >= 2 and changes[1] - changes[0] >= 0.39)
transition = {a["attributeName"]: a["currentValue"] for a in read("pie-phase-transition.json")["attributes"]}
check("damaged_below_half", 0 < transition["Health"] < transition["MaxHealth"] / 2)
for label, path, skill in [("near", "pie-phase2-near-ai.json", 3203), ("far", "pie-phase2-far-ai.json", 3204)]:
    record = read(path)
    check("phase2_" + label + "_selection", {s["phase"] for s in record["samples"]} == {2} and
          skill in {s["action_request"] for s in record["samples"]})
    check("phase2_" + label + "_damage", record["health_after"] < record["health_before"])
fan = read("pie-phase2-far-ai.json")
check("five_blade_projectiles", max(len(s["projectiles"]) for s in fan["samples"]) == 5)
death = read("pie-defeated.json")
check("boss_defeat_and_room_clear", death["hidden_after_damage"] and
      all("CLEARED" in room["state"] for room in death["room_states"]))
saved = read("final-save-check.json")
check("saved_and_pie_stopped", not saved["pie_running"] and not any(saved["dirty"].values()))
result = {"passed": True, "checks": checks, "charge_distance_cm": round(distance, 2),
          "observed_combo_gap_seconds": round(changes[1] - changes[0], 3)}
(DATA / "verification-result.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
print(json.dumps(result))
