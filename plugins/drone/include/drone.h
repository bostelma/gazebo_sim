#ifndef DRONE_H_
#define DRONE_H_

#include <iostream>
#include <queue>

#include <gz/plugin/Register.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/Pose.hh>
#include <gz/sim/components/World.hh>
#include <gz/sim/components/Gravity.hh>
#include <gz/sim/System.hh>
#include <gz/sim/Model.hh>
#include <gz/transport.hh>
#include <gz/msgs.hh>
#include <gz/msgs/Utility.hh>

class Drone
      : public gz::sim::System,
        public gz::sim::ISystemConfigure,
        public gz::sim::ISystemPreUpdate,
        public gz::sim::ISystemPostUpdate
{

    public:
    
        virtual void Configure(const gz::sim::Entity &_entity,
                               const std::shared_ptr<const sdf::Element> &_sdf,
                               gz::sim::EntityComponentManager &_ecm,
                               gz::sim::EventManager &_eventMgr) override;

        virtual void PreUpdate(const gz::sim::UpdateInfo &_info,
                               gz::sim::EntityComponentManager &_ecm) override;

        virtual void PostUpdate(const gz::sim::UpdateInfo &_info,
                                const gz::sim::EntityComponentManager &_ecm) override;

        bool WaypointService(const gz::msgs::Pose &_req, gz::msgs::Boolean &_rep);

        void OnNewRGBImage(const gz::msgs::Image &imageMsg);
        void OnNewThermalImage(const gz::msgs::Image &imageMsg);

    private:

        std::shared_ptr<gz::msgs::Image> rgbImage;
        std::shared_ptr<gz::msgs::Image> thermalImage;

        int imageCounter = 0;

        gz::sim::Entity droneEntity;
        gz::sim::Model droneModel;

        gz::transport::Node waypointNode;
        gz::transport::Node imageNode;
        gz::transport::Node thermalNode;
        gz::transport::Node frameNode;
        gz::transport::Node::Publisher framePub;

        std::queue<std::tuple<std::chrono::steady_clock::duration, gz::math::Pose3d>> poses;
        std::queue<gz::math::Pose3d> waypoints;

        int timer = 1000;


};


#endif  // DRONE_H_