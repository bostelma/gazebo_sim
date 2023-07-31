#include "ground.h"

void Ground::Generate(int size)
{
    double offset = - size / 2.0;

    int index = 0;
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            this->verticies.push_back(gz::math::Vector3d(offset+x, offset+y, 0.0));
            this->verticies.push_back(gz::math::Vector3d(offset+x+ 1, offset+y, 0.0));
            this->verticies.push_back(gz::math::Vector3d(offset+x+ 1, offset+y+1, 0.0));
            this->verticies.push_back(gz::math::Vector3d(offset+x, offset+y+1, 0.0));

            this->UVs.push_back(gz::math::Vector2d(0.0, 0.0));
            this->UVs.push_back(gz::math::Vector2d(1.0, 0.0));
            this->UVs.push_back(gz::math::Vector2d(1.0, 1.0));
            this->UVs.push_back(gz::math::Vector2d(0.0, 1.0));

            this->normals.push_back(gz::math::Vector3d(0.0, 0.0, 1.0));
            this->normals.push_back(gz::math::Vector3d(0.0, 0.0, 1.0));
            this->normals.push_back(gz::math::Vector3d(0.0, 0.0, 1.0));
            this->normals.push_back(gz::math::Vector3d(0.0, 0.0, 1.0));

            this->faces.push_back(gz::math::Vector3i(index, index+1, index+2));
            this->faces.push_back(gz::math::Vector3i(index, index+2, index+3));

            index += 4;
        }
    } 
}