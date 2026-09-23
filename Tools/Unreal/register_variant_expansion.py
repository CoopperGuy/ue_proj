"""Apply reviewed variant rows and allowlists with separate native MCP requests."""
import copy
from datetime import datetime, timezone
import json
from pathlib import Path
import re

from native_mcp_client import UnrealMCP
from register_elemental_pack_native import read, ref, normalize_maps, DT, ASSETS

OUT = Path(__file__).resolve().parents[2] / 'docs/VariantExpansion'
TABLES = ['DT_GASSkillData', 'DT_SkillVariantion', 'DT_JobSkillSet']


def canonical(value):
    value = copy.deepcopy(value)
    normalize_maps(value)
    if isinstance(value, dict):
        return {k: canonical(v) for k, v in value.items()}
    if isinstance(value, list):
        return [canonical(v) for v in value]
    if isinstance(value, str) and value.startswith(('NSLOCTEXT(', 'LOCTEXT(')):
        parts = re.findall(r'"(?:[^"\\]|\\.)*"', value)
        return json.loads(parts[-1]) if parts else value
    return value


def load(name):
    return json.loads((OUT / name).read_text(encoding='utf-8'))


def main():
    c = UnrealMCP()
    if c.call('EditorToolset.EditorAppToolset', 'IsPIERunning', {}):
        raise RuntimeError('Stop PIE before table updates')
    new, updates, assignments = [load(n) for n in ['new-variants.json', 'variant-updates.json', 'assignments.json']]
    before = {name: read(c, name) for name in TABLES}
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    (OUT / ('tables-before-registration-' + stamp + '.json')).write_text(
        json.dumps(before, ensure_ascii=False, indent=2), encoding='utf-8')
    baseline = load('tables-before.json')
    expected = copy.deepcopy(baseline)
    expected[TABLES[1]].update(new)
    for key, patch in updates.items():
        expected[TABLES[1]][key].update(patch)
    for key, patch in assignments.items():
        expected[TABLES[0]][key].update(patch)
    # Resume safely after a partial run, without overwriting intervening edits.
    for name in TABLES:
        current = canonical(before[name])
        if current not in (canonical(baseline[name]), canonical(expected[name])):
            raise RuntimeError('Table differs from reviewed baseline and final state: ' + name)
    variant_ids = [r['variantId'] for r in expected[TABLES[1]].values()]
    assert len(variant_ids) == len(set(variant_ids)) == 53
    linked = set()
    for patch in assignments.values():
        ids = patch['variantIds']
        assert len(ids) == len(set(ids)) and set(ids) <= set(variant_ids)
        linked.update(ids)
    assert linked == set(variant_ids)
    for path in load('assets.json'):
        assert c.call(ASSETS, 'exists', {'path': path}), path
    tags = c.call('GameplayTagsToolset.GameplayTagsToolset', 'ListTags', {'parentTag': 'GASData.Variant'})
    assert all(row['variantTag']['TagName'] in tags for row in new.values())

    name = TABLES[1]
    if canonical(before[name]) != canonical(expected[name]):
        patch = copy.deepcopy(new)
        patch.update(copy.deepcopy(updates))
        # Preserve nested SetByCaller maps during DataTable JSON reimport.
        for key, row in before[name].items():
            if row.get('variantExtraData') and 'variantExtraData' not in patch.get(key, {}):
                patch.setdefault(key, {})['variantExtraData'] = copy.deepcopy(row['variantExtraData'])
        normalize_maps(patch)
        missing = [key for key in new if key not in before[name]]
        if missing:
            c.call(DT, 'add_rows', {'data_table': ref(name), 'row_names': missing})
        c.call(DT, 'set_rows', {'data_table': ref(name), 'values': json.dumps(patch)})
    current = read(c, name)
    if canonical(current) != canonical(expected[name]):
        (OUT / 'variant-verification-failure.json').write_text(json.dumps(current, ensure_ascii=False, indent=2), encoding='utf-8')
        raise RuntimeError('Variant table verification failed; not saving')
    assert c.call(ASSETS, 'save_assets', {'asset_paths': ['/Game/Datatable/' + name]})
    print('Saved and verified 53 variants', flush=True)

    name = TABLES[0]
    if canonical(before[name]) != canonical(expected[name]):
        c.call(DT, 'set_rows', {'data_table': ref(name), 'values': json.dumps(assignments)})
    current = read(c, name)
    if canonical(current) != canonical(expected[name]):
        (OUT / 'skill-verification-failure.json').write_text(json.dumps(current, ensure_ascii=False, indent=2), encoding='utf-8')
        raise RuntimeError('Skill table verification failed; not saving')
    assert c.call(ASSETS, 'save_assets', {'asset_paths': ['/Game/Datatable/' + name]})
    after = {name: read(c, name) for name in TABLES}
    assert canonical(after) == canonical(expected)
    assert after[TABLES[2]] == baseline[TABLES[2]]
    for name in TABLES[:2]:
        assert not c.call(ASSETS, 'is_dirty', {'asset_path': '/Game/Datatable/' + name})
    (OUT / 'tables-after.json').write_text(json.dumps(after, ensure_ascii=False, indent=2), encoding='utf-8')
    result = {'passed': True, 'new_variants': len(new), 'total_variants': len(variant_ids),
              'assigned_player_skills': len(assignments), 'all_variants_assigned': True,
              'steps_and_base_skill_values_preserved': True, 'boss_monster_and_job_rows_preserved': True,
              'saved_tables': TABLES[:2], 'cpp_built': False, 'combat_tested': False}
    (OUT / 'registration-result.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps(result), flush=True)


if __name__ == '__main__':
    main()
