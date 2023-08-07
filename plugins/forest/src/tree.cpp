#include "tree.h"

Tree::Tree(int seed)
{
    rng.seed(seed);
    this->procTree.mProperties.mSeed = seed;
}

Tree::~Tree()
{

}

void Tree::Generate(double angle)
{
    // Compute the new tree
    this->procTree.generate();

    double cos = std::cos(angle);
    double sin = std::sin(angle);

    // Copy and store the trunk data
    // Not that the y and z axis are switched
    this->trunkVerticies.clear();
    this->trunkNormals.clear();
    this->trunkFaces.clear();
    this->trunkUVs.clear();
    for (int i = 0; i < this->procTree.mVertCount; i++) {
        this->trunkVerticies.push_back(gz::math::Vector3d(
            this->procTree.mVert[i].x * cos - this->procTree.mVert[i].z * sin,
            this->procTree.mVert[i].z * cos + this->procTree.mVert[i].x * sin,
            this->procTree.mVert[i].y
        ));
        this->trunkNormals.push_back(gz::math::Vector3d(
            this->procTree.mVert[i].x * cos - this->procTree.mVert[i].z * sin,
            this->procTree.mVert[i].z * cos + this->procTree.mVert[i].x * sin,
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
            this->procTree.mTwigVert[i].x * cos - this->procTree.mTwigVert[i].z * sin,
            this->procTree.mTwigVert[i].z * cos + this->procTree.mTwigVert[i].x * sin,
            this->procTree.mTwigVert[i].y
        ));
        this->twigNormals.push_back(gz::math::Vector3d(
            this->procTree.mTwigNormal[i].x * cos - this->procTree.mTwigNormal[i].z * sin,
            this->procTree.mTwigNormal[i].z * cos + this->procTree.mTwigNormal[i].x * sin,
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
    } else if (property == "clump_min") {
        this->procTree.mProperties.mClumpMin = floatValue;
    } else if (property == "length_falloff_factor") {
        this->procTree.mProperties.mLengthFalloffFactor = floatValue;
    } else if (property == "length_falloff_power") {
        this->procTree.mProperties.mLengthFalloffPower = floatValue;
    } else if (property == "branch_factor") {
        this->procTree.mProperties.mBranchFactor = floatValue;
    } else if (property == "radius_falloff_rate") {
        this->procTree.mProperties.mRadiusFalloffRate = floatValue;
    } else if (property == "climb_rate") {
        this->procTree.mProperties.mClimbRate = floatValue;
    } else if (property == "trunk_kink") {
        this->procTree.mProperties.mTrunkKink = floatValue;
    } else if (property == "max_radius") {
        this->procTree.mProperties.mMaxRadius = floatValue;
    } else if (property == "tree_steps") {
        this->procTree.mProperties.mTreeSteps = intValue;
    } else if (property == "taper_rate") {
        this->procTree.mProperties.mTaperRate = floatValue;
    } else if (property == "twist_rate") {
        this->procTree.mProperties.mTwistRate = floatValue;
    } else if (property == "segments") {
        this->procTree.mProperties.mSegments = intValue;
    } else if (property == "levels") {
        this->procTree.mProperties.mLevels = intValue;
    } else if (property == "sweep_amount") {
        this->procTree.mProperties.mSweepAmount = floatValue;
    } else if (property == "initial_branch_length") {
        this->procTree.mProperties.mInitialBranchLength = floatValue;
    } else if (property == "trunk_length") {
        this->procTree.mProperties.mTrunkLength = floatValue;
    } else if (property == "drop_amount") {
        this->procTree.mProperties.mDropAmount = floatValue;
    } else if (property == "grow_amount") {
        this->procTree.mProperties.mGrowAmount = floatValue;
    } else if (property == "v_multiplier") {
        this->procTree.mProperties.mVMultiplier = floatValue;
    } else if (property == "twig_scale") {
        this->procTree.mProperties.mTwigScale = floatValue;
    } else {
        std::cerr << "[Tree.cpp] Unsupported property: " << property << std::endl;
    }
}