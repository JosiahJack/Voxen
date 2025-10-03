#!/usr/bin/env python3
import bpy
import os
import struct
import bmesh

def write_vnuv_vti_files(vnuv_filepath, vti_filepath, objects):
    """
    Export mesh data to two binary files:
    - .vnuv: Raw floats for all vertices (x, y, z, nx, ny, nz, u, v) across all meshes.
    - .vti: Raw uint32_t for all triangle indices, adjusted for global vertex offsets.
    """
    vertex_data = []
    triangle_data = []
    vertex_offset = 0

    mesh_objects = [obj for obj in objects if obj.type == 'MESH']
    for obj in mesh_objects:
        try:
            # Get evaluated mesh with modifiers applied
            mesh = obj.to_mesh(bpy.context.scene, True, 'RENDER')

            # Ensure the mesh is triangulated
            if not all(len(face.vertices) == 3 for face in mesh.polygons):
                temp_mesh = mesh.copy()
                bm = bmesh.new()
                bm.from_mesh(temp_mesh)
                bmesh.ops.triangulate(bm, faces=bm.faces)
                bm.to_mesh(temp_mesh)
                bm.free()
                mesh = temp_mesh

            # Validate UV map
            if not mesh.uv_layers or not mesh.uv_layers.active:
                raise ValueError("Mesh {0} has no valid UV map".format(obj.name))

            # Collect vertex data
            mesh.calc_normals()
            uv_layer = mesh.uv_layers.active.data
            for vert in mesh.vertices:
                co = vert.co
                normal = vert.normal
                uvs = [uv_layer[loop_index].uv for loop_index in range(len(mesh.loops))
                       if mesh.loops[loop_index].vertex_index == vert.index]
                if not uvs:
                    raise ValueError("No UV data for vertex {0} in mesh {1}".format(vert.index, obj.name))
                uv = uvs[0]
                vertex_data.extend([co.x, co.y, co.z, normal.x, normal.y, normal.z, uv[0], uv[1]])

            # Collect triangle indices, adjusted for global offset
            for poly in mesh.polygons:
                if len(poly.vertices) != 3:
                    raise ValueError("Non-triangular face detected in mesh {0}".format(obj.name))
                triangle_data.extend([v + vertex_offset for v in poly.vertices])

            print("Exported mesh {0}: {1} vertices, {2} triangles".format(obj.name, len(mesh.vertices), len(mesh.polygons)))
            vertex_offset += len(mesh.vertices)

            # Clean up
            if 'temp_mesh' in locals():
                bpy.data.meshes.remove(temp_mesh)
            bpy.data.meshes.remove(mesh)

        except Exception as e:
            print("Failed to process mesh {0}: {1}".format(obj.name, str(e)))
            import traceback
            traceback.print_exc()
            continue

    # Write .vnuv file (raw floats)
    with open(vnuv_filepath, 'wb') as f:
        vertex_bytes = struct.pack('<{0}f'.format(len(vertex_data)), *vertex_data)
        f.write(vertex_bytes)
        print("Wrote {0} floats ({1} bytes) to {2}".format(len(vertex_data), len(vertex_bytes), vnuv_filepath))

    # Write .vti file (raw uint32_t)
    with open(vti_filepath, 'wb') as f:
        triangle_bytes = struct.pack('<{0}I'.format(len(triangle_data)), *triangle_data)
        f.write(triangle_bytes)
        print("Wrote {0} indices ({1} bytes) to {2}".format(len(triangle_data), len(triangle_bytes), vti_filepath))

current_dir = os.getcwd()
blend_files = sorted([f for f in os.listdir(current_dir) if f.endswith(".blend")])  # Sort alphabetically

print("Found {0} .blend files: {1}".format(len(blend_files), blend_files))

for blend_file in blend_files:
    try:
        blend_path = os.path.join(current_dir, blend_file)
        print("Processing {0}".format(blend_file))
        bpy.ops.wm.open_mainfile(filepath=blend_path)

        # Ensure all objects are in object mode
        for obj in bpy.data.objects:
            if obj.mode != 'OBJECT':
                bpy.ops.object.mode_set(mode='OBJECT')

        # Apply transforms and triangulate
        for obj in bpy.data.objects:
            if obj.type == 'MESH':
                bpy.context.scene.objects.active = obj  # Blender 2.76 API
                obj.select = True
                # Apply transforms
                if not obj.animation_data:
                    try:
                        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
                    except Exception as e:
                        print("Failed to apply transforms to {0}: {1}".format(obj.name, str(e)))
                # Triangulate modifier
                if not any(mod.type == 'TRIANGULATE' for mod in obj.modifiers):
                    mod = obj.modifiers.new(name="Triangulate", type='TRIANGULATE')
                    if not obj.data.shape_keys:
                        try:
                            bpy.ops.object.modifier_apply(modifier=mod.name)
                        except Exception as e:
                            print("Failed to apply triangulate modifier to {0}: {1}".format(obj.name, str(e)))
                obj.select = False

        # Export to .vnuv and .vti
        base_name = os.path.splitext(blend_file)[0]
        vnuv_path = os.path.join(current_dir, "{0}.vnuv".format(base_name))
        vti_path = os.path.join(current_dir, "{0}.vti".format(base_name))
        write_vnuv_vti_files(vnuv_path, vti_path, bpy.data.objects)

        print("{0} -> {1}, {2}".format(blend_file, base_name + ".vnuv", base_name + ".vti"))

    except Exception as e:
        print("Failed {0}: {1}".format(blend_file, str(e)))
        import traceback
        traceback.print_exc()

    # Clear data to avoid memory issues
    bpy.ops.wm.memory_statistics()
    bpy.ops.wm.read_factory_settings()

print("Done.")
