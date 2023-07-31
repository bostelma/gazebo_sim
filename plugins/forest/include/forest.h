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
#include <gz/sim/components.hh>
#include <gz/sim.hh>
#include <gz/sim/Util.hh>
#include <gz/transport.hh>

#include "ground.h"
#include "mesh_utils.h"
#include "tree.h"

class Forest
      : public gz::sim::System,
        public gz::sim::ISystemConfigure
{

    public:
    
        virtual void Configure(const gz::sim::Entity &_entity,
                               const std::shared_ptr<const sdf::Element> &_sdf,
                               gz::sim::EntityComponentManager &_ecm,
                               gz::sim::EventManager &_eventMgr) override;

    private:
    
        bool ParseGeneralSDF(sdf::ElementPtr _sdf);

        bool GenerateGround(sdf::ElementPtr _sdf);

        bool GenerateTrees(sdf::ElementPtr _sdf);

        std::string CreateModelStr();

        bool SpawnModel(std::string modelStr);

    private:

        bool generateForest;

        int forestSize;
        int nTrees;

        std::mt19937 rng;

        std::string modelPath;
        std::string worldName;
        std::string groundTextureStr;

        std::vector<std::string> speciesNames;
        std::vector<std::string> trunkTextureStrs;
        std::vector<std::string> twigsTextureStrs;


};

GZ_ADD_PLUGIN(
    Forest,
    Forest::System,
    Forest::ISystemConfigure
)

#endif //   FOREST_FOREST_H_