#include "swarm.h"

void OnNewThermalFrame(uint16_t *_scanDest, const uint16_t *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &_format)
{
    uint16_t u;
    int size =  _width * _height * _channels;
    memcpy(_scanDest, _scan, size * sizeof(u));
}
void OnNewDepthFrame(float *_scanDest, const float *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &_format)
{
    float u;
    int size =  _width * _height;
    memcpy(_scanDest, _scan, size * sizeof(u));
}

void Swarm::Configure(const gz::sim::Entity &_entity,
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
        std::cerr << "[Swarm] Failed to retrieve world name!" << std::endl;
        return;
    }
    this->worldName = worldNameOptional.value();

    if (!this->ParseGeneralSDF(parentSDF)) {
        return;
    }

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
    this->framePublisher = this->framePublishNode.Advertise<gz::msgs::Frame_V>(topic);
    if (!this->framePublisher) {
        std::cerr << "[Swarm] Error advertising topic [" << service << "]" << std::endl;
        return;
    }

    // Establish a connection to the post rendering operations to take the images
    this->connection = _eventMgr.Connect<gz::sim::events::PostRender>(std::bind(
        &Swarm::PerformPostRenderingOperations,
        this)
    );
}

bool Swarm::ParseGeneralSDF(sdf::ElementPtr _sdf)
{

    if (_sdf->HasElement("depth_offset")) {
        this->depth_offset = _sdf->Get<float>("depth_offset");
    }

    if (_sdf->HasElement("depth_scale")) {
        this->depth_scale = _sdf->Get<float>("depth_scale");
    }

    if (!_sdf->HasElement("direct_thermal_factor")) {
        std::cerr << "[Swarm] Direct thermal factor is missing, specify with <direct_thermal_factor> tag!" << std::endl;
        return false;    
    }
    this->direct_thermal_factor = _sdf->Get<float>("direct_thermal_factor");

    if (!_sdf->HasElement("indirect_thermal_factor")) {
        std::cerr << "[Swarm] Indirect thermal factor is missing, specify with <indirect_thermal_factor> tag!" << std::endl;
        return false;    
    }
    this->indirect_thermal_factor = _sdf->Get<float>("indirect_thermal_factor");

    if (!_sdf->HasElement("lower_thermal_threshold")) {
        std::cerr << "[Swarm] Lower thermal threshold value is missing, specify with <lower_thermal_threshold> tag!" << std::endl;
        return false;    
    }
    this->lower_thermal_threshold = _sdf->Get<float>("lower_thermal_threshold");

    if (!_sdf->HasElement("upper_thermal_threshold")) {
        std::cerr << "[Swarm] Upper thermal threshold value is missing, specify with <upper_thermal_threshold> tag!" << std::endl;
        return false;    
    }
    this->upper_thermal_threshold = _sdf->Get<float>("upper_thermal_threshold");

    return true;
}

void Swarm::PreUpdate(const gz::sim::UpdateInfo &_info,
                      gz::sim::EntityComponentManager &_ecm)
{
    for (auto it = this->drones.begin(); it != this->drones.end(); it++) {
        Drone &drone = it->second;

        if (!drone.waypoints.empty() && drone.reachedWaypoint == false) {
            gz::sim::Entity entity = _ecm.EntityByComponents(gz::sim::components::Name(drone.name));
            gz::sim::Model model = gz::sim::Model(entity);
            model.SetWorldPoseCmd(_ecm, drone.waypoints.front());
            drone.reachedWaypoint = true;
        }
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
            } else {
                Drone drone;
                drone.id = droneId;
                drone.name = droneName;

                this->drones.insert(std::pair<int, Drone>(droneId, drone));
            }
        } else {
            std::cerr << "[Swarm] Request to spawn drone timed out, T = " << timeout << std::endl;
            return false;
        }
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
        int droneId = (int) poseMsg.id();

        // Find the drone and add the waypoint
        auto entry = this->drones.find(droneId);
        if (entry != this->drones.end()) {
            entry->second.waypoints.push(gz::msgs::Convert(poseMsg));
        } else {
            std::cerr << "[Swarm] No drone with ID " << droneId << " found!" << std::endl;
            return false;
        }
    }

    return true;
}

