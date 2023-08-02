#include "forest.h"

std::string FormatNumber(int number, int width)
{
    // Add leading zeros to number to be at least width chars wide
    auto targetSize = std::string(width, '0').length();
    std::string str = std::to_string(number);
    return std::string(width - std::min(targetSize, str.length()), '0') + str;
}

void Forest::Configure(const gz::sim::Entity &_entity,
                                   const std::shared_ptr<const sdf::Element> &_sdf,
                                   gz::sim::EntityComponentManager &_ecm,
                                   gz::sim::EventManager &_eventMgr)
{
    // Clone _sdf, to have a non constant version (GetElement is non-constant)
    sdf::ElementPtr parentSDF = _sdf->Clone();

    // Extract the world name
    gz::sim::Entity worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
    std::optional<std::string> worldNameOptional = _ecm.ComponentData<gz::sim::components::Name>(worldEntity);
    this->worldName = worldNameOptional.value();

    if (!this->ParseGeneralSDF(parentSDF)) {
        return;
    }

    
    std::string modelString;
    if (this->generateForest) {
        // Remove previously generate models to avoid unnecessary files
        std::string meshPath = this->modelPath + "/meshes";
        std::filesystem::path fsModelPath(meshPath);
        if (!std::filesystem::exists(fsModelPath)) {
            std::filesystem::create_directory(fsModelPath);
        }
        for (const auto &entry : std::filesystem::directory_iterator(meshPath)) {
            std::filesystem::remove(entry.path());
        }

        if (!this->GenerateGround(parentSDF)) {
            return;
        }

        if (!this->GenerateTrees(parentSDF)) {
            return;
        }

        this->modelString = this->CreateModelStr();
    } else {
        // Load the model string from file
        std::string modelStrPath = this->modelPath + "/model.sdf";
        std::ifstream myFile(modelStrPath);
        std::stringstream buffer;
        buffer << myFile.rdbuf();
        this->modelString = buffer.str();
        myFile.close();
    }

    this->SpawnModel(this->modelString);
}

void Forest::Reset(const gz::sim::UpdateInfo &_info,
           gz::sim::EntityComponentManager &_ecm)
{
    this->SpawnModel(this->modelString);
}

bool Forest::ParseGeneralSDF(sdf::ElementPtr _sdf)
{
    // If a seed is given, use it, otherwise start with a random seed
    int seed;
    if (_sdf->HasElement("seed")) {
        seed = _sdf->Get<int>("seed");
    } else {
        std::random_device rd;
        seed = rd();
    }
    this->rng.seed(seed);

    // Use a sqaure forest with a width and height of 'size'
    if (_sdf->HasElement("size")) {
        this->forestSize = _sdf->Get<int>("size");
    } else {
        std::cerr << "[Forest] The size of the forest is missing, use <size> tag to specify!" << std::endl;
        return false;
    }

    // Check if the forest should be generated or loaded
    this->generateForest = true;
    if (_sdf->HasElement("generate")) {
        this->generateForest = _sdf->Get<bool>("generate");
    }

    // Extract the number of trees to generate
    if (_sdf->HasElement("trees")) {
        this->nTrees = _sdf->Get<int>("trees");
    } else {
        std::cerr << "[Forest] The number of trees is missing, use <trees> tag to specify!" << std::endl;
        return false;
    }

    // Store the texture used for the ground
    int groundTextureIndex = 0;
    if (_sdf->HasElement("ground_texture")) {
        groundTextureIndex = _sdf->Get<int>("ground_texture");
    } else {
        std::cerr << "[Forest] Ground texture index is missing, use <ground_texture> to specify!" << std::endl;
        return false;
    }
    this->groundTextureStr = FormatNumber(groundTextureIndex, 3);

    // Get the absolut path to the corresponding model using the config and gazebos resource paths
    std::string modelName;
    if (_sdf->HasElement("model_name")) {
        modelName = _sdf->Get<std::string>("model_name");
    } else {
        std::cerr << "[Forest] Name of the model is missing, use <model_name> tag to specify!" << std::endl;
        return false;
    }

    this->modelPath = "";
    std::vector<std::string> resourcePaths = gz::sim::resourcePaths();
    for (const std::string &resourcePath : resourcePaths) {
        for (const auto &entry : std::filesystem::directory_iterator(resourcePath)) {
            std::string curPath = entry.path();
            if (0 == curPath.compare(curPath.length()-modelName.length()-1, modelName.length()+1, "/" + modelName)) {
                this->modelPath = curPath;
            }
        }
    }
    if (this->modelPath == "") {
        std::cerr << "[Forest] Unable to find model path for model: " << modelName << std::endl;
        return false;
    }

    return true;
}

