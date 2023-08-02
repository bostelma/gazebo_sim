#ifndef FOREST_TREE_H_
#define FOREST_TREE_H_

#include <random>
#include <vector>
#include <cmath>
#include <any>

#include <gz/math.hh>

#include "proctree.h"

class Tree
{

    public:

        Tree(int seed);
        ~Tree();

        void Generate(double angle = 0.0);
        void SetProperty(const std::string &property, std::any value, float homogeneity = 1.0);

    public:

        std::vector<gz::math::Vector3d> trunkVerticies;
        std::vector<gz::math::Vector3d> trunkNormals;
        std::vector<gz::math::Vector3i> trunkFaces;
        std::vector<gz::math::Vector2d> trunkUVs;

        std::vector<gz::math::Vector3d> twigVerticies;
        std::vector<gz::math::Vector3d> twigNormals;
        std::vector<gz::math::Vector3i> twigFaces;
        std::vector<gz::math::Vector2d> twigUVs;

    private:

        Proctree::Tree procTree;
        std::mt19937 rng;

        


        

};

#endif  // FOREST_TREE_H_