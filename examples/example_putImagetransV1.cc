#include <UnitreeCameraSDK.hpp>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <opencv2/opencv.hpp>


cv::Mat parseOpenCVMatrix(const YAML::Node& node) {
    int rows = node["rows"].as<int>();
    int cols = node["cols"].as<int>();
    std::string dt = node["dt"].as<std::string>();
    std::vector<double> data = node["data"].as<std::vector<double>>();

    cv::Mat mat(rows, cols, CV_64F);
    for (int i = 0; i < rows * cols; ++i) {
        mat.at<double>(i / cols, i % cols) = data[i];
    }

    return mat;
}


int main(int argc, char* argv[])
{
    std::chrono::microseconds t;
    cv::Mat frame;
    cv::Mat depth;
    cv::Mat left, right, feim;
    int transMode = 0;

    std::string configFn = "trans_rect_config.yaml";
    if (argc >= 2) {
        configFn = argv[1];
    }
    std::cout << "Config FN: " << configFn << std::endl;

    // Load the YAML file
    YAML::Node config = YAML::LoadFile(configFn);

    if (config["Transmode"]) {
        cv::Mat transModeMat = parseOpenCVMatrix(config["Transmode"]);
        transMode = transModeMat.at<double>(0,0);
        std::cout << "TransMode: " << transMode << std::endl;
    }

    UnitreeCamera cam(configFn); //Read the settings.
    if (!cam.isOpened())
    {
        exit(EXIT_FAILURE);
    }
    cam.startCapture(true,false); ///< disable share memory sharing and able image h264 encoding

    if (transMode == 4) 
    {
        cam.startStereoCompute(); //start disparity computing
    }
    
    usleep(500000);
    while(cam.isOpened())
    {
        // Original fish Eye image. (Transmode = 0/1, Depthmode = 1)
        if((transMode == 0 || transMode == 1) && !cam.getRawFrame(frame, t)) 
        {
            usleep(1000);
            continue;
        }
        // Rectify image. (Transmode = 2/3, Depthmode = 1)
        else if((transMode == 2 || transMode == 3) && !cam.getRectStereoFrame(left, right))
        {
            usleep(1000);
            continue;
        }
        // Depth image. (Transmode = 4, Depthmode = 2)
        else if(transMode == 4 && !cam.getDepthFrame(depth, true, t))
        {
            usleep(1000);
            continue;
        }

        char key = cv::waitKey(10);
        if(key == 27) // press ESC key
           break;
    }

    cam.stopStereoCompute(); //stop disparity computing 
    cam.stopCapture(); //stop camera capturing

    return 0;
}
