# FBX Renamer Utility

This utility allows you to rename materials and meshes within an FBX file. You can specify the input file, output file, and the materials or meshes you wish to rename.

**Usage:**

```FBXElementRenamer.exe -i <input_fbx> [-o <output_fbx>] [-mat <old_mat_name> <new_mat_name>] [-mesh <old_mesh_name> <new_mesh_name>] [-h | --help]```

**Options:**

- `-i <input_fbx>`
  - Specify the input FBX file. **(Required)**

- `-o <output_fbx>`
  - Specify the output FBX file. If not specified, the input file will be overwritten.

- `-mat <old_mat_name> <new_mat_name>`
  - Rename a material from the old name to the new name. You can use this option multiple times to rename multiple materials.

- `-mesh <old_mesh_name> <new_mesh_name>`
  - Rename a mesh from the old name to the new name. You can use this option multiple times to rename multiple meshes.

- `-h`, `--help`
  - Display the help message.

**Examples:**

1. **Rename a material:**
   
   ```fbx_renamer -i model.fbx -mat "OldMaterialName" "NewMaterialName"```
2. **Rename multiple materials:**

   ```fbx_renamer -i model.fbx -mat "OldMat1" "NewMat1" -mat "OldMat2" "NewMat2"```

4. **Rename a mesh and save to a new file:**

   ```fbx_renamer -i model.fbx -o new_model.fbx -mesh "OldMeshName" "NewMeshName"```

**Notes:**

- If you do not specify the `-o` option, the program will overwrite the input FBX file with the changes.
- Ensure that the old material or mesh names you provide exactly match those in the FBX file.
- You can combine multiple `-mat` and `-mesh` options in a single command to perform multiple renames at once.
