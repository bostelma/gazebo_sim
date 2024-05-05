#include "person.h"

void Person::Configure(const gz::sim::Entity &_entity,
                       const std::shared_ptr<const sdf::Element> &_sdf,
                       gz::sim::EntityComponentManager &_ecm,
                       gz::sim::EventManager &_eventMgr)
{
    // Clone _sdf, to have a non constant version (GetElement is non-constant)
    sdf::ElementPtr parentSDF = _sdf->Clone();

    // Extract the world name
    gz::sim::Entity worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
    std::optional<std::string> worldNameOptional = _ecm.ComponentData<gz::sim::components::Name>(worldEntity);
    
    if (!worldNameOptional) {
        std::cerr << "[Person] Failed to retrieve world name!" << std::endl;
        return;
    }
    
    
    this->worldName = worldNameOptional.value();

    if (!this->ParseGeneralSDF(parentSDF)) {
        return;
    }

    
    // Advertise the spawn service
    std::string service = "/world/" + this->worldName + "/person/spawn";
    if (!this->spawnNode.Advertise(service, &Person::ServiceSpawn, this)) {
        std::cerr << "[Person] Error advertising service [" << service << "]" << std::endl;
        return;
    }

    // Advertise the waypoint service
    service = "/world/" + this->worldName + "/person/waypoint";
    if (!this->waypointNode.Advertise(service, &Person::ServiceWaypoint, this)) {
        std::cerr << "[Person] Error advertising service [" << service << "]" << std::endl;
        return;
    }

    std::string modelString = this->CreateModelStr();

    this->SpawnModel(modelString);
}

bool Person::ParseGeneralSDF(sdf::ElementPtr _sdf)
{
    if (!_sdf->HasElement("model_name")) {
        std::cerr << "[Person] The name of the model is missing, use <model_name> to specify!" << std::endl;
        return false;
    }
    std::string modelName = _sdf->Get<std::string>("model_name");

    if (!_sdf->HasElement("model_pose")) {
        std::cerr << "[Person] The pose of the model is missing, use <model_pose> to specify!" << std::endl;
        return false;
    }
    std::string modelPose = _sdf->Get<std::string>("model_pose");

    if (!_sdf->HasElement("pose")) {
        std::cerr << "[Person] The pose is missing, use <pose> to specify!" << std::endl;
        return false;
    }
    this->modelPose = _sdf->Get<gz::math::Pose3d>("pose");

    if (!_sdf->HasElement("scale")) {
        std::cerr << "[Person] The scale is missing, use <scale> to specify!" << std::endl;
        return false;
    }
    this->scale = _sdf->Get<float>("scale");

    if (_sdf->HasElement("temperature")) {
        this->temperature = _sdf->Get<float>("temperature");
        if (this->temperature < 0.0) {
            std::cerr << "[Person] Temperature values must be positive, as they are in Kelvin!" << std::endl;
            return false;
        }
    }

    std::vector<std::string> resourcePaths = gz::sim::resourcePaths();
    for (const std::string &resourcePath : resourcePaths) {
        for (const auto &entry : std::filesystem::directory_iterator(resourcePath)) {
            std::string curPath = entry.path();
            if (curPath.compare(curPath.length()-modelName.length()-1, modelName.length()+1, "/" + modelName) == 0) {
                this->modelPath = curPath;
            }
        }
    }

    if (this->modelPath == "") {
        std::cerr << "[Person] Unable to find model path for model: " << modelName << std::endl;
        return false;
    }

    std::string fullMeshPath = this->modelPath + "/meshes/" + modelPose + ".dae";
    if (!std::filesystem::exists(std::filesystem::path(fullMeshPath))) {
        std::cerr << "[Person] Unable to find mesh: " << fullMeshPath << std::endl;
        return false;
    }
    this->meshPath = "model://" + modelName + "/meshes/" + modelPose + ".dae";

    return true;
}

void Person::PreUpdate(const gz::sim::UpdateInfo &_info,
                      gz::sim::EntityComponentManager &_ecm)
{

   for (auto it = this->persons.begin(); it != this->persons.end(); it++)
   {
        PersonStruct &person = it->second;
        if (!person.waypoints.empty()) {
            gz::sim::Entity entity = _ecm.EntityByComponents(gz::sim::components::Name(person.name));
            gz::sim::Model model = gz::sim::Model(entity);
            model.SetWorldPoseCmd(_ecm, person.waypoints.front());
            person.waypoints.pop();
        }   
   }

    
}

