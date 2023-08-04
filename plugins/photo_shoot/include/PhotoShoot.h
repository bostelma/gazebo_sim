#ifndef PHOTO_SHOOT_PHOTO_SHOOT_H_
#define PHOTO_SHOOT_PHOTO_SHOOT_H_

#include <iostream>
#include <vector>
#include <filesystem>

#include <gz/common/Image.hh>
#include <gz/plugin/Register.hh>
#include <gz/rendering/Camera.hh>
#include <gz/rendering/RenderTypes.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/ThermalCamera.hh>
#include <gz/rendering/Visual.hh>
#include <gz/sim/rendering/Events.hh>
#include <gz/sim/Util.hh>
#include <gz/sim.hh>

class PhotoShoot
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

        void PerformPostRenderingOperations();

        void SavePicture(const gz::rendering::ThermalCameraPtr _camera,
                         const gz::math::Pose3d &_pose,
                         const std::string &_fileName);

    private:

        bool takePicture{true};

        gz::common::ConnectionPtr connection{nullptr};

        std::string directory;

        std::vector<gz::math::Pose3d> poses;

};

GZ_ADD_PLUGIN(
    PhotoShoot,
    PhotoShoot::System,
    PhotoShoot::ISystemConfigure
)

#endif  // PHOTO_SHOOT_PHOTO_SHOOT_H_