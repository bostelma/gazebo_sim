#ifndef PERSON_PERSON_H_
#define PERSON_PERSON_H_

#include <iostream>
#include <filesystem>

#include <gz/math.hh>
#include <gz/msgs.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim.hh>
#include <gz/sim/components.hh>
#include <gz/sim/Util.hh>
#include <gz/transport.hh>


class Person
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

        std::string CreateModelStr();

        bool SpawnModel(std::string modelStr);

    private:

        float scale;
        float temperature{-1};

        gz::math::Pose3d modelPose;

        std::string meshPath;
        std::string modelPath;
        std::string worldName;

};

GZ_ADD_PLUGIN(
    Person,
    Person::System,
    Person::ISystemConfigure
)

#endif  // PERSON_PERSON_H_