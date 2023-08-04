#include "PhotoShoot.h"

void OnNewThermalFrame(uint16_t *_scanDest, const uint16_t *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &_format)
{
  uint16_t u;
  int size =  _width * _height * _channels;
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
    gz::rendering::ScenePtr scene = gz::rendering::sceneFromFirstRenderEngine();
    gz::rendering::VisualPtr root = scene->RootVisual();

    if (this->takePicture) {
        for (unsigned int i = 0; i < scene->NodeCount(); ++i) {
            auto camera = std::dynamic_pointer_cast<gz::rendering::ThermalCamera>(scene->NodeByIndex(i));
            if (camera != nullptr) {
                if (camera->Name() == "photo_shoot::camera_link::thermal_camera") {
                    for (std::size_t i = 0; i < this->poses.size(); i++) {
                        std::filesystem::path file("pose_" + std::to_string(i) + ".png");
                        std::filesystem::path directory(this->directory);
                        std::filesystem::path fullPath = directory / file;
                        SavePicture(camera, this->poses[i], fullPath.string());
                    }

                    this->takePicture = false;
                }
            }
        }
    }
}

void PhotoShoot::SavePicture(const gz::rendering::ThermalCameraPtr _camera,
                             const gz::math::Pose3d &_pose,
                             const std::string &_fileName)
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

    gz::common::Image image;
    gz::common::Image::ConvertToRGBImage<uint16_t>(thermalData, width, height, image);
    
    image.SavePNG(_fileName);
}