bool Forest::GenerateGround(sdf::ElementPtr _sdf) {

    Ground ground;
    ground.Generate(this->forestSize);

    gz::common::MeshPtr groundMesh = std::make_shared<gz::common::Mesh>();
    
    gz::common::SubMeshPtr groundSubMesh = createGroundMeshFromGround(ground);
    groundMesh->AddSubMesh(*groundSubMesh);

    std::string groundPath = this->modelPath + "/meshes/ground";
    gz::common::MeshManager::Instance()->Export(groundMesh.get(), groundPath, "dae");

    return true;
}

bool Forest::GenerateTrees(sdf::ElementPtr _sdf)
{

    // Distribution to give each tree a different seed and rotation
    std::uniform_int_distribution<int> treeSeedDistribution(0, INT_MAX / 4);
    std::uniform_real_distribution<double> treeRotDistribution(0, 2 * M_PI);

    // Compute the locations using a grid approach
    // (Taken from the original implementation)
    double treeMargin = 1.0;
    double treePosMin = - this->forestSize / 2.1 + treeMargin;
    double treePosMax =   this->forestSize / 2.1 - treeMargin;

    int gridCount = (int) std::ceil(std::sqrt(nTrees));
    double gridSize = 2 * treePosMax / gridCount;

    std::vector<gz::math::Vector3d> treePositions;
    for (int i = 0; i < gridCount; i++) {
        for (int j = 0; j < gridCount; j++) {
            float minX = treePosMin + j * gridSize;
            float maxX = treePosMin + (j+1) * gridSize;
            float minY = treePosMin + i * gridSize;
            float maxY = treePosMin + (i+1) * gridSize;

            // Apply random position within grid
            std::uniform_real_distribution<double> xPosDistribution(minX, maxX);
            std::uniform_real_distribution<double> yPosDistribution(minY, maxY);

            // Store position
            treePositions.push_back(gz::math::Vector3d(xPosDistribution(this->rng), yPosDistribution(this->rng), 0.0));   
        }
    }
    // Shuffle positions because not all are gonne be filled and species would not mix otherwise
    std::shuffle(std::begin(treePositions), std::end(treePositions), this->rng);

    // Generate the specified amount of trees for each species    
    int treesPlanted = 0;

    sdf::ElementPtr speciesSDF = _sdf->GetElement("species");
    while (speciesSDF != nullptr) {

        std::string speciesName;
        if (speciesSDF->HasAttribute("name")) {
            speciesName = speciesSDF->GetAttribute("name")->GetAsString();
        } else {
            std::cerr << "[Forest] Species name is missing in species tag!" << std::endl;
            return false;
        }
        
        float homogeneity = 1.0f;
        if (speciesSDF->HasElement("homogeneity")) {
            homogeneity = speciesSDF->Get<float>("homogeneity");
        } else {
            std::cerr << "[Forest] Homogeneity value is missing for species: " << speciesName << std::endl;
            return false;
        }

        float percentage = 0.0;
        if (speciesSDF->HasElement("percentage")) {
            percentage = speciesSDF->Get<float>("percentage");
        } else {
            std::cerr << "[Forest] Percentage value is missing for species: " << speciesName << std::endl;
            return false;
        }
        int nSpeciesTrees = (int) (std::ceil(nTrees * percentage));

        // Only contiunu if at least one tree is being planted for this species
        if (nSpeciesTrees == 0) {
            speciesSDF = speciesSDF->GetNextElement("species");
            continue;
        } else {
            this->speciesNames.push_back(speciesName);
        }

        gz::common::MeshPtr trunkMesh = std::make_shared<gz::common::Mesh>();
        gz::common::MeshPtr twigsMesh = std::make_shared<gz::common::Mesh>();

        for (int i = 0; i < nSpeciesTrees && treesPlanted < nTrees; i++) {

            // Generate A Tree
            Tree tree(treeSeedDistribution(this->rng));
            if (speciesSDF->HasElement("tree_properties")) {
                sdf::ElementPtr treePropertiesSDF = speciesSDF->GetElement("tree_properties");
                std::set<std::string> childNames = treePropertiesSDF->GetElementTypeNames();
                for (std::string childName : childNames) {
                    tree.SetProperty(childName, treePropertiesSDF->GetAny(childName), homogeneity);
                }
            }
            tree.Generate(treeRotDistribution(this->rng));

            gz::common::SubMeshPtr trunkSubMesh = createTrunkMeshFromTree(tree);
            gz::common::SubMeshPtr twigsSubMesh = createTwigsMeshFromTree(tree);
            
            trunkSubMesh->Scale(3.0);
            twigsSubMesh->Scale(3.0);
            trunkSubMesh->Translate(treePositions.at(treesPlanted));
            twigsSubMesh->Translate(treePositions.at(treesPlanted));

            trunkMesh->AddSubMesh(*trunkSubMesh);
            twigsMesh->AddSubMesh(*twigsSubMesh);

            treesPlanted++;
        }

        // Save the meshes so that they can be loaded using the sdf or used later
        std::string trunkMeshPath = this->modelPath + "/meshes/trunk_" + speciesName;
        std::string twigsMeshPath = this->modelPath + "/meshes/twigs_" + speciesName;
        gz::common::MeshManager::Instance()->Export(trunkMesh.get(), trunkMeshPath, "dae");
        gz::common::MeshManager::Instance()->Export(twigsMesh.get(), twigsMeshPath, "dae");

        // Store the texture choices
        int trunkTextureIndex = 0;
        if (speciesSDF->HasElement("trunk_texture")) {
            trunkTextureIndex = speciesSDF->Get<int>("trunk_texture");
        } else {
            std::cerr << "[Forest] Trunk texture index is missing for species " + speciesName + ", use <trunk_texture> to specify!" << std::endl;
            return false;
        }
        std::string trunkTextureStr = FormatNumber(trunkTextureIndex, 3);

        int twigsTextureIndex = 0;
        if (speciesSDF->HasElement("twigs_texture")) {
            twigsTextureIndex = speciesSDF->Get<int>("twigs_texture");
        } else {
            std::cerr << "[Forest] Trunk texture index is missing for species " + speciesName + ", use <twigs_texture> to specify!" << std::endl;
            return false;
        }
        std::string twigsTextureStr = FormatNumber(twigsTextureIndex, 3);

        this->trunkTextureStrs.push_back(trunkTextureStr);
        this->twigsTextureStrs.push_back(twigsTextureStr);

        // Look for the next species tag
        speciesSDF = speciesSDF->GetNextElement("species");
    }

    return true;
}

