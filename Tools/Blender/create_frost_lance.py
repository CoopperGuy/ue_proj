"""Blender 3.4+: generate the Frost Lance mesh, source scene, and UI renders.

Run with blender --background --python this_file.py -- --output <directory>.
The FBX uses centimeters and points along +X in the source scene.
"""
import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def material(name, color, emission=0.0, metallic=0.2):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1)
    mat.use_nodes = True
    shader = mat.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Base Color'].default_value = (*color, 1)
    shader.inputs['Metallic'].default_value = metallic
    shader.inputs['Roughness'].default_value = 0.22
    shader.inputs['Emission'].default_value = (*color, 1)
    shader.inputs['Emission Strength'].default_value = emission
    return mat


def crystal(name, length, radius, location, rotation, mats):
    sides = 6
    vertices = [(-length * 0.5, 0, 0)]
    for x, scale, twist in [(-length * 0.31, 0.65, 0), (length * 0.04, 1, 0.15)]:
        for i in range(sides):
            angle = i * math.tau / sides + twist
            vertices.append((x, radius * scale * math.cos(angle), radius * scale * math.sin(angle)))
    vertices.append((length * 0.5, 0, 0))
    faces = []
    for i in range(sides):
        j = (i + 1) % sides
        faces.extend([(0, j + 1, i + 1), (i + 1, j + 1, j + 7),
                      (i + 1, j + 7, i + 7), (i + 7, j + 7, 13)])
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    obj.location = location
    obj.rotation_euler = rotation
    for mat in mats:
        mesh.materials.append(mat)
    for p in mesh.polygons:
        p.material_index = (p.index // 4) % len(mats)
    return obj


def aim(obj, point=(0, 0, 0)):
    obj.rotation_euler = (Vector(point) - obj.location).to_track_quat('-Z', 'Y').to_euler()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', required=True)
    args = parser.parse_args(sys.argv[sys.argv.index('--') + 1:])
    out = Path(args.output).resolve()
    out.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.scale_length = 0.01
    ice = material('IceFacet', (0.025, 0.32, 0.55), 0.12)
    edge = material('IceEdge', (0.15, 0.78, 0.95), 0.4)
    core = material('IceCore', (0.52, 0.94, 1.0), 2.5)
    parts = [crystal('LanceBody', 160, 11, (0, 0, 0), (0, 0, 0), [ice, edge, core])]
    for i in range(4):
        a = i * math.tau / 4
        parts.append(crystal('CrownShard', 65, 5.5,
                             (-34, 12 * math.cos(a), 12 * math.sin(a)),
                             (a, -0.13 * math.sin(a), 0.13 * math.cos(a)), [ice, edge, core]))
    bpy.ops.object.select_all(action='DESELECT')
    for obj in parts:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    lance = bpy.context.object
    lance.name = 'SM_FrostLance'
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.uv.smart_project(island_margin=0.03)
    bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.export_scene.fbx(filepath=str(out / 'SM_FrostLance.fbx'), use_selection=True,
                             object_types={'MESH'}, axis_forward='-Y', axis_up='Z',
                             apply_unit_scale=True, bake_anim=False, use_mesh_modifiers=True)
    scene.render.engine = 'BLENDER_EEVEE'
    scene.eevee.use_gtao = True
    scene.eevee.use_bloom = True
    scene.eevee.bloom_intensity = 0.12
    scene.eevee.taa_render_samples = 64
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'
    scene.render.film_transparent = False
    scene.world = bpy.data.worlds.new('FrostNight')
    scene.world.use_nodes = True
    scene.world.node_tree.nodes['Background'].inputs[0].default_value = (0.003, 0.009, 0.025, 1)
    scene.world.node_tree.nodes['Background'].inputs[1].default_value = 0.35
    scene.view_settings.view_transform = 'Standard'
    scene.view_settings.look = 'Medium High Contrast'
    bpy.ops.object.camera_add(location=(105, -270, 190))
    cam = bpy.context.object
    cam.data.type = 'ORTHO'
    cam.data.ortho_scale = 205
    aim(cam)
    scene.camera = cam
    for loc, power, color, size in [((40, -110, 140), 200000, (0.6, 0.9, 1), 100),
                                   ((-60, 90, 70), 280000, (0.15, 0.4, 1), 85)]:
        bpy.ops.object.light_add(type='AREA', location=loc)
        lamp = bpy.context.object
        lamp.data.energy = power
        lamp.data.color = color
        lamp.data.shape = 'DISK'
        lamp.data.size = size
        aim(lamp)
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 1024
    scene.render.resolution_percentage = 100
    bpy.ops.wm.save_as_mainfile(filepath=str(out / 'FrostLance.blend'))
    scene.render.filepath = str(out / 'FrostLance_Preview.png')
    bpy.ops.render.render(write_still=True)
    scene.render.resolution_x = scene.render.resolution_y = 512
    scene.render.filepath = str(out / 'T_FrostLance.png')
    bpy.ops.render.render(write_still=True)
    variants = []
    for i, (pos, angle) in enumerate([((0, -31, 0), -0.22), ((0, 31, 0), 0.22)]):
        obj = bpy.data.objects.new('ForkShard' + str(i), lance.data)
        bpy.context.collection.objects.link(obj)
        obj.scale = (0.62, 0.62, 0.62)
        obj.location = pos
        obj.rotation_euler.z = angle
        variants.append(obj)
    lance.scale = (0.8, 0.8, 0.8)
    scene.render.filepath = str(out / 'T_FrostLance_Fork.png')
    bpy.ops.render.render(write_still=True)
    for obj in variants:
        obj.hide_render = True
    lance.scale = (1, 1, 1)
    rings = []
    for x in [-36, 8, 44]:
        bpy.ops.mesh.primitive_torus_add(major_radius=24, minor_radius=0.9,
                                        major_segments=32, minor_segments=6,
                                        location=(x, 0, 0), rotation=(0, math.pi / 2, 0))
        obj = bpy.context.object
        obj.data.materials.append(core)
        rings.append(obj)
    scene.render.filepath = str(out / 'T_FrostLance_Pierce.png')
    bpy.ops.render.render(write_still=True)
    for obj in rings:
        obj.hide_render = True
    lance.hide_render = True
    for i in range(6):
        angle = i * math.tau / 6
        crystal('FreezeRay', 110, 6, (0, 0, 0), (0, 0, angle), [edge, core])
    cam.location = (0, -25, 270)
    cam.data.ortho_scale = 150
    aim(cam)
    scene.render.filepath = str(out / 'T_FrostLance_Freeze.png')
    bpy.ops.render.render(write_still=True)
    info = {'mesh': 'SM_FrostLance', 'triangles': len(lance.data.polygons),
            'vertices': len(lance.data.vertices), 'length_cm': 160,
            'skill_id': 1012, 'variant_ids': [21, 22, 23]}
    (out / 'model_manifest.json').write_text(json.dumps(info, indent=2), encoding='utf-8')
    print(json.dumps(info))


if __name__ == '__main__':
    main()
