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

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

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

        cv::Mat TakePictureThermal(const gz::rendering::ThermalCameraPtr _camera,
                                const gz::math::Pose3d &_pose);

        cv::Mat TakePictureRGB(const gz::rendering::CameraPtr _camera,
                            const gz::math::Pose3d &_pose);

    private:

        float direct_thermal_factor;
        float indirect_thermal_factor;
        float lower_thermal_threshold;
        float upper_thermal_threshold;

        unsigned char *rgbData;

        bool takePicture{true};

        gz::common::ConnectionPtr connection{nullptr};

        std::string directory;
        std::string prefix{""};

        std::vector<gz::math::Pose3d> poses;

};

GZ_ADD_PLUGIN(
    PhotoShoot,
    PhotoShoot::System,
    PhotoShoot::ISystemConfigure
)

#endif  // PHOTO_SHOOT_PHOTO_SHOOT_H_