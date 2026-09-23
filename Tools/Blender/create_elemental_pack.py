"""Generate three original skill meshes, editable Blender scenes and 12 UI icons.

blender --background --python create_elemental_pack.py -- --output <directory>
"""
import argparse
import math
import sys
from pathlib import Path

import bpy

sys.path.insert(0, str(Path(__file__).parent))
from create_frost_lance import aim, crystal, material


PALETTES = {
    'LavaEruption': [(0.055, 0.009, 0.004), (0.9, 0.07, 0.005), (1, 0.5, 0.025)],
    'ArcScatter': [(0.025, 0.02, 0.2), (0.22, 0.13, 0.95), (0.35, 0.85, 1)],
    'FrostRush': [(0.015, 0.14, 0.22), (0.025, 0.58, 0.7), (0.5, 0.95, 0.85)],
}
VARIANTS = {'LavaEruption': ['Area', 'Cooldown', 'Mana'],
            'ArcScatter': ['Multiple', 'Pierce', 'Fork'],
            'FrostRush': ['Area', 'Freeze', 'Cooldown']}


def ring(radius, tube, mat, z=3):
    bpy.ops.mesh.primitive_torus_add(major_radius=radius, minor_radius=tube,
                                    major_segments=48, minor_segments=6, location=(0, 0, z))
    obj = bpy.context.object
    obj.data.materials.append(mat)
    return obj


def bolt(mats):
    outline = [(-70, -7), (-14, 6), (-32, 23), (72, 0), (9, -8), (27, -26)]
    verts = [(x, y, z) for z in [-5, 5] for x, y in outline]
    faces = [tuple(range(5, -1, -1)), tuple(range(6, 12))]
    faces += [(i, (i+1) % 6, (i+1) % 6+6, i+6) for i in range(6)]
    mesh = bpy.data.meshes.new('LightningBolt')
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new('LightningBolt', mesh)
    bpy.context.collection.objects.link(obj)
    for mat in mats:
        mesh.materials.append(mat)
    for face in mesh.polygons:
        face.material_index = 2 if face.index < 2 else 1
    return obj


def generate(name, out):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.scale_length = .01
    mats = [material(slot, color, [0.05, 0.35, 2.0][i])
            for i, (slot, color) in enumerate(zip(['Facet', 'Edge', 'Core'], PALETTES[name]))]
    if name == 'ArcScatter':
        parts = [bolt(mats)]
        for y in [-22, 22]:
            parts.append(crystal('ArcSatellite', 68, 4, (-25, y, 0), (0, 0, y * .008), mats))
        frame = 200
    else:
        lava = name == 'LavaEruption'
        radius = 240 if lava else 220
        parts = [ring(radius, 3, mats[2]), ring(radius*.68, 2, mats[1])]
        if lava:
            bpy.ops.mesh.primitive_cone_add(vertices=32, radius1=radius*.94,
                                            radius2=radius*.78, depth=14, location=(0, 0, 4))
            base = bpy.context.object
            base.data.materials.append(mats[0])
            parts.append(base)
        for i in range(12):
            a = i*math.tau/12
            r = radius*(.65 if i%2 else .83)
            height = (80 if lava else 100) + 35*(i%3)
            parts.append(crystal('Obsidian' if lava else 'IceBlade', height, 15 if lava else 12,
                                 (r*math.cos(a), r*math.sin(a), height*.35),
                                 (0, -math.pi/2+.2, a), mats))
        parts.append(crystal('Heart', 150, 24, (0, 0, 55), (0, -math.pi/2, 0), mats))
        frame = 620
    bpy.ops.object.select_all(action='DESELECT')
    for part in parts:
        part.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    obj.name = 'SM_' + name
    scene.cursor.location = (0, 0, 0)
    bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.uv.smart_project(island_margin=.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    out.mkdir(parents=True, exist_ok=True)
    bpy.ops.export_scene.fbx(filepath=str(out / ('SM_'+name+'.fbx')), use_selection=True,
                             object_types={'MESH'}, axis_forward='-Y', axis_up='Z',
                             apply_unit_scale=True, bake_anim=False)
    scene.render.engine = 'BLENDER_EEVEE'
    scene.eevee.use_gtao = True
    scene.eevee.use_bloom = True
    scene.eevee.bloom_intensity = .08
    scene.eevee.taa_render_samples = 48
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'
    scene.world = bpy.data.worlds.new('Night')
    scene.world.use_nodes = True
    scene.world.node_tree.nodes['Background'].inputs[0].default_value = (.003, .006, .015, 1)
    scene.world.node_tree.nodes['Background'].inputs[1].default_value = .3
    scene.view_settings.view_transform = 'Standard'
    scene.view_settings.look = 'Medium High Contrast'
    bpy.ops.object.camera_add(location=(frame*.25, -frame, frame*.85))
    cam = bpy.context.object
    cam.data.type = 'ORTHO'
    cam.data.ortho_scale = frame
    aim(cam, (0, 0, 25 if name != 'ArcScatter' else 0))
    scene.camera = cam
    for loc, power in [((frame*.2, -frame*.3, frame*.7), 800000),
                       ((-frame*.3, frame*.4, frame*.3), 600000)]:
        bpy.ops.object.light_add(type='AREA', location=loc)
        lamp = bpy.context.object
        lamp.data.energy = power
        lamp.data.size = frame*.5
        aim(lamp)
    scene.render.resolution_x = scene.render.resolution_y = 768
    scene.render.resolution_percentage = 100
    bpy.ops.wm.save_as_mainfile(filepath=str(out / (name+'.blend')))
    scene.render.filepath = str(out / (name+'_Preview.png'))
    bpy.ops.render.render(write_still=True)
    scene.render.resolution_x = scene.render.resolution_y = 512
    scene.render.filepath = str(out / ('T_'+name+'.png'))
    bpy.ops.render.render(write_still=True)
    for variant in VARIANTS[name]:
        extras = []
        if variant in ['Multiple', 'Fork']:
            obj.scale = (.65,)*3
            for y in [-40, 40]:
                extra = bpy.data.objects.new('Split', obj.data)
                bpy.context.collection.objects.link(extra)
                extra.scale = (.58,)*3
                extra.location.y = y
                extra.rotation_euler.z = y*.006
                extras.append(extra)
        elif variant == 'Pierce':
            for x in [-35, 15, 50]:
                extra = ring(24, 1.3, mats[2], 0)
                extra.rotation_euler.y = math.pi/2
                extra.location.x = x
                extras.append(extra)
        else:
            for k in range(2 if variant == 'Area' else 1):
                extras.append(ring(frame*(.39+.04*k), 3, mats[2], 16+15*k))
            if variant in ['Cooldown', 'Mana', 'Freeze']:
                for i in range(3 if variant == 'Cooldown' else 6):
                    a = i*math.tau/(3 if variant == 'Cooldown' else 6)
                    extras.append(crystal('Rune', frame*.2, 8,
                                          (math.cos(a)*frame*.28, math.sin(a)*frame*.28, 130),
                                          (0, 0, a), [mats[2]]))
        scene.render.filepath = str(out / ('T_'+name+'_'+variant+'.png'))
        bpy.ops.render.render(write_still=True)
        obj.scale = (1,)*3
        for extra in extras:
            bpy.data.objects.remove(extra, do_unlink=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', required=True)
    args = parser.parse_args(sys.argv[sys.argv.index('--')+1:])
    for skill in PALETTES:
        generate(skill, Path(args.output).resolve() / skill)
