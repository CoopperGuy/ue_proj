"""Read-only native MCP test. Pass text to ProgrammaticToolset.execute_tool_script.

Verifies editor configuration; it does not execute combat or compile C++.
"""
import json


def call(toolset, name, args):
    return execute_tool(toolset + '.' + name, json.dumps(args)).get('returnValue')


def rows(name):
    table = {'refPath': '/Game/Datatable/' + name + '.' + name}
    names = call('editor_toolset.toolsets.data_table.DataTableTools', 'list_rows', {'data_table': table})
    return json.loads(call('editor_toolset.toolsets.data_table.DataTableTools', 'get_rows',
                           {'data_table': table, 'row_names': names}))


def props(path, names):
    return json.loads(call('editor_toolset.toolsets.object.ObjectTools', 'get_properties',
                           {'instance': {'refPath': path}, 'properties': names}))


def exists(path):
    return call('editor_toolset.toolsets.asset.AssetTools', 'exists', {'path': path})


def input_data(path, script, module, name):
    return call('NiagaraToolsets.NiagaraToolset_System', 'GetStackInputData', {'stackInputRef': {
        'system': {'refPath': path}, 'emitterName': 'Fountain', 'scriptName': script,
        'moduleName': module, 'rendererIndex': -1, 'inputNameStack': [name]}})


