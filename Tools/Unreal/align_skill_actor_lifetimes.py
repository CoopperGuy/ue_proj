"""Align three existing Blueprint actor defaults with their unchanged Step data."""
import json
from native_mcp_client import UnrealMCP
from register_variant_expansion import OUT, load


def main():
    c = UnrealMCP()
    if c.call('EditorToolset.EditorAppToolset', 'IsPIERunning', {}):
        raise RuntimeError('Stop PIE before updating actor defaults')
    records = []
    table = load('tables-after.json')['DT_GASSkillData']
    for name in ['LavaEruption', 'ArcScatter', 'FrostRush']:
        step = table[name]['steps'][-1]
        duration = step['ProjectileData' if name == 'ArcScatter' else 'GroundData']['LifeSpan']
        path = '/Game/Skills/' + name + '/BP_' + name
        ref = {'refPath': path + '.BP_' + name}
        group = 'editor_toolset.toolsets.object.ObjectTools'
        before = json.loads(c.call(group, 'get_properties', {'instance': ref, 'properties': ['lifeSpan']}))
        assert c.call(group, 'set_properties', {'instance': ref, 'values': json.dumps({'lifeSpan': duration})})
        c.call('editor_toolset.toolsets.blueprint.BlueprintTools', 'compile_blueprint', {'blueprint': ref, 'warnings_as_errors': True})
        after = json.loads(c.call(group, 'get_properties', {'instance': ref, 'properties': ['lifeSpan']}))
        assert abs(after['lifeSpan'] - duration) < .0001, after
        assert c.call('editor_toolset.toolsets.asset.AssetTools', 'save_assets', {'asset_paths': [path]})
        records.append({'asset': path, 'before': before, 'after': after})
        print(json.dumps(records[-1]), flush=True)
    (OUT / 'actor-lifetime-alignment.json').write_text(json.dumps(records, indent=2), encoding='utf-8')


if __name__ == '__main__':
    main()
