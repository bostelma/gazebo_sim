# include "swarm.h"

void Swarm::Configure(const gz::sim::Entity &_entity,
                               const std::shared_ptr<const sdf::Element> &_sdf,
                               gz::sim::EntityComponentManager &_ecm,
                               gz::sim::EventManager &_eventMgr)
{
    // Extract the world name
    gz::sim::Entity worldEntity = _ecm.EntityByComponents(gz::sim::components::World());
    std::optional<std::string> worldNameOptional = _ecm.ComponentData<gz::sim::components::Name>(worldEntity);
    if (!worldNameOptional) {
        std::cerr << "[Swarm] Failed to rerieve world name!" << std::endl;
        return;
    }
    this->worldName = worldNameOptional.value();

    // Advertise the spawn service
    std::string service = "/world/" + this->worldName + "/swarm/spawn";
    if (!this->spawnNode.Advertise(service, &Swarm::ServiceSpawn, this)) {
        std::cerr << "[Swarm] Error advertising service [" << service << "]" << std::endl;
        return;
    }

    // Advertise the waypoint service
    service = "/world/" + this->worldName + "/swarm/waypoint";
    if (!this->waypointNode.Advertise(service, &Swarm::ServiceWaypoint, this)) {
        std::cerr << "[Swarm] Error advertising service [" << service << "]" << std::endl;
        return;
    }

    // Publish the frame topic
    std::string topic = "/world/" + this->worldName + "/swarm/frame";
    this->framePublisher = this->framePublishNode.Advertise<gz::msgs::Frame>(topic);
    if (!this->framePublisher) {
        std::cerr << "[Swarm] Error advertising topic [" << service << "]" << std::endl;
        return;
    }
}

void Swarm::Reset(const gz::sim::UpdateInfo &_info,
                           gz::sim::EntityComponentManager &_ecm)
{
    std::cout << "Reset Swarm" << std::endl;
}


bool Swarm::ServiceSpawn(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep)
{
    // Setup spawn service
    gz::transport::Node node;
    gz::msgs::EntityFactory req;
    gz::msgs::Boolean res;
    bool result;

    std::string topic = "/world/" + worldName + "/create";
    int timeout = 1000; // ms

    // Iterate over all drones
    for (const gz::msgs::Pose &poseMsg : _req.pose()) {
        // Extract details from message
        std::string droneModel = poseMsg.name();
        int droneId = (int) poseMsg.id();
        std::string droneName = droneModel + "_" + std::to_string(droneId);

        // Try to spawn respective drone
        req.set_sdf_filename("model://" + droneModel);
        req.set_name(droneName);
        req.mutable_pose()->CopyFrom(poseMsg);
        bool executed = node.Request(topic, req, timeout, res, result);
        if (executed) {
            if (!result) {
                std::cerr << "[Swarm] Request to spawn drone failed!" << std::endl;
                return false;
            }
        } else {
            std::cerr << "[Swarm] Request to spawn drone timed out, T = " << timeout << std::endl;
            return false;
        }

        // Create a custom callback for each drone's frame message
        auto callBack = std::function<void(const gz::msgs::Frame &)>(
            [=](const gz::msgs::Frame &frame)
            {
                gz::msgs::Frame taggedFrame(frame);
                taggedFrame.set_name(droneModel);
                taggedFrame.set_id(droneId);

                if (!this->framePublisher.Publish(taggedFrame)) {
                    std::cerr << "[Swarm] Failed to publish frame!" << std::endl;
                }
            }
        );
        std::shared_ptr<gz::transport::Node> frameNode = std::make_shared<gz::transport::Node>();
        std::string frameTopic = "world/" + worldName + "/model/" + droneName + "/frame";
        if (!frameNode->Subscribe(frameTopic, callBack)) {
            std::cerr << "[DroneController] Failed to subscripe topic: " << frameTopic << std::endl;
        }
        this->frameNodes.push_back(frameNode);
    }
    return true;
}

bool Swarm::ServiceWaypoint(const gz::msgs::Pose_V &_req, gz::msgs::Boolean &_rep)
{
    // Setup waypoint service of drone
    gz::transport::Node node;
    gz::msgs::Pose req;
    gz::msgs::Boolean res;
    bool result;

    int timeout = 1000; // ms

    // Iterate over all waypoints
    for (const gz::msgs::Pose &poseMsg : _req.pose()) {
        // Extract details from message
        std::string droneModel = poseMsg.name();
        int droneId = (int) poseMsg.id();

        // Try to set waypoint
        std::string topic = "/world/" + worldName + "/model/" + droneModel + "_" + std::to_string(droneId) + "/waypoint";
        req.CopyFrom(poseMsg);
        bool executed = node.Request(topic, req, timeout, res, result);
        if (executed) {
            if (!result) {
                std::cerr << "[Swarm] Request to add waypoint failed!" << std::endl;
                return false;
            }
        } else {
            std::cerr << "[Swarm] Request to add waypoint timed out, T = " << timeout << std::endl;
            return false;
        }
    }

    return true;
}