#include "global.h"

cv::Mat imgCv;
std::mutex mut;
std::thread *guiThread = nullptr;
cv::VideoCapture camera;