std::string Forest::CreateModelStr()
{

    std::string trunkVisualString;
    std::string twigVisualString;

    for (int i = 0; i < speciesNames.size(); i++) {
        std::string &speciesName = speciesNames.at(i);

        trunkVisualString += std::string("<visual name='Trunks_") + speciesName + "'>" + "\n" +
        "               <geometry>" + "\n" +
        "                   <mesh>" + "\n" +
        "                       <uri>model://procedural-forest/meshes/trunk_" + speciesName + ".dae</uri>" + "\n" +
        "                   </mesh>" + "\n" +
        "               </geometry>" + "\n" +
        "               <material>" + "\n" +
        "                   <double_sided>true</double_sided>" + "\n" +
        "                   <diffuse>1.0 1.0 1.0</diffuse>" + "\n" +
        "                   <pbr>" + "\n" +
        "                       <metal>" + "\n" +
        "                           <albedo_map>model://procedural-forest/materials/textures/trunk_" + trunkTextureStrs.at(i) + "_diffuse.tga</albedo_map>" + "\n" +
        "                           <normal_map>model://procedural-forest/materials/textures/trunk_" + trunkTextureStrs.at(i) + "_normal.tga</normal_map>" + "\n" +
        "                           <metalness_map>model://procedural-forest/materials/textures/trunk_" + trunkTextureStrs.at(i) + "_specular.tga</metalness_map>" + "\n" +
        "                       </metal>" + "\n" +
        "                   </pbr>" + "\n" +
        "               </material>" + "\n" +
        "           </visual>" + "\n";

        twigVisualString += std::string("<visual name='Twigs_") + speciesName + "'>" + "\n" +
        "               <geometry>" + "\n" +
        "                   <mesh>" + "\n" +
        "                       <uri>model://procedural-forest/meshes/twigs_" + speciesName + ".dae</uri>" + "\n" +
        "                   </mesh>" + "\n" +
        "               </geometry>" + "\n" +
        "               <material>" + "\n" +
        "                   <double_sided>true</double_sided>" + "\n" +
        "                   <diffuse>1.0 1.0 1.0</diffuse>" + "\n" +
        "                   <pbr>" + "\n" +
        "                       <metal>" + "\n" +
        "                           <albedo_map>model://procedural-forest/materials/textures/twig_" + twigsTextureStrs.at(i) + "_diffuse.tga</albedo_map>" + "\n" +
        "                           <normal_map>model://procedural-forest/materials/textures/twig_" + twigsTextureStrs.at(i) + "_normal.tga</normal_map>" + "\n" +
        "                           <metalness_map>model://procedural-forest/materials/textures/twig_" + twigsTextureStrs.at(i) + "_specular.tga</metalness_map>" + "\n" +
        "                       </metal>" + "\n" +
        "                   </pbr>" + "\n" +
        "               </material>" + "\n" +
        "           </visual>" + "\n";
    }

    std::string modelStr = std::string("<?xml version='1.0'?>") + "\n" +
        "<sdf version='1.6'>" + "\n" +
        "   <model name='Procedural Forest'>" + "\n" +
        "       <static>true</static>" + "\n" +
        "       <link name='forest_link'>" + "\n" +
        "           <visual name='Ground'>" + "\n" +
        "               <geometry>" + "\n" + 
        "                   <mesh>" + "\n" +
        "                       <uri>model://procedural-forest/meshes/ground.dae</uri>" + "\n" +
        "                   </mesh>" + "\n" +
        "               </geometry>" + "\n" +
        "               <material>" + "\n" +
        "                   <double_sided>true</double_sided>" + "\n" +
        "                   <diffuse>1.0 1.0 1.0</diffuse>" + "\n" +
        "                   <pbr>" + "\n" +
        "                       <metal>" + "\n" +
        "                           <albedo_map>model://procedural-forest/materials/textures/ground_" + this->groundTextureStr + "_diffuse.tga</albedo_map>" + "\n" +
        "                           <normal_map>model://procedural-forest/materials/textures/ground_" + this->groundTextureStr + "_normal.tga</normal_map>" + "\n" +
        "                           <metalness_map>model://procedural-forest/materials/textures/ground_" + this->groundTextureStr + "_specular.tga</metalness_map>" + "\n" +
        "                       </metal>" + "\n" +
        "                   </pbr>" + "\n" +
        "               </material>" + "\n" +
        "           </visual>" + "\n" +
        "           " + trunkVisualString +
        "           " + twigVisualString +
        "       </link>" + "\n" +
        "   </model>" + "\n" +
        "</sdf>";
    
    // Store the sdf so that it can be used as a standalone model
    std::string modelStrPath = this->modelPath + "/model.sdf";
    std::ofstream myFile(modelStrPath, std::fstream::out | std::fstream::trunc);
    myFile << modelStr;
    myFile.close();

    return modelStr;
}

bool Forest::SpawnModel(std::string modelStr)
{
    gz::transport::Node node;
    gz::msgs::EntityFactory req;
    gz::msgs::Boolean res;
    bool result;

    req.set_sdf(modelStr);

    std::string topic = "/world/" + this->worldName + "/create";

    int timeout = 1000; // ms
    bool executed = node.Request(topic, req, timeout, res, result);
    if (executed) {
        if (!result) {
            std::cerr << "[Forest] Request to spawn forest failed!" << std::endl;
            return false;
        }
    } else {
        std::cerr << "[Forest] Request to spawn forest timed out, T = " << timeout << std::endl;
        return false;
    }

    return true;
}