def run():
    checks = []

    def check(name, actual, expected):
        if isinstance(actual, (int, float)) and isinstance(expected, (int, float)):
            passed = abs(actual - expected) < .0001
        else:
            passed = actual == expected
        checks.append({'name': name, 'passed': passed, 'actual': actual, 'expected': expected})

    skills = rows('DT_GASSkillData')
    variants = rows('DT_SkillVariantion')
    jobs = rows('DT_JobSkillSet')
    for table, field, name in [(skills, 'skillId', 'skills'), (variants, 'variantId', 'variants')]:
        ids = [r[field] for r in table.values()]
        check(name + '.unique_ids', len(ids), len(set(ids)))
    configs = [('LavaEruption', 1013, 28, 24, 7, 4, 8, .5),
               ('ArcScatter', 1014, 36, 16, 3.5, 1.2, 1, 0),
               ('FrostRush', 1015, 55, 20, 6, 1.8, 3, .6)]
    all_assets = []
    for i, (name, skill_id, damage, mana, cooldown, life, hits, interval) in enumerate(configs):
        root = '/Game/Skills/' + name
        row = skills[name]
        for field, value in [('skillId', skill_id), ('baseDamage', damage), ('manaCost', mana),
                             ('cooldownDuration', cooldown), ('hitCount', hits), ('hitInterval', interval)]:
            check(name + '.' + field, row[field], value)
        check(name + '.original_variants_preserved',
              all(v in row['variantIds'] for v in [24 + i*3, 25 + i*3, 26 + i*3]), True)
        check(name + '.unique_variant_ids', len(row['variantIds']), len(set(row['variantIds'])))
        check(name + '.cooldown_tag', row['coolDownTags']['TagName'], 'GASData.CoolDown.' + str(skill_id))
        check(name + '.reward_pool', jobs['WarriorSkillSet']['skillIds'].count(skill_id), 1)
        check(name + '.icon', exists(row['icon']), True)
        bp = root + '/BP_' + name + '.BP_' + name
        actor = props(bp, ['lifeSpan', 'skillEffect', 'hitEffect'])
        check(name + '.actor_lifetime', actor['lifeSpan'], life)
        mesh_path = root + '/Meshes/SM_' + name + '.SM_' + name
        mesh_comp = root + '/BP_' + name + '.Default__BP_' + name + '_C:SkillObjectMesh'
        check(name + '.mesh_binding', props(mesh_comp, ['staticMesh'])['staticMesh']['refPath'], mesh_path)
        check(name + '.mesh_exists', exists(mesh_path), True)
        step = row['steps'][-1]
        spawn = step['ProjectileData' if name == 'ArcScatter' else 'GroundData']
        check(name + '.spawn_class', spawn['SkillActorClass'].split("'")[1], root + '/BP_' + name + '.BP_' + name + '_C')
        check(name + '.step_lifetime', spawn['LifeSpan'], life)
        if name == 'ArcScatter':
            for key, value in [('ProjectileCount', 3), ('SpreadAngle', 12), ('bFireMultipleProjectiles', True)]:
                check(name + '.' + key, spawn[key], value)
            check(name + '.speed', props(bp, ['projectileSpeed'])['projectileSpeed'], 2300)
        else:
            radius = 240 if name == 'LavaEruption' else 220
            check(name + '.step_radius', spawn['Radius'], radius)
            check(name + '.actor_radius', props(bp, ['radius'])['radius'], radius)
            sphere = root + '/BP_' + name + '.Default__BP_' + name + '_C:SphereComp'
            check(name + '.no_extra_overlap_hits', props(sphere, ['bGenerateOverlapEvents'])['bGenerateOverlapEvents'], False)
        for field, kind in [('skillEffect', 'Trail'), ('hitEffect', 'Impact')]:
            path = root + '/Effects/NS_' + name + '_' + kind + '.NS_' + name + '_' + kind
            check(name + '.' + kind + '.binding', actor[field]['refPath'], path)
            status = call('NiagaraToolsets.NiagaraToolset_System', 'GetSystemCompileState', {'system': {'refPath': path}})
            check(name + '.' + kind + '.compile', status['aggregateStatus'], 'UpToDate')
            check(name + '.' + kind + '.errors', status['bHasErrors'], False)
            check(name + '.' + kind + '.warnings', status['bHasWarnings'], False)
            if kind == 'Impact':
                loop = input_data(path, 'EmitterUpdateScript', 'EmitterState', 'Loop Behavior')
                check(name + '.impact_finite', loop['value']['displayName'], 'Once')
            all_assets.append(path)
        all_assets.extend([bp, mesh_path, row['icon']])
        all_assets.extend(root + '/Materials/M_' + name + '_' + slot for slot in ['Facet', 'Edge', 'Core'])

    expected = [('LavaEruption_Area', 24, 'AreaMultiplier', 1.35),
                ('LavaEruption_Cooldown', 25, 'CooldownReduction', .75),
                ('LavaEruption_Mana', 26, 'ManaCostMultiplier', .7),
                ('ArcScatter_Multiple', 27, 'MultipleSkill', 2),
                ('ArcScatter_Pierce', 28, 'Pierce', 3),
                ('ArcScatter_Fork', 29, 'Fork', 2),
                ('FrostRush_Area', 30, 'AreaMultiplier', 1.4),
                ('FrostRush_Freeze', 31, 'FreezeStack', 1.2),
                ('FrostRush_Cooldown', 32, 'CooldownReduction', .75)]
    for name, variant_id, tag, magnitude in expected:
        row = variants[name]
        check(name + '.id', row['variantId'], variant_id)
        check(name + '.executor', row['variantTag']['TagName'], 'GASData.Variant.' + tag)
        check(name + '.magnitude', row['modifierValue'], magnitude)
        check(name + '.icon', exists(row['icon']), True)
        all_assets.append(row['icon'])
    rush = skills['FrostRush']['steps']
    check('rush.charge_first', rush[0]['_structType'], '/Script/ARPG.GASChargeStepData')
    check('rush.wait_for_charge', rush[0]['bWaitUntilFinished'], True)
    check('rush.charge_distance', rush[0]['ChargeData']['ChargeDist'], 600)
    check('rush.charge_duration', rush[0]['ChargeData']['ChargeDuration'], .25)
    check('rush.use_arrival_location', rush[1]['bUseLastStepEndLocation'], True)
    for suffix, duration in [('Slow', 2), ('Freeze', 1.2)]:
        path = '/Game/Skills/FrostRush/GE_FrostRush_' + suffix
        effect = props(path + '.GE_FrostRush_' + suffix, ['durationPolicy', 'durationMagnitude', 'modifiers'])
        check(suffix + '.duration_policy', effect['durationPolicy'], 'HasDuration')
        check(suffix + '.fixed_duration', effect['durationMagnitude']['magnitudeCalculationType'], 'ScalableFloat')
        check(suffix + '.seconds', effect['durationMagnitude']['scalableFloatMagnitude']['value'], duration)
        if suffix == 'Freeze':
            check('freeze.stops_movement', any(m['attribute']['attributeName'] == 'MovementSpeed'
                  and m['modifierOp'] == 'Override' and m['modifierMagnitude']['scalableFloatMagnitude']['value'] == 0
                  for m in effect['modifiers']), True)
        all_assets.append(path)
    check('rush.slow_binding', skills['FrostRush']['effectsToApplyOnHit'],
          ["/Script/Engine.BlueprintGeneratedClass'/Game/Skills/FrostRush/GE_FrostRush_Slow.GE_FrostRush_Slow_C'"])
    check('rush.freeze_binding', variants['FrostRush_Freeze']['variantExtraData']['TargetEffects'][0]['EffectClass'],
          "/Script/Engine.BlueprintGeneratedClass'/Game/Skills/FrostRush/GE_FrostRush_Freeze.GE_FrostRush_Freeze_C'")
    slow_values = variants['SlowHit']['variantExtraData']['TargetEffects'][0]['SetByCallerMagnitudes']
    for tag, expected in [('GASData.Duration', 2), ('GASData.StatusValue', .1)]:
        value = slow_values.get('(TagName="' + tag + '")')
        if isinstance(value, dict):
            value = value.get('SetByCallerMagnitudes')
        check('existing_slow.' + tag, value, expected)
    all_assets += ['/Game/Datatable/' + n for n in ['DT_GASSkillData', 'DT_SkillVariantion', 'DT_JobSkillSet']]
    for path in all_assets:
        # is_dirty compares package names and queries on-disk package data.
        package_path = path.split('.', 1)[0]
        check(package_path + '.saved', call('editor_toolset.toolsets.asset.AssetTools', 'is_dirty', {'asset_path': package_path}), False)
    return {'suite': 'ElementalPack.EditorSmoke', 'passed': all(c['passed'] for c in checks),
            'total': len(checks), 'checks': checks,
            'scope': 'Editor configuration only; C++ build and gameplay execution are separate.'}