void Swarm::PerformPostRenderingOperations()
{
    std::vector<int> ids;
    std::vector<gz::math::Pose3d> poses;

    for (auto it = this->drones.begin(); it != this->drones.end(); it++) {
        Drone &drone = it->second;

        if (drone.reachedWaypoint) {
            // Store the drones id and pose
            ids.push_back(drone.id);
            poses.push_back(drone.waypoints.front());

            // Remove the waypoint
            drone.waypoints.pop();
            drone.reachedWaypoint = false;
        }
    }

    if (!ids.empty()) {
        
        //std::vector<std::pair<cv::Mat, cv::Mat>> images = this->CreateImages(poses);
        std::vector<std::tuple<cv::Mat, cv::Mat, cv::Mat>> images = this->CreateImages(poses);
        gz::msgs::Frame_V frames;

        std::vector<const char*> dynamic_arrays;
        std::vector<const uint16_t*> dynamic_arrays_depth;
        for (int i = 0; i < ids.size(); i++) {
            int id = ids.at(i);

            gz::msgs::Frame *frame = frames.add_frame();

            frame->set_id(id);
            frame->mutable_pose()->CopyFrom(gz::msgs::Convert(poses.at(i)));

            // RGB image with 1 Byte per channel (3 Channels)
            cv::Mat rgbImage = std::get<0>(images.at(i));
            char* rgb_img_data = new char[rgbImage.cols * rgbImage.rows * 3];
            std::memcpy(rgb_img_data, rgbImage.data, rgbImage.cols * rgbImage.rows * 3);
            dynamic_arrays.push_back(rgb_img_data);

            frame->mutable_rgbimage()->set_width(rgbImage.cols);
            frame->mutable_rgbimage()->set_height(rgbImage.rows);
            frame->mutable_rgbimage()->set_step(rgbImage.cols * 3);     
            frame->mutable_rgbimage()->set_data(rgb_img_data, rgbImage.cols * rgbImage.rows * 3);
            frame->mutable_rgbimage()->set_pixel_format_type(gz::msgs::PixelFormatType::RGB_INT8);

            // Grayscale image with 1 Byte per channel (1 Channel)
            cv::Mat thermalImage = std::get<1>(images.at(i));
            char* thermal_img_data = new char[thermalImage.cols * thermalImage.rows];
            std::memcpy(thermal_img_data, thermalImage.data, thermalImage.cols * thermalImage.rows);
            dynamic_arrays.push_back(thermal_img_data);

            frame->mutable_thermalimage()->set_width(thermalImage.cols);
            frame->mutable_thermalimage()->set_height(thermalImage.rows);
            frame->mutable_thermalimage()->set_step(thermalImage.cols);     
            frame->mutable_thermalimage()->set_data(thermal_img_data, thermalImage.cols * thermalImage.rows);
            frame->mutable_thermalimage()->set_pixel_format_type(gz::msgs::PixelFormatType::L_INT8);
        
        
            cv::Mat depthImage = std::get<2>(images.at(i));
            uint16_t* depth_img_data = new uint16_t[depthImage.cols * depthImage.rows];
            std::memcpy(depth_img_data, depthImage.data, depthImage.cols * depthImage.rows * 2);
            dynamic_arrays_depth.push_back(depth_img_data);

            frame->mutable_depthimage()->set_width(depthImage.cols);
            frame->mutable_depthimage()->set_height(depthImage.rows);
            frame->mutable_depthimage()->set_step(depthImage.cols * 2);     
            frame->mutable_depthimage()->set_data(depth_img_data, depthImage.cols * depthImage.rows * 2);
            frame->mutable_depthimage()->set_pixel_format_type(gz::msgs::PixelFormatType::L_INT16);
        }

        // Publish frame message
        if (!this->framePublisher.Publish(frames)) {
            std::cerr << "[Swarm] Failed to publish frames!" << std::endl;
            return;
        }

        for (const char* dynamic_array : dynamic_arrays) {
            delete[] dynamic_array;
        }

        for (const uint16_t* dynamic_array : dynamic_arrays_depth) {
            delete[] dynamic_array;
        }
    }
}

