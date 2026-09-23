"""Create expansion assets through native MCP; never write binary assets directly."""
import copy
import json
from pathlib import Path
from native_mcp_client import UnrealMCP

ROOT = '/Game/Skills/Variants'
OUT = Path(__file__).resolve().parents[2] / 'docs/VariantExpansion'
A = 'editor_toolset.toolsets.asset.AssetTools'
O = 'editor_toolset.toolsets.object.ObjectTools'
B = 'editor_toolset.toolsets.blueprint.BlueprintTools'


def main():
    c = UnrealMCP()
    if c.call('EditorToolset.EditorAppToolset', 'IsPIERunning', {}):
        raise RuntimeError('Stop PIE before asset setup')
    tags = c.call('GameplayTagsToolset.GameplayTagsToolset', 'ListTags', {'parentTag': 'GASData.Variant'})
    for name in ['CastSelfEffect', 'CastGround']:
        tag = 'GASData.Variant.' + name
        if tag not in tags:
            c.call('GameplayTagsToolset.GameplayTagsToolset', 'AddTag',
                   {'tagName': tag, 'tagSource': '', 'comment': 'Step variant expansion: ' + name})
    template = json.loads((OUT / 'effect-template.json').read_text(encoding='utf-8'))
    saved = []
    for name, attribute, value, duration in [('Haste', 'MovementSpeed', 120, 2),
                                             ('Ward', 'Defense', 20, 3),
                                             ('Mana', 'Mana', 12, 0)]:
        path = ROOT + '/GE_Variant_' + name
        if not c.call(A, 'exists', {'path': path}):
            c.call(A, 'duplicate', {'path': '/Game/Skills/FrostRush/GE_FrostRush_Slow', 'new_path': path})
        props = copy.deepcopy(template)
        props['durationPolicy'] = 'HasDuration' if duration else 'Instant'
        props['durationMagnitude']['scalableFloatMagnitude']['value'] = duration
        if not duration:
            props['stackingType'] = 'None'
        modifier = props['modifiers'][0]
        modifier['attribute'] = {'attributeName': attribute, 'attribute': '/Script/ARPG.DiaAttributeSet:' + attribute,
                                 'attributeOwner': {'refPath': '/Script/ARPG.DiaAttributeSet'}}
        modifier['modifierOp'] = 'Additive'
        modifier['modifierMagnitude']['scalableFloatMagnitude']['value'] = value
        props['gEComponents'] = []
        ref = {'refPath': path + '.GE_Variant_' + name}
        assert c.call(O, 'set_properties', {'instance': ref, 'values': json.dumps(props)})
        c.call(B, 'compile_blueprint', {'blueprint': ref, 'warnings_as_errors': True})
        read = json.loads(c.call(O, 'get_properties', {'instance': ref, 'properties': list(props)}))
        assert read['modifiers'][0]['attribute']['attributeName'] == attribute
        assert read['modifiers'][0]['modifierMagnitude']['scalableFloatMagnitude']['value'] == value
        assert read['gEComponents'] == []
        assert c.call(A, 'save_assets', {'asset_paths': [path]})
        saved.append(path)
        print('Saved ' + path, flush=True)
    path = ROOT + '/GE_Variant_ChillStack'
    if not c.call(A, 'exists', {'path': path}):
        c.call(A, 'duplicate', {'path': '/Game/Skills/FrostRush/GE_FrostRush_Slow', 'new_path': path})
    props = copy.deepcopy(template)
    props.update(modifiers=[], gEComponents=[], stackingType='AggregateByTarget', stackLimitCount=2,
                 bDenyOverflowApplication=True, bClearStackOnOverflow=True,
                 stackDurationRefreshPolicy='RefreshOnSuccessfulApplication', stackExpirationPolicy='ClearEntireStack',
                 overflowEffects=[{'refPath': '/Game/Skills/FrostRush/GE_FrostRush_Freeze.GE_FrostRush_Freeze_C'}])
    props['durationMagnitude']['scalableFloatMagnitude']['value'] = 3
    ref = {'refPath': path + '.GE_Variant_ChillStack'}
    assert c.call(O, 'set_properties', {'instance': ref, 'values': json.dumps(props)})
    c.call(B, 'compile_blueprint', {'blueprint': ref, 'warnings_as_errors': True})
    assert c.call(A, 'save_assets', {'asset_paths': [path]})
    saved.append(path)
    for name, source in [('EmberField', 'LavaEruption'), ('RimeField', 'FrostRush')]:
        path = ROOT + '/BP_Variant_' + name
        if not c.call(A, 'exists', {'path': path}):
            c.call(A, 'duplicate', {'path': '/Game/Skills/' + source + '/BP_' + source, 'new_path': path})
        ref = {'refPath': path + '.BP_Variant_' + name}
        assert c.call(O, 'set_properties', {'instance': ref, 'values': json.dumps({'radius': 200, 'lifeSpan': 3})})
        for suffix in ['SkillObjectMesh', 'SkillEffectComp']:
            comp = path + '.Default__BP_Variant_' + name + '_C:' + suffix
            assert c.call(O, 'set_properties', {'instance': {'refPath': comp},
                                               'values': json.dumps({'relativeLocation': {'x': 0, 'y': 0, 'z': 0}})})
        c.call(B, 'compile_blueprint', {'blueprint': ref, 'warnings_as_errors': True})
        assert c.call(A, 'save_assets', {'asset_paths': [path]})
        saved.append(path)
        print('Saved ' + path, flush=True)
    (OUT / 'assets.json').write_text(json.dumps(saved, indent=2), encoding='utf-8')


if __name__ == '__main__':
    main()
