"""Run with Python outside Unreal. Uses sequential native MCP tool calls."""
import copy
from datetime import datetime, timezone
import json
from pathlib import Path

from native_mcp_client import UnrealMCP

DATA = Path(__file__).resolve().parents[2] / 'docs' / 'ElementalPack'
DT = 'editor_toolset.toolsets.data_table.DataTableTools'
ASSETS = 'editor_toolset.toolsets.asset.AssetTools'


def ref(name):
    return {'refPath': '/Game/Datatable/' + name + '.' + name}


def read(client, name):
    table = ref(name)
    names = client.call(DT, 'list_rows', {'data_table': table})
    return json.loads(client.call(DT, 'get_rows', {'data_table': table, 'row_names': names}))


def normalize_maps(value):
    if isinstance(value, list):
        for child in value:
            normalize_maps(child)
    elif isinstance(value, dict):
        for key, child in value.items():
            if key == 'SetByCallerMagnitudes' and isinstance(child, dict):
                for tag, magnitude in list(child.items()):
                    if isinstance(magnitude, dict) and set(magnitude) == {'SetByCallerMagnitudes'}:
                        child[tag] = magnitude['SetByCallerMagnitudes']
            else:
                normalize_maps(child)


def main():
    client = UnrealMCP()
    if client.call('EditorToolset.EditorAppToolset', 'IsPIERunning', {}):
        raise RuntimeError('Stop PIE before updating the skill tables')
    skills = json.loads((DATA / 'skills.json').read_text(encoding='utf-8'))
    variants = json.loads((DATA / 'variants.json').read_text(encoding='utf-8'))
    names = ['DT_GASSkillData', 'DT_SkillVariantion', 'DT_JobSkillSet']
    before = {name: read(client, name) for name in names}
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    (DATA / ('tables-before-registration-' + stamp + '.json')).write_text(
        json.dumps(before, ensure_ascii=False, indent=2), encoding='utf-8')
    for name, desired, field in [(names[0], skills, 'skillId'), (names[1], variants, 'variantId')]:
        occupied = {r[field] for key, r in before[name].items() if key not in desired}
        for key, row in desired.items():
            if row[field] in occupied:
                raise ValueError('ID conflict: ' + key)
            if key in before[name] and before[name][key][field] not in (0, row[field]):
                raise ValueError('Row name conflict: ' + key)
    for name in skills:
        if not client.call(ASSETS, 'exists', {'path': '/Game/Skills/' + name + '/BP_' + name}):
            raise ValueError('Missing skill blueprint: ' + name)
    tags = client.call('GameplayTagsToolset.GameplayTagsToolset', 'ListTags', {'parentTag': 'GASData.CoolDown'})
    for row in skills.values():
        if row['coolDownTags']['TagName'] not in tags:
            raise ValueError('Missing cooldown tag: ' + row['coolDownTags']['TagName'])

    for name, desired in [(names[0], skills), (names[1], variants)]:
        patch = copy.deepcopy(desired)
        if name == names[1]:
            for key, row in before[name].items():
                if key not in patch and row.get('variantExtraData'):
                    extra = copy.deepcopy(row['variantExtraData'])
                    normalize_maps(extra)
                    patch[key] = {'variantExtraData': extra}
        missing = [key for key in desired if key not in before[name]]
        if missing:
            client.call(DT, 'add_rows', {'data_table': ref(name), 'row_names': missing})
        client.call(DT, 'set_rows', {'data_table': ref(name), 'values': json.dumps(patch)})
        current = read(client, name)
        changed = [key for key, row in before[name].items() if key not in desired and current.get(key) != row]
        if changed:
            raise RuntimeError('Existing rows changed; not saving: ' + repr(changed))
        if not client.call(ASSETS, 'save_assets', {'asset_paths': ['/Game/Datatable/' + name]}):
            raise RuntimeError('Save failed: ' + name)
        print(json.dumps({'saved': name, 'new_rows': list(desired), 'preserved_rows': len(before[name])}), flush=True)
    ids = list(before[names[2]]['WarriorSkillSet']['skillIds'])
    for row in skills.values():
        if row['skillId'] not in ids:
            ids.append(row['skillId'])
    client.call(DT, 'set_rows', {'data_table': ref(names[2]),
                               'values': json.dumps({'WarriorSkillSet': {'skillIds': ids}})})
    if not client.call(ASSETS, 'save_assets', {'asset_paths': ['/Game/Datatable/' + names[2]]}):
        raise RuntimeError('Job table save failed')
    after = {name: read(client, name) for name in names}
    (DATA / 'tables-after-registration.json').write_text(
        json.dumps(after, ensure_ascii=False, indent=2), encoding='utf-8')
    result = {'registered_skill_ids': [r['skillId'] for r in skills.values()],
              'registered_variant_ids': [r['variantId'] for r in variants.values()],
              'warrior_skill_ids': ids, 'tables_saved': names,
              'existing_skill_and_variant_rows_preserved': True}
    (DATA / 'registration-result.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps(result), flush=True)


if __name__ == '__main__':
    main()
