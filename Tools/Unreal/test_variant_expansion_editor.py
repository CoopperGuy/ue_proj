"""Read-only external native MCP checks; this does not compile C++ or run combat."""
import json
from native_mcp_client import UnrealMCP
from register_variant_expansion import OUT, TABLES, load, canonical
from register_elemental_pack_native import read, ASSETS


def main():
    c = UnrealMCP()
    checks = []

    def check(name, actual, expected):
        checks.append({'name': name, 'passed': actual == expected, 'actual': actual, 'expected': expected})

    def props(path, names):
        return json.loads(c.call('editor_toolset.toolsets.object.ObjectTools', 'get_properties',
                                {'instance': {'refPath': path}, 'properties': names}))

    tables = {name: read(c, name) for name in TABLES}
    baseline = load('tables-before.json')
    check('live_tables_match_saved_snapshot', canonical(tables), canonical(load('tables-after.json')))
    # Avoid duplicating full snapshots in the report.
    checks[-1].update(actual=checks[-1]['passed'], expected=True)
    skills, variants = tables[TABLES[0]], tables[TABLES[1]]
    ids = [r['variantId'] for r in variants.values()]
    check('53_unique_variant_ids', len(set(ids)), 53)
    check('no_duplicate_variant_ids', len(ids), len(set(ids)))
    linked = set()
    for key, assignment in load('assignments.json').items():
        row = skills[key]
        check(key + '.assignment', row['variantIds'], assignment['variantIds'])
        check(key + '.no_duplicate_variants', len(row['variantIds']), len(set(row['variantIds'])))
        check(key + '.no_missing_variants', set(row['variantIds']) <= set(ids), True)
        check(key + '.steps_preserved', row['steps'], baseline[TABLES[0]][key]['steps'])
        linked.update(row['variantIds'])
    check('all_variants_assigned_to_player_skills', sorted(linked), sorted(ids))
    # Every referenced status/field class must resolve in the live editor.
    refs = set()
    for row in variants.values():
        refs.add(row['icon'])
        extra = row['variantExtraData']
        for effect in extra.get('TargetEffects', []):
            path = effect['EffectClass']
            check('variant_' + str(row['variantId']) + '.effect_class_set', path != 'None', True)
            if path != 'None':
                refs.add(path.split("'")[1])
        if 'SkillActorClass' in extra:
            refs.add(extra['SkillActorClass'].split("'")[1])
    for path in sorted(refs):
        # Asset registry existence queries use the Blueprint asset, not its generated class.
        asset_path = path[:-2] if path.endswith('_C') else path
        check(path + '.asset_exists', c.call(ASSETS, 'exists', {'path': asset_path}), True)
    root = '/Game/Skills/Variants/'
    for name, attribute, value, duration in [('Haste', 'MovementSpeed', 120, 2), ('Ward', 'Defense', 20, 3), ('Mana', 'Mana', 12, 0)]:
        p = props(root + 'GE_Variant_' + name + '.GE_Variant_' + name,
                  ['durationPolicy', 'durationMagnitude', 'modifiers', 'stackingType', 'stackLimitCount', 'gEComponents'])
        check(name + '.duration_policy', p['durationPolicy'], 'HasDuration' if duration else 'Instant')
        check(name + '.duration', p['durationMagnitude']['scalableFloatMagnitude']['value'], duration)
        check(name + '.attribute', p['modifiers'][0]['attribute']['attributeName'], attribute)
        check(name + '.magnitude', p['modifiers'][0]['modifierMagnitude']['scalableFloatMagnitude']['value'], value)
        # Current engine serializes the Additive enum alias as AddBase.
        check(name + '.additive', p['modifiers'][0]['modifierOp'] in ('Additive', 'AddBase'), True)
        check(name + '.stacking', p['stackingType'], 'AggregateByTarget' if duration else 'None')
        check(name + '.no_slow_components', p['gEComponents'], [])
        if duration:
            check(name + '.stack_cap', p['stackLimitCount'], 1)
    p = props(root + 'GE_Variant_ChillStack.GE_Variant_ChillStack',
              ['durationMagnitude', 'stackLimitCount', 'overflowEffects', 'bDenyOverflowApplication', 'bClearStackOnOverflow', 'modifiers'])
    check('chill_stack.window', p['durationMagnitude']['scalableFloatMagnitude']['value'], 3)
    check('chill_stack.third_hit_overflow', p['stackLimitCount'], 2)
    check('chill_stack.freeze', p['overflowEffects'][0]['refPath'], '/Game/Skills/FrostRush/GE_FrostRush_Freeze.GE_FrostRush_Freeze_C')
    check('chill_stack.clear', p['bClearStackOnOverflow'], True)
    check('chill_stack.deny_overflow', p['bDenyOverflowApplication'], True)
    check('chill_stack.no_attribute_modifier', p['modifiers'], [])
    for name in ['EmberField', 'RimeField']:
        path = root + 'BP_Variant_' + name
        p = props(path + '.BP_Variant_' + name, ['radius', 'lifeSpan'])
        check(name + '.default_radius', p['radius'], 200)
        check(name + '.default_lifetime', p['lifeSpan'], 3)
        for component in ['SkillObjectMesh', 'SkillEffectComp']:
            p = props(path + '.Default__BP_Variant_' + name + '_C:' + component, ['relativeLocation'])
            check(name + '.' + component + '.ground_origin', p['relativeLocation'], {'x': 0, 'y': 0, 'z': 0})
    for path in load('assets.json') + ['/Game/Datatable/' + name for name in TABLES[:2]]:
        check(path + '.saved', c.call(ASSETS, 'is_dirty', {'asset_path': path}), False)
    result = {'passed': all(x['passed'] for x in checks), 'check_count': len(checks), 'checks': checks,
              'cpp_built': False, 'combat_tested': False}
    (OUT / 'editor-test-result.json').write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps({'passed': result['passed'], 'checks': len(checks), 'failures': [x for x in checks if not x['passed']]}), flush=True)
    if not result['passed']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
