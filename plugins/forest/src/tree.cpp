#include "tree.h"

Tree::Tree(int seed)
{
    rng.seed(seed);
    this->procTree.mProperties.mSeed = seed;
}

Tree::~Tree()
{

}

void Tree::Generate()
{
    // Compute the new tree
    this->procTree.generate();

    // Copy and store the trunk data
    // Not that the y and z axis are switched
    this->trunkVerticies.clear();
    this->trunkNormals.clear();
    this->trunkFaces.clear();
    this->trunkUVs.clear();
    for (int i = 0; i < this->procTree.mVertCount; i++) {
        this->trunkVerticies.push_back(gz::math::Vector3d(
            this->procTree.mVert[i].x,
            this->procTree.mVert[i].z,
            this->procTree.mVert[i].y
        ));
        this->trunkNormals.push_back(gz::math::Vector3d(
            this->procTree.mNormal[i].x,
            this->procTree.mNormal[i].z,
            this->procTree.mNormal[i].y
        ));
        this->trunkUVs.push_back(gz::math::Vector2d(
            this->procTree.mUV[i].u,
            this->procTree.mUV[i].v
        ));
    }
    for (int i = 0; i < this->procTree.mFaceCount; i++) {
        this->trunkFaces.push_back(gz::math::Vector3i(
            this->procTree.mFace[i].x,
            this->procTree.mFace[i].z,
            this->procTree.mFace[i].y
        ));
    }

    // Copy and store the twig data
    this->twigVerticies.clear();
    this->twigNormals.clear();
    this->twigFaces.clear();
    this->twigUVs.clear();
    for (int i = 0; i < this->procTree.mTwigVertCount; i++) {
        this->twigVerticies.push_back(gz::math::Vector3d(
            this->procTree.mTwigVert[i].x,
            this->procTree.mTwigVert[i].z,
            this->procTree.mTwigVert[i].y
        ));
        this->twigNormals.push_back(gz::math::Vector3d(
            this->procTree.mTwigNormal[i].x,
            this->procTree.mTwigNormal[i].z,
            this->procTree.mTwigNormal[i].y
        ));
        this->twigUVs.push_back(gz::math::Vector2d(
            this->procTree.mTwigUV[i].u,
            this->procTree.mTwigUV[i].v
        ));
    }
    for (int i = 0; i < this->procTree.mTwigFaceCount; i++) {
        this->twigFaces.push_back(gz::math::Vector3i(
            this->procTree.mTwigFace[i].x,
            this->procTree.mTwigFace[i].z,
            this->procTree.mTwigFace[i].y
        ));
    }
}

void Tree::SetProperty(const std::string &property, std::any value, float homogeneity)
{
    float floatValue = 0.0;
    int intValue = 0;

    try {
        std::string valueAsString = std::any_cast<std::string>(value);
        floatValue = std::stof(valueAsString);
        intValue = std::stoi(valueAsString);
    } catch (const std::bad_any_cast &/*e*/) {
        try {
            floatValue = std::any_cast<float>(value);
        } catch(const std::bad_any_cast &/*e*/) {
            try {
                intValue = std::any_cast<int>(value);
            } catch (std::bad_any_cast &/*e*/) {
                std::cerr << "[Tree.cpp] Unsupported property type for property: " << property << std::endl;
                return;
            }
        }
    }

    // Apply random noise if dersired
    if (homogeneity < 1.0f) {
        std::uniform_real_distribution<float> boolDist(0.0, 1.0);
        if (boolDist(this->rng) > 0.5) {
            std::uniform_real_distribution<float> dist(homogeneity, 2 - homogeneity);
            floatValue = floatValue * dist(this->rng);
            intValue = (int) (std::round(intValue * dist(this->rng)));
        }
    }

    if (property == "clump_max") {
        this->procTree.mProperties.mClumpMax = floatValue;
    } else if ("clump_min") {
        this->procTree.mProperties.mClumpMin = floatValue;
    } else if ("length_falloff_factor") {
        this->procTree.mProperties.mLengthFalloffFactor = floatValue;
    } else if ("length_falloff_power") {
        this->procTree.mProperties.mLengthFalloffPower = floatValue;
    } else if ("branch_factor") {
        this->procTree.mProperties.mBranchFactor = floatValue;
    } else if ("radius_falloff_rate") {
        this->procTree.mProperties.mRadiusFalloffRate = floatValue;
    } else if ("climb_rate") {
        this->procTree.mProperties.mClimbRate = floatValue;
    } else if ("trunk_kink") {
        this->procTree.mProperties.mTrunkKink = floatValue;
    } else if ("max_radius") {
        this->procTree.mProperties.mMaxRadius = floatValue;
    } else if ("tree_steps") {
        this->procTree.mProperties.mTreeSteps = intValue;
    } else if ("taper_rate") {
        this->procTree.mProperties.mTaperRate = floatValue;
    } else if ("twist_rate") {
        this->procTree.mProperties.mTwistRate = floatValue;
    } else if ("segments") {
        this->procTree.mProperties.mSegments = intValue;
    } else if ("levels") {
        this->procTree.mProperties.mLevels = intValue;
    } else if ("sweep_mount") {
        this->procTree.mProperties.mSweepAmount = floatValue;
    } else if ("initial_branch_length") {
        this->procTree.mProperties.mInitialBranchLength = floatValue;
    } else if ("trunk_length") {
        this->procTree.mProperties.mTrunkLength = floatValue;
    } else if ("drop_amount") {
        this->procTree.mProperties.mDropAmount = floatValue;
    } else if ("grow_amount") {
        this->procTree.mProperties.mGrowAmount = floatValue;
    } else if ("v_multiplier") {
        this->procTree.mProperties.mVMultiplier = floatValue;
    } else if ("twig_scale") {
        this->procTree.mProperties.mTwigScale = floatValue;
    } else {
        std::cerr << "[Tree.cpp] Unsupported property: " << property << std::endl;
    }
}