#pragma once
#define DENOISE_SAMPLE_SIZE 10 //musi być parzysta

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/photo.hpp>
//#include <opencv2/gpu/gpu.hpp>

#include <QtWidgets/QMainWindow>
#include <QtGui/QPainter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QCheckBox >
#include <QtWidgets/QListWidget>

#include <boost\serialization\base_object.hpp>
#include <boost\serialization\export.hpp>
#include <boost\archive\binary_iarchive.hpp>
#include <boost\archive\binary_oarchive.hpp>
#include <boost\serialization\vector.hpp>
#include <boost\filesystem.hpp>

#include <csignal>
#include <vector>
#include <sstream>
#include <iomanip>

extern cv::Mat imgCv;
extern std::mutex mut;
extern std::thread *guiThread;
extern std::condition_variable used;
extern cv::VideoCapture camera;

struct SliderVal {
    unsigned val, min, max;
};
