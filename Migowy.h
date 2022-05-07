#pragma once

#include "Global.h"
#include "ui_migowy.h"

class Migowy : public QMainWindow
{
    Q_OBJECT

public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {

        ar& bDownsize;      ar& bGauss;         ar& bEq;            ar& bEqS;                           //Options 1
        ar& bSkin;          ar& bSkinHInvert;                                                           //Options 2
        ar& bApprox;        ar& bApproxSphere;                                                          //Options 3  
        ar& bWhiteBalance;  ar& bDilate;        ar& bDilateSphere;  ar& bClose; ar& bCloseSphere;       //Options 4

        ar& downsizeScale       .val; 
        ar& gaussSigma          .val;
        ar& gaussKernel         .val;
        ar& eqVClaheThreshold   .val;
        ar& eqVClaheSize        .val;
        ar& simplify            .val;
        ar& cutoff              .val;       //Options 1
                                
        ar& skinHThresholdLow   .val;
        ar& skinHThresholdHigh  .val;
        ar& skinSThresholdLow   .val;
        ar& skinSThresholdHigh  .val;
        ar& skinVThresholdLow   .val;
        ar& skinVThresholdHigh  .val;       //Options 2
                                
        ar& approxKernel        .val;
        ar& approxHThresholdLow .val;
        ar& approxHThresholdHigh.val;
        ar& approxSThresholdLow .val;
        ar& approxSThresholdHigh.val;
        ar& approxVThresholdLow .val;
        ar& approxVThresholdHigh.val;       //Options 3
                                
        ar& whiteBalanceRed     .val;
        ar& whiteBalanceGreen   .val;
        ar& whiteBalanceBlue    .val;
        ar& dilateKernel        .val;
        ar& closeKernel         .val;
        ar& closeIterations     .val;       //Options 4
    }
    Migowy(QWidget *parent = Q_NULLPTR);
    ~Migowy();

    void setupGuiAll();
protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private: 
    void setupGuiCheckBoxes();
    void setupGuiLabels();
    void setupGuiSliders();
    void setupGuiButtons();

    void clear(unsigned short slot);
    void draw(cv::Mat& data, unsigned short slot, QImage::Format format);

    void resetFilters();
    bool filter(unsigned& inIndex, unsigned outIndex, QString what);
    std::string makeDecision(std::vector<cv::Point>& contour, int decisionMode = 3, int *ret = nullptr);

    long long countHandAliasPixels(cv::Mat& oldSkin, cv::Mat& mask);
    void calibrationContourLoad();
    void channelCalibrate(cv::Mat& image, cv::Mat& mask, bool& bcamWhiteBalanceCalibratedChannel, SliderVal& channelMultiplier);
    void calibrateSkin(cv::Mat& image, cv::Mat& mask);
    void calibrate(std::vector<cv::Point>& scaledContours, cv::Mat& image);

    void skinFilter(cv::Mat& in, cv::Mat& out);
    void approxFilter(cv::Mat& in, cv::Mat& out); 

    void loadSigns();
    void createErrorMatrix();

    Ui::MigowyClass ui;

    std::vector<QLabel*> drawArea;
    std::vector<cv::Mat> drawImages;

    cv::Size size;

    //UI
    const cv::Size
        gridSize = cv::Size(6u, 3u);

    bool        bDownsize       = true, bGauss          = true, bEq             = true, bEqS    = true;                                 //Options 1
    bool        bSkin           = true, bSkinHInvert    = true;                                                                         //Options 2
    bool        bApprox         = true, bApproxSphere   = false;                                                                        //Options 3
    bool        bWhiteBalance   = true, bDilate         = true, bDilateSphere   = false, bClose = true, bCloseSphere = false;           //Options 4

    SliderVal   downsizeScale       = { 4,      1,      8   }, 
                gaussSigma          = { 16,     0,      100 },
                gaussKernel         = { 17,     1,      51  },
                eqVClaheThreshold   = { 42,     1,      80  },
                eqVClaheSize        = { 3,      0,      20  },
                simplify            = { 10,     1,      1000}, 
                cutoff              = { 60,     10,     100 };      //Options 1

    
    SliderVal   skinHThresholdLow   = { 0,      0,      179 },
                skinHThresholdHigh  = { 180,    1,      180 },
                skinSThresholdLow   = { 0,      0,      255 },
                skinSThresholdHigh  = { 256,    1,      256 },
                skinVThresholdLow   = { 0,      0,      255 },
                skinVThresholdHigh  = { 256,    1,      256 };      //Options 2

    SliderVal   approxKernel        = { 5,      1,      150 },
                approxHThresholdLow = { 0,      0,      179 },
                approxHThresholdHigh= { 0,      0,      179 },
                approxSThresholdLow = { 0,      0,      255 },
                approxSThresholdHigh= { 0,      0,      255 },
                approxVThresholdLow = { 0,      0,      255 },
                approxVThresholdHigh= { 0,      0,      255 };      //Options 3

    SliderVal   whiteBalanceRed     = { 255,    0,      767 },
                whiteBalanceGreen   = { 255,    0,      767 },
                whiteBalanceBlue    = { 255,    0,      767 },
                dilateKernel        = { 2,      1,      31  },
                closeKernel         = { 2,      1,      31  },
                closeIterations     = { 1,      1,      5   };      //Options 4


    unsigned    properIndex = 0, rawSkinIndexAdd = 0, rawSkinIndex = UINT_MAX;

    bool    bDoNotDraw = false, bBlack = false, bGreyOnce = false,
            bCalibrating = false, bcamWhiteBalanceCalibrate = false, 
            bCalibrateApproxHLow = true, bCalibrateApproxHHigh = true, bCalibrateApproxSLow = true, bCalibrateApproxSHigh = true, bCalibrateApproxVLow = true, bCalibrateApproxVHigh = true;
    //double match = 0.0, calibrateApproxRatio = 10.0;
    //int calibrateApproxKernelIterationsTry = 4;

    std::vector<cv::Point>              contour[10];
    cv::Size                            contourSize[10];
    double                              convexTrait[10];
    double                              defectTrait[10];
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Point> calibrationContour;
    cv::Size calibrationContourSize;
    std::vector<std::string> names; 
    std::string signs[10] = { "1Finger" , "2Fingers" , "3Fingers" , "4Fingers" , "5Fingers",
    "ILoveYou", "No", "Paper", "Rock", "ThumbsUp" };
    unsigned errorMatrix[10][10][3];
};
