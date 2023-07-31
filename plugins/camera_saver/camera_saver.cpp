#include <iostream>

#include <gz/sim/Model.hh>
#include <gz/sim/Util.hh>
#include <gz/msgs.hh>
#include <gz/transport.hh>
#include <gz/plugin/Register.hh>
#include <gz/sim/System.hh>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

class CameraSaver
      : public gz::sim::System,
        public gz::sim::ISystemConfigure
{
    public: virtual void Configure(const gz::sim::Entity &_entity,
                                   const std::shared_ptr<const sdf::Element> &_sdf,
                                   gz::sim::EntityComponentManager &_ecm,
                                   gz::sim::EventManager &_eventMgr) override
    {
        // Grab the topic from the config
        if (_sdf->HasElement("topic")) {
            this->topic = _sdf->Get<std::string>("topic");
        } else {
            std::cerr << "Topic is missing in the configuration!" << std::endl;
        }

        // Try to subscribe to the topic
        if (!this->node.Subscribe(this->topic, &CameraSaver::OnNewImage, this)) {
            std::cerr << "Error subscribing to topic [" << this->topic << "]" << std::endl;
        }

        // Grab the output path from the config
        if (_sdf->HasElement("directory")) {
            this->directory = _sdf->Get<std::string>("directory");
        } else {
            std::cerr << "Directory is missing in the configuration!" << std::endl;
        }
    }

    public: void OnNewImage(const gz::msgs::Image &imageMSGS)
    {
        int width = (int) imageMSGS.width();
        int height = (int) imageMSGS.height();
  
        char* data = new char[imageMSGS.data().length() + 1];
        memcpy(data, imageMSGS.data().c_str(),imageMSGS.data().length());

        cv::Mat image(height, width, CV_8UC3, data);
        cv::cvtColor(image, image, 4);  // convert from rgb to bgr

        char fileName[100];
        snprintf(fileName, 100, "%s/image_%04d.jpg", this->directory.c_str(), this->imageCounter++);

        cv::imwrite(fileName, image); 

        delete[] data;
    }

    // The transport node that subscribes to the image messages
    private: int imageCounter = 0;
    private: gz::transport::Node node;
    private: std::string topic = "";
    private: std::string directory = "";
};

GZ_ADD_PLUGIN(
    CameraSaver,
    gz::sim::System,
    CameraSaver::ISystemConfigure
)