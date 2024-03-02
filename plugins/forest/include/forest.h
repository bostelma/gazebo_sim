#ifndef FOREST_FOREST_H_
#define FOREST_FOREST_H_

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>

#include <gz/common/Mesh.hh>
#include <gz/common/MeshManager.hh>
#include <gz/common/SubMesh.hh>
#include <gz/math.hh>
#include <gz/msgs.hh>
#include <gz/plugin/Register.hh>
#include <gz/rendering.hh>
#include <gz/sim/components.hh>
#include <gz/sim.hh>
#include <gz/sim/Util.hh>
#include <gz/transport.hh>

#include "ground.h"
#include "mesh_utils.h"
#include "tree.h"

class Forest
      : public gz::sim::System,
        public gz::sim::ISystemConfigure,
        public gz::sim::ISystemReset
{

    public:
    
        virtual void Configure(const gz::sim::Entity &_entity,
                               const std::shared_ptr<const sdf::Element> &_sdf,
                               gz::sim::EntityComponentManager &_ecm,
                               gz::sim::EventManager &_eventMgr) override;
        virtual void Reset(const gz::sim::UpdateInfo &_info,
                           gz::sim::EntityComponentManager &_ecm) override;

        void PerformPreRenderingOperations();

    private:
    
        void AddToScene();

        bool ParseGeneralSDF(sdf::ElementPtr _sdf);

        bool GenerateGround(sdf::ElementPtr _sdf);

        bool GenerateTrees(sdf::ElementPtr _sdf);

        std::string CreateModelStr();

        bool SpawnModel(std::string modelStr);

    private:

        struct debugData {
            std::string species;    // species name
            double x;               // x coordinate in meters, 0,0 in the middle
            double y;               // y coordinate in meters, 0,0 in the middle
            double orientation;     // orientation in radians
            int seed;               // seed for procedural generation
        };

        bool debugForest{false};
        bool generateForest{true};
        bool directSpawning{false};

        float trunkTemperature{288.15};
        float twigsTemperature{288.15};
        float textureSize{1.0};

        int seed;
        int forestSize;
        int nTrees;

        std::mt19937 rng;

        std::string debugPath{""};
        std::string modelName;
        std::string modelPath;
        std::string modelString;
        std::string worldName;
        std::string groundTextureStr;
        std::string groundThermalStr;
        std::string groundMeshStr;
        std::string treesMeshPrefixStr;
        std::string treesMeshPostfixStr;

        std::vector<std::string> speciesNames;
        std::vector<std::string> trunkTextureStrs;
        std::vector<std::string> twigsTextureStrs;
        std::vector<debugData> debugDataVec;


};

GZ_ADD_PLUGIN(
    Forest,
    Forest::System,
    Forest::ISystemConfigure,
    Forest::ISystemReset
)

#endif //   FOREST_FOREST_H_