#ifndef FOREST_MESH_UTILS_H_
#define FOREST_MESH_UTILS_H_

#include <memory>

#include <gz/common/SubMesh.hh>
#include <gz/common/graphics/Types.hh>

#include "tree.h"
#include "ground.h"

gz::common::SubMeshPtr createTrunkMeshFromTree(Tree &tree);
gz::common::SubMeshPtr createTwigsMeshFromTree(Tree &tree);
gz::common::SubMeshPtr createGroundMeshFromGround(Ground &ground);

#endif  // FOREST_MESH_UTILS_H_