std::vector<std::tuple<cv::Mat, cv::Mat, cv::Mat>> Swarm::CreateImages(std::vector<gz::math::Pose3d> poses)
{
    gz::rendering::ScenePtr scene = gz::rendering::sceneFromFirstRenderEngine();
    gz::rendering::ThermalCameraPtr thermal_camera;
    gz::rendering::DepthCameraPtr depth_camera;
    gz::rendering::CameraPtr rgb_camera;

    for (unsigned int i = 0; i < scene->NodeCount(); ++i) {    
        auto tmp_thermal_camera = std::dynamic_pointer_cast<gz::rendering::ThermalCamera>(scene->NodeByIndex(i));
        if (tmp_thermal_camera != nullptr) {
            if (tmp_thermal_camera->Name() == "drone_camera::camera_link::thermal_camera") {
                thermal_camera = std::dynamic_pointer_cast<gz::rendering::ThermalCamera>(scene->NodeByIndex(i));
                thermal_camera->SetMinTemperature(0.0);
                thermal_camera->SetMaxTemperature(655.35);
                thermal_camera->SetLinearResolution(0.01);
            }
        }
        
        auto tmp_rgb_camera = std::dynamic_pointer_cast<gz::rendering::Camera>(scene->NodeByIndex(i));
        if (tmp_rgb_camera != nullptr) {
            if (tmp_rgb_camera->Name() == "drone_camera::camera_link::rgb_camera") {
                rgb_camera = std::dynamic_pointer_cast<gz::rendering::Camera>(scene->NodeByIndex(i));
            }
        }

        auto tmp_depth_camera = std::dynamic_pointer_cast<gz::rendering::DepthCamera>(scene->NodeByIndex(i));
        if (tmp_depth_camera != nullptr) {
            if (tmp_depth_camera->Name() == "drone_camera::camera_link::depth_camera") {
                depth_camera = std::dynamic_pointer_cast<gz::rendering::DepthCamera>(scene->NodeByIndex(i));
            }
        }
    }

    // Take the thermal images
    std::vector<cv::Mat> thermal_images;
    for (std::size_t i = 0; i < poses.size(); i++) {
        thermal_images.push_back(this->TakePictureThermal(thermal_camera, poses[i]));
    }

    // Take normal rgb images
    std::vector<cv::Mat> rgb_light_images;
    for (std::size_t i = 0; i < poses.size(); i++) {
        cv::Mat mat = this->TakePictureRGB(rgb_camera, poses[i]);
        cv::cvtColor(mat, mat, 4);  // convert from rgb to bgr
        rgb_light_images.push_back(mat);
    }

    // Take depth images
    std::vector<cv::Mat> depth_images;
    for (std::size_t i = 0; i < poses.size(); i++) {
        depth_images.push_back(this->TakePictureDepth(depth_camera, poses[i]));
    }

    // Disable lights
    std::vector<double> intensities;
    for (unsigned int i = 0; i < scene->LightCount(); ++i) {
        auto light = std::dynamic_pointer_cast<gz::rendering::Light>(scene->LightByIndex(i));
        if (light != nullptr) {
            intensities.push_back(light->Intensity());
            light->SetIntensity(0.0);
        } else {
            intensities.push_back(0.0);
        }
    }

    // Take rgb images with only ambient lighting
    std::vector<cv::Mat> rgb_dark_images;
    for (std::size_t i = 0; i < poses.size(); i++) {
        cv::Mat mat = this->TakePictureRGB(rgb_camera, poses[i]);
        cv::cvtColor(mat, mat, 4);  // convert from rgb to bgr
        rgb_dark_images.push_back(mat);
    }

    // Enable lights
    for (unsigned int i = 0; i < scene->LightCount(); ++i) {
        auto light = std::dynamic_pointer_cast<gz::rendering::Light>(scene->LightByIndex(i));
        if (light != nullptr) {
            light->SetIntensity(intensities.at(i));
        }
    }

    //TODO: Compute Depth Output

    // Compute thermal output
    //std::vector<std::pair<cv::Mat, cv::Mat>> images;
    std::vector<std::tuple<cv::Mat, cv::Mat, cv::Mat>> images;
    
    for (unsigned int i = 0; i < rgb_light_images.size(); i++) {
        cv::Mat rgbLight = rgb_light_images.at(i).clone();
        cv::Mat rgbDark = rgb_dark_images.at(i).clone();
        cv::Mat thermal_raw = thermal_images.at(i).clone();
        cv::Mat thermal;

        cv::Mat depthOut;
        cv::Mat &depth = depth_images.at(i);

        depth += this->depth_offset;
        depth *= this->depth_scale;
        depth.convertTo(depthOut, CV_16UC1);

        thermal_raw.convertTo(thermal, CV_32F);
        thermal = thermal * thermal_camera->LinearResolution();

        // Calculate the brightness values of both rgb images
        // as grayscale images in the range from 0 to 1
        cv::Mat brightnessLight, brightnessDark;
        cv::cvtColor(rgbLight, brightnessLight, cv::COLOR_RGB2GRAY);
        cv::cvtColor(rgbDark, brightnessDark, cv::COLOR_RGB2GRAY);
        brightnessLight.convertTo(brightnessLight, CV_32F);
        brightnessDark.convertTo(brightnessDark, CV_32F);
        brightnessLight /= 255.0;
        brightnessDark /= 255.0;

        // Calculate the difference between the two brightness
        // images, which indicates the effect of the sun
        cv::Mat lightInfluence;
        cv::subtract(brightnessLight, brightnessDark, lightInfluence);

        // Calculate the direct light contribution
        cv::Mat directLightContribution;
        cv::multiply((1.0 - brightnessDark), lightInfluence, directLightContribution);

        // Convert contributions to kelvin
        cv::Mat directLightContributionThermal;
        cv::multiply(directLightContribution, this->direct_thermal_factor,  directLightContributionThermal);

        cv::Mat indirectLightContributionThermal;
        cv::multiply((1.0 - brightnessDark), this->indirect_thermal_factor, indirectLightContributionThermal);

        // Calculate the final thermal image
        float res = thermal_camera->LinearResolution();
        float absMin = thermal_camera->MinTemperature();
        float absMax = thermal_camera->MaxTemperature();
        float minTemp = 288.15;
        float maxTemp = 303.15;

        cv::Mat thermalOut = thermal + directLightContributionThermal + indirectLightContributionThermal;

        cv::Mat mask;

        cv::inRange(thermalOut, cv::Scalar(this->lower_thermal_threshold), cv::Scalar(this->upper_thermal_threshold), mask);
        thermalOut.copyTo(thermalOut, mask);
        thermalOut = (thermalOut - this->lower_thermal_threshold) / (this->upper_thermal_threshold - this->lower_thermal_threshold) * 255;

        thermalOut.convertTo(thermalOut, CV_8U);
        rgbLight.convertTo(rgbLight, CV_8UC3);

        images.push_back(std::make_tuple(rgbLight, thermalOut, depthOut));
    }

    for (cv::Mat mat : thermal_images) {
        delete[] mat.ptr();
    }
    for (cv::Mat mat : rgb_light_images) {
        delete[] mat.ptr();
    }
    for (cv::Mat mat : rgb_dark_images) {
        delete[] mat.ptr();
    }
    for (cv::Mat mat : depth_images) {
        delete[] mat.ptr();
    }
    return images;
}

