#include "ground.h"

void Ground::Generate(int size, float textureSize)
{
    float offset = size / 2.0;

    int index = 0;
    float x = - offset;
    while (x < offset) {
        float y = - offset;
        while (y < offset) {

            float delta_x = textureSize;
            float delta_y = textureSize;
            if (x + delta_x > offset)
                delta_x = offset - x;
            if (y + delta_y > offset)
                delta_y = offset - y;

            // Create the vertices
            this->verticies.push_back(gz::math::Vector3d(x, y, 0.0));
            this->verticies.push_back(gz::math::Vector3d(x + delta_x, y, 0.0));
            this->verticies.push_back(gz::math::Vector3d(x + delta_x, y + delta_y, 0.0));
            this->verticies.push_back(gz::math::Vector3d(x, y + delta_y, 0.0));

            // Create the UVs
            float uv = delta_x / textureSize;
            this->UVs.push_back(gz::math::Vector2d(0.0, 0.0));
            this->UVs.push_back(gz::math::Vector2d( uv, 0.0));
            this->UVs.push_back(gz::math::Vector2d( uv,  uv));
            this->UVs.push_back(gz::math::Vector2d(0.0,  uv));

            // Create the normals
            this->normals.push_back(gz::math::Vector3d(0.0, 0.0, 1.0));
            this->normals.push_back(gz::math::Vector3d(0.0, 0.0, 1.0));
            this->normals.push_back(gz::math::Vector3d(0.0, 0.0, 1.0));
            this->normals.push_back(gz::math::Vector3d(0.0, 0.0, 1.0));

            // Create the faces
            this->faces.push_back(gz::math::Vector3i(index, index+1, index+2));
            this->faces.push_back(gz::math::Vector3i(index, index+2, index+3));

            y += textureSize;
            index += 4;
        }
        x += textureSize;
    } 
}