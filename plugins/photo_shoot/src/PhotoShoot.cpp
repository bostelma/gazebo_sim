#include "PhotoShoot.h"

#include <iostream>

void error_aware_imwrite(const std::filesystem::path& output_file_path, const cv::Mat& mat);
void save_image(const std::filesystem::path& output_file_path, const cv::Mat& mat);
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

void PhotoShoot::Configure(const gz::sim::Entity &_entity,
                           const std::shared_ptr<const sdf::Element> &_sdf,
                           gz::sim::EntityComponentManager &_ecm,
                           gz::sim::EventManager &_eventMgr)
{
    // Clone _sdf, to have a non constant version (GetElement is non-constant)
    sdf::ElementPtr parentSDF = _sdf->Clone();

    if (!this->ParseGeneralSDF(parentSDF)) {
        return;
    }

    this->connection = _eventMgr.Connect<gz::sim::events::PostRender>(std::bind(
        &PhotoShoot::PerformPostRenderingOperations,
        this)
    );
}

bool PhotoShoot::ParseGeneralSDF(sdf::ElementPtr _sdf)
{
    if (!_sdf->HasElement("poses")) {
        std::cerr << "[PhotoShoot] Poses are missing, specify with <poses> tag!" << std::endl;
        return false;    
    }

    if (!_sdf->HasElement("directory")) {
        std::cerr << "[PhotoShoot] Directory is missing, specify with <directory> tag!" << std::endl;
        return false;
    }
    this->directory = _sdf->Get<std::string>("directory");

    if (_sdf->HasElement("prefix")) {
        this->prefix = _sdf->Get<std::string>("prefix");
    }

    if (_sdf->HasElement("depth_offset")) {
        this->depth_offset = _sdf->Get<float>("depth_offset");
    }

    if (_sdf->HasElement("depth_scale")) {
        this->depth_scale = _sdf->Get<float>("depth_scale");
    }

    if (_sdf->HasElement("save_rgb")) {
        this->save_rgb = _sdf->Get<bool>("save_rgb");
    }

    if (_sdf->HasElement("save_thermal")) {
        this->save_thermal = _sdf->Get<bool>("save_thermal");
    }

    if (_sdf->HasElement("save_depth")) {
        this->save_depth = _sdf->Get<bool>("save_depth");
    }

    if (!_sdf->HasElement("direct_thermal_factor")) {
        std::cerr << "[PhotoShoot] Direct thermal factor is missing, specify with <direct_thermal_factor> tag!" << std::endl;
        return false;    
    }
    this->direct_thermal_factor = _sdf->Get<float>("direct_thermal_factor");

    if (!_sdf->HasElement("indirect_thermal_factor")) {
        std::cerr << "[PhotoShoot] Indirect thermal factor is missing, specify with <indirect_thermal_factor> tag!" << std::endl;
        return false;    
    }
    this->indirect_thermal_factor = _sdf->Get<float>("indirect_thermal_factor");

    if (!_sdf->HasElement("lower_thermal_threshold")) {
        std::cerr << "[PhotoShoot] Lower thermal threshold value is missing, specify with <lower_thermal_threshold> tag!" << std::endl;
        return false;    
    }
    this->lower_thermal_threshold = _sdf->Get<float>("lower_thermal_threshold");

    if (!_sdf->HasElement("upper_thermal_threshold")) {
        std::cerr << "[PhotoShoot] Upper thermal threshold value is missing, specify with <upper_thermal_threshold> tag!" << std::endl;
        return false;    
    }
    this->upper_thermal_threshold = _sdf->Get<float>("upper_thermal_threshold");

    sdf::ElementPtr poseSDF = _sdf->GetElement("poses")->GetElement("pose");
    while (poseSDF != nullptr) {
        
        gz::math::Pose3d pose;
        poseSDF->GetValue()->Get(pose);
        this->poses.push_back(pose);

        poseSDF = poseSDF->GetNextElement("pose");
    }

    return true;
}

