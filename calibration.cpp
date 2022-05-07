#include "Migowy.h"

long long Migowy::countHandAliasPixels(cv::Mat &oldSkin, cv::Mat &mask)
{
    cv::Mat invertedMask = cv::Scalar(255, 255, 255) - mask;
    cv::Mat skinBGR[3], newSkin;
    cv::split(oldSkin, skinBGR);

    skinBGR[0] *= whiteBalanceBlue.val / 255.0;
    skinBGR[1] *= whiteBalanceGreen.val / 255.0;
    skinBGR[2] *= whiteBalanceRed.val / 255.0;
    cv::merge(skinBGR, 3, newSkin);

    cv::Mat newHand = cv::Mat::zeros(newSkin.size(), newSkin.type());
    newSkin.copyTo(newHand, mask);

    cv::cvtColor(newHand, newHand, cv::COLOR_BGR2HSV);
    cv::Mat skinHSV[3];
    cv::split(newHand, skinHSV);
    double minVal, maxVal;
    cv::minMaxLoc(skinHSV[0], &minVal, &maxVal, NULL, NULL, skinHSV[0]);
    cv::Mat temp = cv::Mat::zeros(oldSkin.size(), oldSkin.type());
    newSkin.copyTo(temp, invertedMask);
    cv::split(temp, skinHSV);
    temp = cv::Mat::zeros(oldSkin.size(), oldSkin.type());
    cv::threshold(skinHSV[0], temp, minVal-1, 179.0, cv::THRESH_BINARY);
    cv::threshold(skinHSV[0], skinHSV[0], maxVal-1, 179.0, cv::THRESH_BINARY_INV);
    cv::bitwise_and(skinHSV[0], temp, skinHSV[0]);
    return cv::sum(skinHSV[0])[0];
}

void Migowy::channelCalibrate(cv::Mat &image, cv::Mat &mask, bool &bcamWhiteBalanceCalibratedChannel, SliderVal &channelMultiplier)
{
    for (unsigned i = 5u; i != 0u; --i)
    {

        long long basicCount = countHandAliasPixels(image, mask);

        if (!(i = std::min(channelMultiplier.max - channelMultiplier.val, i)))
            break;

        channelMultiplier.val += i;

        calibrateSkin(image, mask);

        long long newCount = countHandAliasPixels(image, mask);
        
        if (basicCount <= newCount)
        {
            channelMultiplier.val -= i;
            calibrateSkin(image, mask);
            continue;
        }
        bcamWhiteBalanceCalibratedChannel = false;
    }
    for (unsigned i = 5u; i != 0u; --i)
    {
        long long basicCount = countHandAliasPixels(image, mask);

        if (!(i = std::min(channelMultiplier.val - channelMultiplier.min, i)))
            break;

        channelMultiplier.val += i;

        calibrateSkin(image, mask);

        long long newCount = countHandAliasPixels(image, mask);

        if (basicCount <= newCount)
        {
            channelMultiplier.val += i;
            calibrateSkin(image, mask);
            continue;
        }
        bcamWhiteBalanceCalibratedChannel = false;
    }
    ui.whiteBalanceBlueSlider   ->setValue(whiteBalanceBlue.val);
    ui.whiteBalanceGreenSlider  ->setValue(whiteBalanceGreen.val);
    ui.whiteBalanceRedSlider    ->setValue(whiteBalanceRed.val);
}

void Migowy::calibrateSkin(cv::Mat& image, cv::Mat &mask)
{
    cv::Mat skin = cv::Mat::zeros(image.size(), image.type()), skinBGR[3];
    image.copyTo(skin, mask);
    cv::split(skin, skinBGR);
    skinBGR[0] *= whiteBalanceBlue.val / 255.0;
    skinBGR[1] *= whiteBalanceGreen.val / 255.0;
    skinBGR[2] *= whiteBalanceRed.val / 255.0;
    cv::merge(skinBGR, 3, skin);
    cv::cvtColor(skin, skin, cv::COLOR_BGR2HSV);
    cv::Mat skinHSV[3];
    cv::split(skin, skinHSV);
    double minVal, maxVal;
    if (!bSkinHInvert)
    {
        cv::minMaxLoc(skinHSV[0], &minVal, &maxVal, NULL, NULL, skinHSV[0]);
        ui.skinHThresholdLowSlider->setValue(minVal);
        ui.skinHThresholdHighSlider->setValue(maxVal);
    }
    cv::minMaxLoc(skinHSV[1], &minVal, &maxVal, NULL, NULL, skinHSV[1]);
    ui.skinSThresholdLowSlider->setValue(minVal);
    ui.skinSThresholdHighSlider->setValue(maxVal);
    cv::minMaxLoc(skinHSV[2], &minVal, &maxVal, NULL, NULL, skinHSV[2]);
    ui.skinVThresholdLowSlider->setValue(minVal);
    ui.skinVThresholdHighSlider->setValue(maxVal);
}

void Migowy::calibrate(std::vector<cv::Point> &scaledContours, cv::Mat &image)
{
    if (bCalibrating)
    {
        cv::Mat mask = cv::Mat::zeros(image.size(), image.type());
        cv::fillPoly(mask, scaledContours, cv::Scalar(255, 255, 255));
        cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2, 2)));
        if (bcamWhiteBalanceCalibrate)
        {
            bool bcamWhiteBalanceCalibratedB = true, bcamWhiteBalanceCalibratedG = true, bcamWhiteBalanceCalibratedR = true;
            channelCalibrate(image, mask, bcamWhiteBalanceCalibratedB, whiteBalanceBlue);
            channelCalibrate(image, mask, bcamWhiteBalanceCalibratedG, whiteBalanceGreen);
            channelCalibrate(image, mask, bcamWhiteBalanceCalibratedR, whiteBalanceRed);

            if (bcamWhiteBalanceCalibratedB && bcamWhiteBalanceCalibratedG && bcamWhiteBalanceCalibratedR)
                bcamWhiteBalanceCalibrate = false;
        }
        else
        {
            calibrateSkin(image, mask);
            bCalibrating = false;
            ui.camDefaultButton->setText("Calibrate");
        } 
    }
}