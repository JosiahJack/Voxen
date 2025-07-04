import bpy
import os

current_dir = os.getcwd()
obj_files = [f for f in os.listdir(current_dir) if f.endswith(".obj")]

for obj_file in obj_files:
    try:
        # Clear the scene
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete(use_global=False)

        # Import OBJ
        obj_path = os.path.join(current_dir, obj_file)
        bpy.ops.import_scene.obj(filepath=obj_path)

        # Apply transforms and triangulate
        for obj in bpy.context.scene.objects:
            if obj.type == 'MESH':
                bpy.context.scene.objects.active = obj
                obj.select = True

                try:
                    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
                except:
                    pass

                # Add and apply triangulate modifier if needed
                if not any(mod.type == 'TRIANGULATE' for mod in obj.modifiers):
                    mod = obj.modifiers.new(name="Triangulate", type='TRIANGULATE')
                    if not obj.data.shape_keys:
                        try:
                            bpy.ops.object.modifier_apply(modifier=mod.name)
                        except:
                            pass

        # Export FBX
        fbx_name = os.path.splitext(obj_file)[0] + ".fbx"
        fbx_path = os.path.join(current_dir, fbx_name)
        bpy.ops.export_scene.fbx(
            filepath=fbx_path,
            use_selection=False,
            global_scale=1.0,
            axis_forward='-Z',
            axis_up='Y',
            use_mesh_modifiers=True,
            mesh_smooth_type='OFF'
        )

        print("{} -> {}".format(obj_file, fbx_name))

    except Exception as e:
        print("Failed {}: {}".format(obj_file, str(e)))

print("Done.")