cv::Mat Swarm::TakePictureThermal(const gz::rendering::ThermalCameraPtr _camera,
                                    const gz::math::Pose3d &_pose)
{
    unsigned int width = _camera->ImageWidth();
    unsigned int height = _camera->ImageHeight();
    
    _camera->SetWorldPose(_pose);

    uint16_t *thermalData = new uint16_t[width * height];
    gz::common::ConnectionPtr connection =
      _camera->ConnectNewThermalFrame(
          std::bind(&::OnNewThermalFrame, thermalData,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));
    _camera->Update();

    return cv::Mat(height, width, CV_16UC1, thermalData);;
}

cv::Mat Swarm::TakePictureRGB(const gz::rendering::CameraPtr _camera,
                                const gz::math::Pose3d &_pose)
{
    unsigned int width = _camera->ImageWidth();
    unsigned int height = _camera->ImageHeight();

    _camera->SetWorldPose(_pose);

    gz::rendering::Image cameraImage = _camera->CreateImage();
    _camera->Capture(cameraImage);

    uint8_t *rbgData = new uint8_t[width * height * 3];
    memcpy(rbgData, cameraImage.Data<uint8_t>(), width * height * 3 * sizeof(uint8_t));

    return cv::Mat(height, width, CV_8UC3, rbgData);
}

cv::Mat Swarm::TakePictureDepth(const gz::rendering::DepthCameraPtr _camera,
                                const gz::math::Pose3d &_pose)
{
    unsigned int width = _camera->ImageWidth();
    unsigned int height = _camera->ImageHeight();

    _camera->SetWorldPose(_pose);

    float *depthData = new float[width * height];
    gz::common::ConnectionPtr connection =
      _camera->ConnectNewDepthFrame(
          std::bind(&::OnNewDepthFrame, depthData,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));
    _camera->Update();

    return cv::Mat(height, width, CV_32F, depthData);
}