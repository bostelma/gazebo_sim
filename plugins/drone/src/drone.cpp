#include "drone.h"

void Drone::Configure(const gz::sim::Entity &_entity,
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
        std::cerr << "[Drone] Failed to rerieve world name!" << std::endl;
        return;
    }
    std::string worldName = worldNameOptional.value();

    // Extract the name of this drone
    std::optional<std::string> droneNameOptional = _ecm.ComponentData<gz::sim::components::Name>(_entity);
    if (!droneNameOptional) {
        std::cerr << "[Drone] Failed to rerieve drone name!" << std::endl;
        return;
    }
    std::string droneName = droneNameOptional.value();
    this->droneEntity = _entity;
    this->droneModel = gz::sim::Model(this->droneEntity);

    // Disable gravity
    _ecm.SetComponentData<gz::sim::components::Gravity>(this->droneEntity, gz::math::Vector3d(0.0, 0.0, 0.0));

    // Create the waypoint topic
    std::string waypointTopic = "/world/" + worldName + "/model/" + droneName + "/waypoint";
    if (!this->waypointNode.Advertise(waypointTopic, &Drone::WaypointService, this)) {
        std::cerr << "[Drone] Failed to advertise topic: " << waypointTopic << std::endl;
    }

    // Create the frame topic
    std::string frameTopic = "/world/" + worldName + "/model/" + droneName + "/frame";
    this->framePub = this->frameNode.Advertise<gz::msgs::Frame>(frameTopic);
    if (!this->framePub) {
        std::cerr << "[Drone] Failed to advertise topic: " << frameTopic << std::endl;
    }

    // Subscribe to the own camera image topic
    std::string imageTopic = "/world/" + worldName + "/model/" + droneName + "/link/camera_link/sensor/camera/image";
    if (!this->imageNode.Subscribe(imageTopic, &Drone::OnNewRGBImage, this)) {
        std::cerr << "[Drone] Failed to subscribe topic: " << imageTopic << std::endl;
    }

    // Subscribe to the own thermal image topic
    std::string thermalTopic = "/world/" + worldName + "/model/" + droneName + "/link/camera_link/sensor/thermal_camera/image";
    if (!this->thermalNode.Subscribe(thermalTopic, &Drone::OnNewThermalImage, this)) {
        std::cerr << "[Drone] Failed to subscribe topic: " << thermalTopic << std::endl;
    }
}

void Drone::PreUpdate(const gz::sim::UpdateInfo &_info,
                      gz::sim::EntityComponentManager &_ecm)
{
    if (this->waypoints.empty()) {
        return;
    }

    this->droneModel.SetWorldPoseCmd(_ecm, this->waypoints.front());
}

void Drone::PostUpdate(const gz::sim::UpdateInfo &_info,
                       const gz::sim::EntityComponentManager &_ecm)
{
    // Store the current pose in a queue together with the current simulation time
    this->poses.push({
        _info.simTime,
        _ecm.ComponentData<gz::sim::components::Pose>(this->droneEntity).value()
    });
}

bool Drone::WaypointService(const gz::msgs::Pose &_req, gz::msgs::Boolean &_rep)
{
    this->waypoints.push(gz::msgs::Convert(_req));
    return true;
}

void Drone::OnNewRGBImage(const gz::msgs::Image &imageMsg)
{
    if (this->waypoints.empty()) {
        return;
    }

    // Get the position of the drone at the time the image was taken
    gz::msgs::Time msgTime = imageMsg.header().stamp();
    std::chrono::steady_clock::duration imageTime = std::chrono::seconds(msgTime.sec()) + std::chrono::nanoseconds(msgTime.nsec());

    std::tuple<std::chrono::steady_clock::duration, gz::math::Pose3d> entry = this->poses.front();
    this->poses.pop();
    while (std::get<0>(entry) < imageTime) {
        entry = this->poses.front();
        this->poses.pop();
    }
    gz::math::Pose3d imagePose = std::get<1>(entry);
    
    // Check if the waypoint has been reached
    gz::math::Pose3d currentWaypoint = this->waypoints.front();
    if (imagePose.Equal(currentWaypoint, 0.01)) {
        if (this->thermalImage != nullptr) {
            // Publish frame
            gz::msgs::Frame frame;
            
            frame.mutable_header()->mutable_stamp()->CopyFrom(msgTime);
            frame.mutable_pose()->CopyFrom(gz::msgs::Convert(imagePose));
            frame.mutable_rgbimage()->CopyFrom(imageMsg);
            frame.mutable_thermalimage()->CopyFrom(*(this->thermalImage));

            // Publish frame message
            if (!this->framePub.Publish(frame)) {
                std::cerr << "[Drone] Failed to publish frame!" << std::endl;
                return;
            }
            // Remove the waypoint
            this->waypoints.pop();

            this->rgbImage = nullptr;
            this->thermalImage = nullptr;
        } else {
            // Store image data
            this->rgbImage = std::make_shared<gz::msgs::Image>(imageMsg);
        }
    }
}

void Drone::OnNewThermalImage(const gz::msgs::Image &imageMsg)
{
    if (this->waypoints.empty()) {
        return;
    }

    // Get the position of the drone at the time the image was taken
    gz::msgs::Time msgTime = imageMsg.header().stamp();
    std::chrono::steady_clock::duration imageTime = std::chrono::seconds(msgTime.sec()) + std::chrono::nanoseconds(msgTime.nsec());

    std::tuple<std::chrono::steady_clock::duration, gz::math::Pose3d> entry = this->poses.front();
    this->poses.pop();
    while (std::get<0>(entry) < imageTime) {
        entry = this->poses.front();
        this->poses.pop();
    }
    gz::math::Pose3d imagePose = std::get<1>(entry);
    
    // Check if the waypoint has been reached
    gz::math::Pose3d currentWaypoint = this->waypoints.front();
    if (imagePose.Equal(currentWaypoint, 0.01)) {
        if (this->rgbImage != nullptr) {
            // Publish frame
            gz::msgs::Frame frame;
            
            frame.mutable_header()->mutable_stamp()->CopyFrom(msgTime);
            frame.mutable_pose()->CopyFrom(gz::msgs::Convert(imagePose));
            frame.mutable_rgbimage()->CopyFrom(*(this->rgbImage));
            frame.mutable_thermalimage()->CopyFrom(imageMsg);

            // Publish frame message
            if (!this->framePub.Publish(frame)) {
                std::cerr << "[Drone] Failed to publish frame!" << std::endl;
                return;
            }
            // Remove the waypoint
            this->waypoints.pop();

            this->rgbImage = nullptr;
            this->thermalImage = nullptr;
        } else {
            // Store image data
            this->thermalImage = std::make_shared<gz::msgs::Image>(imageMsg);
        }
    }
}

GZ_ADD_PLUGIN(
    Drone,
    gz::sim::System,
    Drone::ISystemConfigure,
    Drone::ISystemPreUpdate,
    Drone::ISystemPostUpdate
)