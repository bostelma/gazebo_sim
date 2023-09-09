#ifndef SWARM_H_
#define SWARM_H_

#include <iostream>
#include <queue>

#include <gz/plugin/Register.hh>
#include <gz/rendering/Camera.hh>
#include <gz/rendering/RenderTypes.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/ThermalCamera.hh>
#include <gz/rendering/Visual.hh>
#include <gz/sim/rendering/Events.hh>
#include <gz/transport.hh>
#include <gz/msgs.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/System.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/World.hh>
#include <gz/msgs/Utility.hh>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

class Swarm
      : public gz::sim::System,
        public gz::sim::ISystemConfigure,
        public gz::sim::ISystemPreUpdate,
        public gz::sim::ISystemReset
{

    private:

        struct Drone {
            int id;
            std::string name;
            std::queue<gz::math::Pose3d> waypoints;
            bool reachedWaypoint{false};
        };

    public:

        virtual void Configure(const gz::sim::Entity &_entity,
                               const std::shared_ptr<const sdf::Element> &_sdf,
                               gz::sim::EntityComponentManager &_ecm,
                               gz::sim::EventManager &_eventMgr) override;

        bool ParseGeneralSDF(sdf::ElementPtr _sdf);

        virtual void PreUpdate(const gz::sim::UpdateInfo &_info,
                               gz::sim::EntityComponentManager &_ecm) override;

        virtual void Reset(const gz::sim::UpdateInfo &_info,
                           gz::sim::EntityComponentManager &_ecm) override;

        bool ServiceSpawn(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep);

        bool ServiceWaypoint(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep);

        void PerformPostRenderingOperations();

        std::vector<std::pair<cv::Mat, cv::Mat>> CreateImages(std::vector<gz::math::Pose3d> poses);

        cv::Mat TakePictureThermal(const gz::rendering::ThermalCameraPtr _camera,
                                const gz::math::Pose3d &_pose);

        cv::Mat TakePictureRGB(const gz::rendering::CameraPtr _camera,
                            const gz::math::Pose3d &_pose);

    private:

        std::vector<std::shared_ptr<gz::transport::Node>> frameNodes;

        gz::transport::Node spawnNode;
        gz::transport::Node waypointNode;
        gz::transport::Node framePublishNode;
        gz::transport::Node::Publisher framePublisher;

        std::string worldName;

        std::map<int, Drone> drones;

        gz::common::ConnectionPtr connection{nullptr};

        float direct_thermal_factor;
        float indirect_thermal_factor;
        float lower_thermal_threshold;
        float upper_thermal_threshold;

        unsigned char *rgbData;

};

GZ_ADD_PLUGIN(
    Swarm,
    gz::sim::System,
    Swarm::ISystemConfigure,
    Swarm::ISystemPreUpdate,
    Swarm::ISystemReset
)

#endif  // SWARM_H_