void PhotoShoot::PerformPostRenderingOperations()
{
    if (this->takePicture) {
        gz::rendering::ScenePtr scene = gz::rendering::sceneFromFirstRenderEngine();
        gz::rendering::CameraPtr rgb_camera;
        gz::rendering::DepthCameraPtr depth_camera;
        gz::rendering::ThermalCameraPtr thermal_camera;
        
        for (unsigned int i = 0; i < scene->NodeCount(); ++i) { 

            auto tmp_rgb_camera = std::dynamic_pointer_cast<gz::rendering::Camera>(scene->NodeByIndex(i));
            if (tmp_rgb_camera != nullptr) {
                if (tmp_rgb_camera->Name() == "photo_shoot::camera_link::rgb_camera") {
                    rgb_camera = std::dynamic_pointer_cast<gz::rendering::Camera>(scene->NodeByIndex(i));
                }
            }

            auto tmp_depth_camera = std::dynamic_pointer_cast<gz::rendering::DepthCamera>(scene->NodeByIndex(i));
            if (tmp_depth_camera != nullptr) {
                if (tmp_depth_camera->Name() == "photo_shoot::camera_link::depth_camera") {
                    depth_camera = std::dynamic_pointer_cast<gz::rendering::DepthCamera>(scene->NodeByIndex(i));
                }
            }

            auto tmp_thermal_camera = std::dynamic_pointer_cast<gz::rendering::ThermalCamera>(scene->NodeByIndex(i));
            if (tmp_thermal_camera != nullptr) {
                if (tmp_thermal_camera->Name() == "photo_shoot::camera_link::thermal_camera") {
                    thermal_camera = std::dynamic_pointer_cast<gz::rendering::ThermalCamera>(scene->NodeByIndex(i));
                    thermal_camera->SetMinTemperature(0.0);
                    thermal_camera->SetMaxTemperature(655.35);
                    thermal_camera->SetLinearResolution(0.01);
                }
            }
        }

        // Take the thermal images
        std::vector<cv::Mat> thermal_images;
        if (this->save_thermal) {
            for (std::size_t i = 0; i < this->poses.size(); i++) {
                thermal_images.push_back(this->TakePictureThermal(thermal_camera, this->poses[i]));
            }
        }

        // Take normal rgb images
        std::vector<cv::Mat> rgb_light_images;
        if (this->save_rgb || this->save_thermal) {
            for (std::size_t i = 0; i < this->poses.size(); i++) {
                cv::Mat mat = this->TakePictureRGB(rgb_camera, this->poses[i]);
                cv::cvtColor(mat, mat, 4);  // convert from rgb to bgr
                rgb_light_images.push_back(mat);
            }
        }

        // Take depth images
        std::vector<cv::Mat> depth_images;
        if (this->save_depth) {
            for (std::size_t i = 0; i < this->poses.size(); i++) {
                depth_images.push_back(this->TakePictureDepth(depth_camera, this->poses[i]));
            }
        }


        // Take rgb images with only ambient lighting
        std::vector<cv::Mat> rgb_dark_images;
        if (this->save_thermal) {
            
            for (unsigned int i = 0; i < scene->LightCount(); ++i) {
                auto light = std::dynamic_pointer_cast<gz::rendering::Light>(scene->LightByIndex(i));
                if (light != nullptr) {
                    light->SetIntensity(0.0);
                }
            }

            for (std::size_t i = 0; i < this->poses.size(); i++) {
                cv::Mat mat = this->TakePictureRGB(rgb_camera, this->poses[i]);
                cv::cvtColor(mat, mat, 4);  // convert from rgb to bgr
                rgb_dark_images.push_back(mat);
            }
        }

        // Compute outputs
        for (unsigned int i = 0; i < this->poses.size(); i++) {

            // Process Depth image
            cv::Mat depthOut;
            if (this->save_depth) {
                cv::Mat &depth = depth_images.at(i);

                depth += this->depth_offset;
                depth *= this->depth_scale;

                depth.convertTo(depthOut, CV_16U);
            }            

            // Process Thermal image
            cv::Mat thermalOut;
            if (this->save_thermal) {
                cv::Mat &rgbLight = rgb_light_images.at(i);
                cv::Mat &rgbDark = rgb_dark_images.at(i);
                cv::Mat thermal;
                thermal_images.at(i).convertTo(thermal, CV_32F);
                thermal = thermal * thermal_camera->LinearResolution();

                //cv::imwrite("../../data/photo_shoot/rgbLight.png", rgbLight);
                //cv::imwrite("../../data/photo_shoot/rgbDark.png", rgbDark);

                // Calculate the brightness values of both rgb images
                // as grayscale images in the range from 0 to 1
                cv::Mat brightnessLight, brightnessDark;
                cv::cvtColor(rgbLight, brightnessLight, cv::COLOR_RGB2GRAY);
                cv::cvtColor(rgbDark, brightnessDark, cv::COLOR_RGB2GRAY);
                brightnessLight.convertTo(brightnessLight, CV_32F);
                brightnessDark.convertTo(brightnessDark, CV_32F);
                brightnessLight /= 255.0;
                brightnessDark /= 255.0;

                //cv::imwrite("../../data/photo_shoot/brightnessLight.png", brightnessLight * 255.0);
                //cv::imwrite("../../data/photo_shoot/brightnessDark.png", brightnessDark * 255.0);

                // Calculate the difference between the two brightness
                // images, which indicates the effect of the sun
                cv::Mat lightInfluence;
                cv::subtract(brightnessLight, brightnessDark, lightInfluence);

                //cv::imwrite("../../data/photo_shoot/lightInfluence.png", lightInfluence * 255.0);

                // Calculate the direct light contribution
                cv::Mat directLightContribution;
                cv::multiply((1.0 - brightnessDark), lightInfluence, directLightContribution);

                //cv::imwrite("../../data/photo_shoot/directLightContribution.png", directLightContribution * 255.0);

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

                thermalOut = thermal + directLightContributionThermal + indirectLightContributionThermal;

                //double min, max;
                //cv::minMaxLoc(thermal, &min, &max);
                //std::cout << "Original Thermal: Min = " + std::to_string(min) + ", Max = " + std::to_string(max) << std::endl;
                //cv::minMaxLoc(directLightContributionThermal, &min, &max);
                //std::cout << "Direct Light Contribution Thermal: Min = " + std::to_string(min) + ", Max = " + std::to_string(max) << std::endl;
                //cv::minMaxLoc(indirectLightContributionThermal, &min, &max);
                //std::cout << "Indirect Light Contribution Thermal: Min = " + std::to_string(min) + ", Max = " + std::to_string(max) << std::endl;

                cv::Mat mask;
                //cv::inRange(thermal, cv::Scalar(this->lower_thermal_threshold), cv::Scalar(this->upper_thermal_threshold), mask);
                //thermal.copyTo(thermal, mask);
                //thermal = (thermal - this->lower_thermal_threshold) / (this->upper_thermal_threshold - this->lower_thermal_threshold) * 255;

                cv::inRange(thermalOut, cv::Scalar(this->lower_thermal_threshold), cv::Scalar(this->upper_thermal_threshold), mask);
                thermalOut.copyTo(thermalOut, mask);
                thermalOut = (thermalOut - this->lower_thermal_threshold) / (this->upper_thermal_threshold - this->lower_thermal_threshold) * 255;

                //cv::imwrite("../../data/photo_shoot/thermalIn.png", thermal);
                //cv::imwrite("../../data/photo_shoot/thermalOut.png", thermalOut);
            }

            // Save images
            std::string connected_prefix = this->prefix + (this->prefix.empty() ? "" : "_");
            std::filesystem::path directory(this->directory);
            std::filesystem::path file;
            std::filesystem::path fullPath;

            if (this->save_thermal) {
                file = std::filesystem::path(connected_prefix + "pose_" + std::to_string(i) + "_thermal.png");
                fullPath = directory / file;
                //error_aware_imwrite(fullPath, thermalOut);
                save_image(fullPath, thermalOut);
            }
            
            if (this->save_depth) {
                file = std::filesystem::path(connected_prefix + "pose_" + std::to_string(i) + "_depth.png");
                fullPath = directory / file;
                //error_aware_imwrite(fullPath, depthOut);
                save_image(fullPath, depthOut);
            }
            
            if (this->save_rgb) {
                file = std::filesystem::path(connected_prefix + "pose_" + std::to_string(i) + "_rgb.png");
                fullPath = directory / file;
                //error_aware_imwrite(fullPath, rgb_light_images.at(i));
                save_image(fullPath, rgb_light_images.at(i));
            }
            
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

        this->takePicture = false;
    }
}

cv::Mat PhotoShoot::TakePictureThermal(const gz::rendering::ThermalCameraPtr _camera,
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

    return cv::Mat(height, width, CV_16UC1, thermalData);
}

cv::Mat PhotoShoot::TakePictureRGB(const gz::rendering::CameraPtr _camera,
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

cv::Mat PhotoShoot::TakePictureDepth(const gz::rendering::DepthCameraPtr _camera,
                                const gz::math::Pose3d &_pose)
{
    unsigned int width = _camera->ImageWidth();
    unsigned int height = _camera->ImageHeight();

    _camera->SetWorldPose(_pose);

    float *thermalData = new float[width * height];
    gz::common::ConnectionPtr connection =
      _camera->ConnectNewDepthFrame(
          std::bind(&::OnNewDepthFrame, thermalData,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));
    _camera->Update();

    return cv::Mat(height, width, CV_32F, thermalData);
}


bool is_file_path_writable(const std::filesystem::path& file_path)
{
    const auto status = std::filesystem::status(file_path);
    const auto permissions = status.permissions();

    // Check if the file or directory is writable
    return (permissions & std::filesystem::perms::owner_write) != std::filesystem::perms::none ||
        (permissions & std::filesystem::perms::group_write) != std::filesystem::perms::none ||
        (permissions & std::filesystem::perms::others_write) != std::filesystem::perms::none;
}

void write_string_to_file(const std::filesystem::path& file_path, const std::string& file_contents)
{
    std::ofstream file_writer;
    file_writer.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file_writer.open(file_path.string(), std::ios::out | std::ios::binary);
    file_writer << file_contents;
}

void error_aware_imwrite(const std::filesystem::path& output_file_path, const cv::Mat& mat)
{
    if (const auto parent_path = output_file_path.parent_path();
        !is_directory(parent_path))
    {
        //std::cerr << "[PhotoShoot] Parent directory did not exist, create data directory with mkdir - p ~/data/photo_shoot" << std::endl;
        throw std::runtime_error("[PhotoShoot] Parent directory did not exist, create data directory with mkdir - p ~/data/photo_shoot");
    }

    if (is_regular_file(output_file_path) && !is_file_path_writable(output_file_path))
    {
        //std::cerr << "[PhotoShoot] File path:" + output_file_path.string() + "is not writable" << std::endl;
        throw std::runtime_error("[PhotoShoot] File path:" + output_file_path.string() + "is not writable");
    }

    const auto file_extension = output_file_path.extension().string();

    std::vector<uchar> buffer;
#define MB ((size_t)1024*1024)
    buffer.resize(10 * MB);

    if (const auto successfully_encoded = imencode(file_extension, mat, buffer);
        !successfully_encoded)
    {
        //std::cerr << "[PhotoShoot] Image Encoding failed" << std::endl;
        throw std::runtime_error("[PhotoShoot] Image Encoding failed");
    }

    // Write to the file
    const auto written_file_contents = std::string(buffer.begin(), buffer.end());
    if (written_file_contents.empty())
    {
        //std::cerr << "[PhotoShoot] Written image bytes were empty" << std::endl;
        throw std::runtime_error("[PhotoShoot] Written image bytes were empty");
    }

    write_string_to_file(output_file_path, written_file_contents);
}

void save_image(const std::filesystem::path& path, const cv::Mat& image) {
    try {
        error_aware_imwrite(path, image);
        std::cout << "[PhotoShoot] Image successfully written to " << path << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "[PhotoShoot] Error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[PhotoShoot] An unknown error occurred while writing image." << std::endl;
    }
}