import bpy
import os

current_dir = os.getcwd()
blend_files = [f for f in os.listdir(current_dir) if f.endswith(".blend")]

for blend_file in blend_files:
    try:
        blend_path = os.path.join(current_dir, blend_file)
        bpy.ops.wm.open_mainfile(filepath=blend_path)

        # Delete all objects using data API (no context issues)
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete(use_global=False)

        # Re-load blend file again to get original contents after delete
        bpy.ops.wm.open_mainfile(filepath=blend_path)

        for obj in bpy.data.objects:
            if obj.type == 'MESH':
                bpy.context.scene.objects.active = obj
                obj.select = True
                # Safe transform apply
                if not obj.animation_data:
                    try:
                        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
                    except:
                        pass

                # Triangulate modifier
                if not any(mod.type == 'TRIANGULATE' for mod in obj.modifiers):
                    mod = obj.modifiers.new(name="Triangulate", type='TRIANGULATE')
                    if not obj.data.shape_keys:
                        try:
                            bpy.ops.object.modifier_apply(modifier=mod.name)
                        except:
                            pass

        # Export FBX
        fbx_path = os.path.join(current_dir, os.path.splitext(blend_file)[0] + ".fbx")
        bpy.ops.export_scene.fbx(
            filepath=fbx_path,
            use_selection=False,
            global_scale=1.0,
            axis_forward='-Z',
            axis_up='Y',
            use_mesh_modifiers=True,
            mesh_smooth_type='OFF'
        )

        print("{} -> {}.fbx".format(blend_file, os.path.splitext(blend_file)[0]))

    except Exception as e:
        print("Failed {}: {}".format(blend_file, str(e)))

print("Done.")