bool Person::ServiceSpawn(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep)
{
    // Setup spawn service
    gz::transport::Node node;
    gz::msgs::EntityFactory req;
    gz::msgs::Boolean res;
    bool result;

    std::string topic = "/world/" + worldName + "/create";
    int timeout = 1000; // ms

    // Iterate over all persons
    for (const gz::msgs::Pose &poseMsg : _req.pose()) {
        // Extract details from message
        std::string personModel = poseMsg.name();
        int personId = (int) poseMsg.id();
        std::string personName = personModel + "_" + std::to_string(personId);

        // Try to spawn respective person
        req.set_sdf_filename("model://" + personModel);
        req.set_name(personName);
        req.mutable_pose()->CopyFrom(poseMsg);
        bool executed = node.Request(topic, req, timeout, res, result);
        if (executed) {
            if (!result) {
                std::cerr << "[Person] Request to spawn person failed!" << std::endl;
                return false;
            } else {
                PersonStruct person;
                person.id = personId;
                person.name = personName;

                this->persons.insert(std::pair<int, PersonStruct>(personId, person));
            }
        } else {
            std::cerr << "[Person] Request to spawn person timed out, T = " << timeout << std::endl;
            return false;
        }
    }
    return true;
}

bool Person::ServiceWaypoint(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep)
{
    gz::transport::Node node;
    gz::msgs::Pose req;
    gz::msgs::Boolean res;
    bool result;

    int timeout = 1000; //ms

    for (const gz::msgs::Pose &poseMsg : _req.pose()) {
        //Extract details from person
        int personId = (int) poseMsg.id();

        //Find the person and add the waypoint
        auto entry = this->persons.find(personId);
        if (entry != this->persons.end()) {
            entry->second.waypoints.push(gz::msgs::Convert(poseMsg));
        } else{
            std::cerr << "[Person] No person with ID " << personId << "found!" << std::endl;
            return false;
        }    
    }
    return true;
}

std::string Person::CreateModelStr()
{
    std::string modelStr = std::string("<?xml version='1.0'?>") + "\n" +
    "<sdf version='1.6'>" + "\n" +
    "   <model name='Person'>" + "\n" +
    "       <static>true</static>" + "\n" +
    "       <link name='person_link'>" + "\n" +
    "           <visual name='person'>" + "\n" +
    "               <geometry>" + "\n" +
    "                   <mesh>" + "\n" +
    "                       <uri>" + this->meshPath + "</uri>" + "\n" +
    "                       <scale>" + std::to_string(this->scale) + " " + std::to_string(this->scale) + " " + std::to_string(this->scale) + "</scale>" + "\n" +
    "                   </mesh>" + "\n" +
    "               </geometry>" + "\n";
    
    if (this->temperature >= 0.0) {
        modelStr += std::string("               <plugin") + "\n" +
        "                   filename='ignition-gazebo-thermal-system'" + "\n" +
        "                   name='ignition::gazebo::systems::Thermal'>" + "\n" +
        "                   <temperature>" + std::to_string(this->temperature) + "</temperature>" + "\n" +
        "               </plugin>" + "\n";
    }

    modelStr += std::string("           </visual>") + "\n" +
    "       </link>" + "\n" +
    "   </model>" + "\n" +
    "</sdf>";

    return modelStr;
}

bool Person::SpawnModel(std::string modelStr)
{
    gz::transport::Node node;
    gz::msgs::EntityFactory req;
    gz::msgs::Boolean res;
    bool result;

    req.set_sdf(modelStr);
    gz::msgs::Set(req.mutable_pose(), this->modelPose);
    std::string topic = "/world/" + this->worldName + "/create";

    int timeout = 1000; // ms
    bool executed = node.Request(topic, req, timeout, res, result);
    if (executed) {
        if (!result) {
            std::cerr << "[Person] Request to spawn person failed!" << std::endl;
            return false;
        }
    } else {
        std::cerr << "[Person] Request to spawn person timed out, T = " << timeout << std::endl;
        return false;
    }

    return true;
}