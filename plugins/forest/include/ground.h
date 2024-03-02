#ifndef FOREST_GROUND_H_
#define FOREST_GROUND_H_

#include <vector>

#include <gz/math.hh>

class Ground
{

    public:

        void Generate(int size, float textureSize);

    public:

        std::vector<gz::math::Vector3d> verticies;
        std::vector<gz::math::Vector3d> normals;
        std::vector<gz::math::Vector3i> faces;
        std::vector<gz::math::Vector2d> UVs;

};

#endif  // FOREST_GROUND_H_