"""Native MCP asset setup script; no DataTable writes."""
import json


def call(group, name, args):
    return execute_tool('editor_toolset.toolsets.' + group + '.' + name,
                        json.dumps(args)).get('returnValue')


def run():
    result = []
    for name, lifetime in [('LavaEruption', 4), ('ArcScatter', 1.2), ('FrostRush', 1.8)]:
        path = '/Game/Skills/' + name + '/BP_' + name
        bp = {'refPath': path + '.BP_' + name}
        assert call('object.ObjectTools', 'set_properties',
                    {'instance': bp, 'values': json.dumps({'lifeSpan': lifetime})})
        call('blueprint.BlueprintTools', 'compile_blueprint',
             {'blueprint': bp, 'warnings_as_errors': True})
        actual = json.loads(call('object.ObjectTools', 'get_properties',
                                 {'instance': bp, 'properties': ['lifeSpan']}))['lifeSpan']
        assert abs(actual - lifetime) < .0001, (name, actual, lifetime)
        assert call('asset.AssetTools', 'save_assets', {'asset_paths': [path]})
        result.append({'name': name, 'lifeSpan': actual, 'saved': True})
    return {'blueprints': result}
