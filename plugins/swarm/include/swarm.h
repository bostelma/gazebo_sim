#ifndef SWARM_H_
#define SWARM_H_

#include <iostream>

#include <gz/plugin/Register.hh>
#include <gz/transport.hh>
#include <gz/msgs.hh>
#include <gz/sim/System.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/World.hh>
#include <gz/msgs/Utility.hh>

class Swarm
      : public gz::sim::System,
        public gz::sim::ISystemConfigure,
        public gz::sim::ISystemReset
{

    public:

        virtual void Configure(const gz::sim::Entity &_entity,
                               const std::shared_ptr<const sdf::Element> &_sdf,
                               gz::sim::EntityComponentManager &_ecm,
                               gz::sim::EventManager &_eventMgr) override;

        virtual void Reset(const gz::sim::UpdateInfo &_info,
                           gz::sim::EntityComponentManager &_ecm) override;

        bool ServiceSpawn(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep);

        bool ServiceWaypoint(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep);

    private:

        std::vector<std::shared_ptr<gz::transport::Node>> frameNodes;

        gz::transport::Node spawnNode;
        gz::transport::Node waypointNode;
        gz::transport::Node framePublishNode;
        gz::transport::Node::Publisher framePublisher;

        std::string worldName;

};

GZ_ADD_PLUGIN(
    Swarm,
    gz::sim::System,
    Swarm::ISystemConfigure,
    Swarm::ISystemReset
)

#endif  // SWARM_H_