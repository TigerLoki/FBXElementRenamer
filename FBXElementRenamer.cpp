#include <fbxsdk.h>
#include <iostream>
#include <string>
#include <vector>
#include <regex> // Added for regex functionality

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(sdkManager->GetIOSettings()))
#endif

void PrintHelp(const char* programName) {
    std::cout << "Usage: " << programName << " -i <input_fbx> [-o <output_fbx>] [-mat <old_mat_name> <new_mat_name>] [-mesh <old_mesh_name> <new_mesh_name>] [-atf] [-regex] [-h | --help]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -i <input_fbx>                        Specify the input FBX file.\n";
    std::cout << "  -o <output_fbx>                       Specify the output FBX file. If not specified, overwrites the input file.\n";
    std::cout << "  -mat <old_mat_name> <new_mat_name>    Rename a material from old name to new name.\n";
    std::cout << "  -mesh <old_mesh_name> <new_mesh_name> Rename meshes that contain <old_mesh_name> (or match regex if -regex is used) to have it replaced with <new_mesh_name>.\n";
    std::cout << "  -atf                                  Convert FBX file from ASCII to binary format.\n";
    std::cout << "  -regex                                Treat old_mat_name/old_mesh_name as a regex pattern.\n";
    std::cout << "  -h, --help                            Display this help message.\n";
}

