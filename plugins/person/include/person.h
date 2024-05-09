#ifndef PERSON_PERSON_H_
#define PERSON_PERSON_H_

#include <iostream>
#include <filesystem>
#include <queue>

#include <gz/math.hh>
#include <gz/msgs.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim.hh>
#include <gz/sim/components.hh>
#include <gz/sim/Util.hh>
#include <gz/transport.hh>
#include <gz/msgs.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/System.hh>
#include <gz/sim/components/Name.hh>
#include <gz/sim/components/World.hh>
#include <gz/msgs/Utility.hh>


class Person
      : public gz::sim::System,
        public gz::sim::ISystemConfigure,
        public gz::sim::ISystemPreUpdate
{

    //Added struct so that each person has an own id, name and waypoint queue
    private:
        
        struct PersonStruct{
            int id;
            std::string name;
            std::queue<gz::math::Pose3d> waypoints;
        };


    public:

        virtual void Configure(const gz::sim::Entity &_entity,
                               const std::shared_ptr<const sdf::Element> &_sdf,
                               gz::sim::EntityComponentManager &_ecm,
                               gz::sim::EventManager &_eventMgr) override;

        virtual void PreUpdate(const gz::sim::UpdateInfo &_info,
                               gz::sim::EntityComponentManager &_ecm) override;

        bool ServiceSpawn(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep);
        bool ServiceWaypoint(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep);

    private:

        bool ParseGeneralSDF(sdf::ElementPtr _sdf);

        std::string CreateModelStr(std::string personName = "Person");

        bool SpawnModel(std::string modelStr, gz::math::Pose3d pose);

    private:

        float scale;
        float temperature{-1};

        gz::math::Pose3d modelPose;

        std::string meshPath;
        std::string modelPath;
        std::string worldName;

        std::map<int, PersonStruct> persons;

        gz::transport::Node spawnNode;
        gz::transport::Node waypointNode;
        std::vector<gz::math::Pose3d> poses;

};

GZ_ADD_PLUGIN(
    Person,
    Person::System,
    Person::ISystemConfigure,
    Person::ISystemPreUpdate
)

#endif  // PERSON_PERSON_H_