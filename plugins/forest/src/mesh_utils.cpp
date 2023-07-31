#include "mesh_utils.h"

gz::common::SubMeshPtr createTrunkMeshFromTree(Tree &tree)
{
    gz::common::SubMeshPtr subMeshTrunk = std::make_shared<gz::common::SubMesh>();
    
    for (gz::math::Vector3d &vertex : tree.trunkVerticies) {
        subMeshTrunk->AddVertex(vertex);
    }

    for (gz::math::Vector3d &normal : tree.trunkNormals) {
        subMeshTrunk->AddNormal(normal);
    }

    for (gz::math::Vector3i &face : tree.trunkFaces) {
        subMeshTrunk->AddIndex(face.X());
        subMeshTrunk->AddIndex(face.Y());
        subMeshTrunk->AddIndex(face.Z());
    }

    for (gz::math::Vector2d &uv : tree.trunkUVs) {
        subMeshTrunk->AddTexCoord(uv);
    }

    // Return Submesh
    return subMeshTrunk;
}

gz::common::SubMeshPtr createTwigsMeshFromTree(Tree &tree)
{
    gz::common::SubMeshPtr subMeshTwigs = std::make_shared<gz::common::SubMesh>();

    for (gz::math::Vector3d &vertex : tree.twigVerticies) {
        subMeshTwigs->AddVertex(vertex);
    }

    for (gz::math::Vector3d &normal : tree.twigNormals) {
        subMeshTwigs->AddNormal(normal);
    }

    for (gz::math::Vector3i &face : tree.twigFaces) {
        subMeshTwigs->AddIndex(face.X());
        subMeshTwigs->AddIndex(face.Y());
        subMeshTwigs->AddIndex(face.Z());
    }

    for (gz::math::Vector2d &uv : tree.twigUVs) {
        subMeshTwigs->AddTexCoord(uv);
    }

    // Return Submesh
    return subMeshTwigs;
}

gz::common::SubMeshPtr createGroundMeshFromGround(Ground &ground)
{
    gz::common::SubMeshPtr subMesh = std::make_shared<gz::common::SubMesh>();

    for (gz::math::Vector3d &vertex : ground.verticies) {
        subMesh->AddVertex(vertex);
    }

    for (gz::math::Vector3d &normal : ground.normals) {
        subMesh->AddNormal(normal);
    }

    for (gz::math::Vector3i &face : ground.faces) {
        subMesh->AddIndex(face.X());
        subMesh->AddIndex(face.Y());
        subMesh->AddIndex(face.Z());
    }

    for (gz::math::Vector2d &uv : ground.UVs) {
        subMesh->AddTexCoord(uv);
    }

    // Return Submesh
    return subMesh;
}