int main(int argc, char** argv)
{
    std::string inputFbx;
    std::string outputFbx;
    bool convertToBinary = false;
    bool useRegex = false;

    struct RenameOperation {
        std::string type; // "material" or "mesh"
        std::string oldName;
        std::string newName;
    };

    std::vector<RenameOperation> renameOps;

    // Parse command-line arguments
    if (argc < 2) { // At least the program name and one argument
        PrintHelp(argv[0]);
        return -1;
    }

    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];
        if ((arg == "-i") && (i + 1 < argc)) {
            inputFbx = argv[++i];
        }
        else if ((arg == "-o") && (i + 1 < argc)) {
            outputFbx = argv[++i];
        }
        else if ((arg == "-mat") && (i + 2 < argc)) {
            RenameOperation op;
            op.type = "material";
            op.oldName = argv[++i];
            op.newName = argv[++i];
            renameOps.push_back(op);
        }
        else if ((arg == "-mesh") && (i + 2 < argc)) {
            RenameOperation op;
            op.type = "mesh";
            op.oldName = argv[++i];
            op.newName = argv[++i];
            renameOps.push_back(op);
        }
        else if (arg == "-atf") {
            convertToBinary = true;
        }
        else if (arg == "-regex") {
            useRegex = true;
        }
        else if (arg == "-h" || arg == "--help") {
            PrintHelp(argv[0]);
            return 0;
        }
        else {
            std::cout << "Unknown or incomplete argument: " << arg << "\n";
            PrintHelp(argv[0]);
            return -1;
        }
        ++i;
    }

    // Validate required arguments
    if (inputFbx.empty()) {
        std::cout << "Error: Input file not specified.\n";
        PrintHelp(argv[0]);
        return -1;
    }

    if (renameOps.empty() && !convertToBinary) {
        std::cout << "Error: No operations specified.\n";
        PrintHelp(argv[0]);
        return -1;
    }

    if (outputFbx.empty()) {
        outputFbx = inputFbx;
    }

    // Create SDK manager
    FbxManager* sdkManager = FbxManager::Create();
    if (!sdkManager) {
        std::cout << "Error: Unable to create FbxManager.\n";
        return -1;
    }

    // IO settings
    FbxIOSettings* ios = FbxIOSettings::Create(sdkManager, IOSROOT);
    sdkManager->SetIOSettings(ios);

    // Create scene
    FbxScene* scene = FbxScene::Create(sdkManager, "My Scene");
    if (!scene) {
        std::cout << "Error: Unable to create FbxScene.\n";
        sdkManager->Destroy();
        return -1;
    }

    // Create importer
    FbxImporter* importer = FbxImporter::Create(sdkManager, "");
    if (!importer->Initialize(inputFbx.c_str(), -1, sdkManager->GetIOSettings())) {
        std::cout << "Error initializing importer: " << importer->GetStatus().GetErrorString() << "\n";
        importer->Destroy();
        sdkManager->Destroy();
        return -1;
    }

    // Import scene
    if (!importer->Import(scene)) {
        std::cout << "Error importing file.\n";
        importer->Destroy();
        sdkManager->Destroy();
        return -1;
    }
    importer->Destroy();

    // Perform rename operations
    for (const auto& op : renameOps) {
        bool found = false;

        if (op.type == "material") {
            int materialCount = scene->GetMaterialCount();
            for (int i = 0; i < materialCount; ++i) {
                FbxSurfaceMaterial* material = scene->GetMaterial(i);
                if (material) {
                    std::string currentName = material->GetName();
                    std::string newName;

                    if (useRegex) {
                        try {
                            std::regex pattern(op.oldName);
                            newName = std::regex_replace(currentName, pattern, op.newName);
                        }
                        catch (std::regex_error& e) {
                            std::cout << "Regex error in material rename: " << e.what() << "\n";
                            continue;
                        }
                    }
                    else {
                        // Non-regex: exact match only
                        if (currentName == op.oldName) {
                            newName = op.newName;
                        }
                        else {
                            newName = currentName;
                        }
                    }

                    if (newName != currentName) {
                        material->SetName(newName.c_str());
                        found = true;
                        std::cout << "Material '" << currentName << "' renamed to '" << newName << "'.\n";
                    }
                }
            }
            if (!found) {
                std::cout << "Material with pattern '" << op.oldName << "' not found.\n";
            }

        }
        else if (op.type == "mesh") {
            int nodeCount = scene->GetNodeCount();
            for (int i = 0; i < nodeCount; ++i) {
                FbxNode* node = scene->GetNode(i);
                if (node && node->GetMesh()) {
                    std::string nodeName = node->GetName();
                    std::string newNodeName;

                    if (useRegex) {
                        try {
                            std::regex pattern(op.oldName);
                            newNodeName = std::regex_replace(nodeName, pattern, op.newName);
                        }
                        catch (std::regex_error& e) {
                            std::cout << "Regex error in mesh rename: " << e.what() << "\n";
                            continue;
                        }
                    }
                    else {
                        // Non-regex: substring find and replace first occurrence
                        size_t pos = nodeName.find(op.oldName);
                        if (pos != std::string::npos) {
                            newNodeName = nodeName;
                            newNodeName.replace(pos, op.oldName.size(), op.newName);
                        }
                        else {
                            newNodeName = nodeName;
                        }
                    }

                    if (newNodeName != nodeName) {
                        node->SetName(newNodeName.c_str());
                        found = true;
                        std::cout << "Mesh '" << nodeName << "' renamed to '" << newNodeName << "'.\n";
                    }
                }
            }
            if (!found) {
                std::cout << "No mesh matching pattern '" << op.oldName << "' found.\n";
            }
        }
    }

    // Create exporter
    FbxExporter* exporter = FbxExporter::Create(sdkManager, "");
    if (!exporter->Initialize(outputFbx.c_str(), -1, sdkManager->GetIOSettings())) {
        std::cout << "Error initializing exporter: " << exporter->GetStatus().GetErrorString() << "\n";
        exporter->Destroy();
        sdkManager->Destroy();
        return -1;
    }

    // Set export format to binary if -atf is specified
    if (convertToBinary) {
        int formatCount = sdkManager->GetIOPluginRegistry()->GetWriterFormatCount();
        int binaryFormatIndex = -1;

        for (int i = 0; i < formatCount; ++i) {
            if (sdkManager->GetIOPluginRegistry()->WriterIsFBX(i)) {
                std::string description = sdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(i);
                if (description.find("binary") != std::string::npos) {
                    binaryFormatIndex = i;
                    break;
                }
            }
        }

        if (binaryFormatIndex != -1) {
            exporter->SetFileExportVersion("FBX_2014_BINARY", FbxSceneRenamer::eNone);
            std::cout << "Exporting in binary format.\n";
        }
        else {
            std::cout << "Binary format not found. Exporting in default format.\n";
        }
    }

    // Export scene
    if (!exporter->Export(scene)) {
        std::cout << "Error exporting file.\n";
        exporter->Destroy();
        sdkManager->Destroy();
        return -1;
    }
    exporter->Destroy();

    std::cout << "Modified file saved as '" << outputFbx << "'.\n";

    // Clean up resources
    sdkManager->Destroy();

    return 0;
}
