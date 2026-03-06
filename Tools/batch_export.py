import bpy
import os
import sys

class DevNull:
    def write(self, msg): pass
    def flush(self): pass

current_dir = os.getcwd()
blend_files = [f for f in os.listdir(current_dir) if f.endswith(".blend")]

for blend_file in blend_files:
    try:
        old_stdout = sys.stdout
        old_stderr = sys.stderr
        sys.stdout = sys.stderr = DevNull()

        blend_path = os.path.join(current_dir, blend_file)
        bpy.ops.wm.open_mainfile(filepath=blend_path)

        for obj in bpy.data.objects:
            if obj.type != 'MESH': continue
            bpy.context.scene.objects.active = obj
            obj.select = True

            if not obj.animation_data:
                try:
                    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
                except:
                    pass

            if not any(mod.type == 'TRIANGULATE' for mod in obj.modifiers):
                mod = obj.modifiers.new(name="Triangulate", type='TRIANGULATE')
                if not obj.data.shape_keys:
                    try:
                        bpy.ops.object.modifier_apply(modifier=mod.name)
                    except:
                        pass

        # Export settings (exactly what you asked for)
        base_name = os.path.splitext(blend_file)[0]
        obj_path = os.path.join(current_dir, base_name + ".obj")
        has_animation = bpy.context.scene.frame_end > bpy.context.scene.frame_start + 1

        bpy.ops.export_scene.obj(
            filepath=obj_path,
            use_selection=False,
            use_apply_modifiers=True,
            use_animation=has_animation,
            use_normals=True,
            use_uvs=True,
            use_materials=False,
            use_triangles=True,
            use_nurbs=False,
            use_blen_objects=False,
            group_by_object=False,
            keep_vertex_order=True,
            global_scale=1.0,
            axis_forward='-Y',
            axis_up='Z'
        )

        sys.stdout = old_stdout
        sys.stderr = old_stderr

        if has_animation:
            print("{} -> {}.obj (animated sequence)".format(blend_file, base_name))
        else:
            print("{} -> {}.obj".format(blend_file, base_name))

    except Exception:
        sys.stdout = old_stdout
        sys.stderr = old_stderr
        # silent fail (no crash)

print("Done.")
