#include "Migowy.h"

void Migowy::resetFilters()
{
    bBlack = false;

    properIndex = 0;
    rawSkinIndexAdd = 0;
    rawSkinIndex = UINT_MAX;

    if (!bCalibrating)
    for (auto it = drawImages.begin(); it != drawImages.end(); ++it)
        it->release();
    contours.clear();
}

void Migowy::skinFilter(cv::Mat &in, cv::Mat &out)
{
    cv::Mat temp;
    cv::Mat HSV[3];
    cv::cvtColor(in, temp, cv::COLOR_BGR2HSV);

    cv::split(temp, HSV);

    cv::threshold(HSV[0], temp, (int)skinHThresholdLow.val - 1, skinHThresholdLow.max, cv::THRESH_BINARY);
    cv::threshold(HSV[0], HSV[0], (int)skinHThresholdHigh.val - 1, skinHThresholdHigh.max - 1, cv::THRESH_BINARY_INV);
    cv::bitwise_and(HSV[0], temp, HSV[0]);
    if (bSkinHInvert)
        HSV[0] = (int)skinHThresholdHigh.max - 1 - HSV[0];

    cv::threshold(HSV[1], temp, (int)skinSThresholdLow.val - 1, skinSThresholdLow.max, cv::THRESH_BINARY);
    cv::threshold(HSV[1], HSV[1], (int)skinSThresholdHigh.val - 1, skinSThresholdHigh.max - 1, cv::THRESH_BINARY_INV);
    cv::bitwise_and(HSV[1], temp, HSV[1]);
    cv::bitwise_and(HSV[0], HSV[1], HSV[1]);

    cv::threshold(HSV[2], temp, (int)skinVThresholdLow.val - 1, skinVThresholdLow.max, cv::THRESH_BINARY);
    cv::threshold(HSV[2], HSV[2], (int)skinVThresholdHigh.val - 1, skinVThresholdHigh.max - 1, cv::THRESH_BINARY_INV);
    cv::bitwise_and(HSV[2], temp, HSV[2]);
    cv::bitwise_and(HSV[1], HSV[2], HSV[2]);

    out.setTo(cv::Scalar(0, 0, 0));
    in.copyTo(out, HSV[2]);
}

void Migowy::approxFilter(cv::Mat& in, cv::Mat& out)
{
    cv::Mat temp, dilated;
    cv::Mat HSV[3];

    cv::split(in, HSV);
    cv::bitwise_or(HSV[0], HSV[1], HSV[1]);
    cv::bitwise_or(HSV[1], HSV[2], HSV[2]);
    cv::morphologyEx(HSV[2], dilated, cv::MORPH_DILATE, cv::getStructuringElement(bApproxSphere ? cv::MORPH_ELLIPSE : cv::MORPH_RECT, cv::Size(approxKernel.val, approxKernel.val))); //dilate on separate channels maybe?
    drawImages[rawSkinIndex].copyTo(out, dilated);

    cv::cvtColor(out, temp, cv::COLOR_BGR2HSV);
    cv::split(temp, HSV);

    cv::threshold(HSV[0], temp, std::max(-1, (int)skinHThresholdLow.val - 1 - (bSkinHInvert ? -(int)approxHThresholdLow.val : (int)approxHThresholdLow.val)), skinHThresholdLow.max, cv::THRESH_BINARY);
    cv::threshold(HSV[0], HSV[0], std::min(skinHThresholdHigh.max - 1, skinHThresholdHigh.val - 1 + (bSkinHInvert ? -(int)approxHThresholdHigh.val : (int)approxHThresholdHigh.val)), skinHThresholdHigh.max - 1, cv::THRESH_BINARY_INV);
    cv::bitwise_and(HSV[0], temp, HSV[0]);
    if (bSkinHInvert)
    {
        HSV[0] = (int)skinHThresholdHigh.max - 1 - HSV[0];
        cv::bitwise_and(HSV[0], dilated, HSV[0]);
    }

    cv::threshold(HSV[1], temp, std::max(-1, (int)skinSThresholdLow.val - 1 - (int)approxSThresholdLow.val), skinSThresholdLow.max, cv::THRESH_BINARY);
    cv::threshold(HSV[1], HSV[1], std::min(skinSThresholdHigh.max - 1, skinSThresholdHigh.val - 1 + approxSThresholdHigh.val), skinSThresholdHigh.max - 1, cv::THRESH_BINARY_INV);
    cv::bitwise_and(HSV[1], temp, HSV[1]);
    cv::bitwise_and(HSV[0], HSV[1], HSV[1]);

    cv::threshold(HSV[2], temp, std::max(-1, (int)skinVThresholdLow.val - 1 - (int)approxVThresholdLow.val), skinVThresholdLow.max, cv::THRESH_BINARY);
    cv::threshold(HSV[2], HSV[2], std::min(skinVThresholdHigh.max - 1, skinVThresholdHigh.val - 1 + approxVThresholdHigh.val), skinVThresholdHigh.max - 1, cv::THRESH_BINARY_INV);
    cv::bitwise_and(HSV[2], temp, HSV[2]);
    cv::bitwise_and(HSV[1], HSV[2], HSV[2]);
    cv::bitwise_and(dilated, HSV[2], HSV[2]);
    out.setTo(cv::Scalar(0, 0, 0));

    drawImages[rawSkinIndex].copyTo(out, HSV[2]);
    cv::cvtColor(out, temp, cv::COLOR_BGR2HSV);
}


bool Migowy::filter(unsigned& inIndex, unsigned outIndex, QString what)
{
    cv::Mat& in = drawImages[inIndex], &out = drawImages[outIndex];

    if (!what.compare("Original"))
    {
        //Zbiera obraz, chyba że program jest akurat w fazie kalibracji
        if (!bCalibrating)
        {
            mut.lock();
            imgCv.copyTo(out);
            mut.unlock();
        }

        if (out.empty())
            return false;
    }
    else if (in.empty())
        return false;
    else if (!what.compare("White balance"))
    {
        if (bCalibrating)
            return false;
        cv::Mat BGR[3];
        cv::split(in, BGR);

        BGR[0] *= whiteBalanceBlue.val / 255.0;
        BGR[1] *= whiteBalanceGreen.val / 255.0;
        BGR[2] *= whiteBalanceRed.val / 255.0;
        cv::merge(BGR, 3, out);
    }
    else if (!what.compare("Downsize"))
        cv::resize(in, out, cv::Size(in.cols / downsizeScale.val, in.rows / downsizeScale.val));
    else if (!what.compare("Gauss"))
        cv::GaussianBlur(in, out, cv::Size(gaussKernel.val, gaussKernel.val), gaussSigma.val / 10.0);
    else if (!what.compare("Equalize"))
    {
        if (bBlack) return false;
        cv::cvtColor(in, out, cv::COLOR_BGR2HSV);

        cv::Mat HSV[3];
        cv::split(out, HSV);

        if (bEqS) cv::equalizeHist(HSV[1], HSV[1]);
        auto clahe = cv::createCLAHE(eqVClaheThreshold.val / 10.0, cv::Size(eqVClaheSize.val, eqVClaheSize.val));
        if (eqVClaheSize.val) clahe->apply(HSV[2], HSV[2]);
        cv::merge(HSV, 3, out);
        cv::cvtColor(out, out, cv::COLOR_HSV2BGR);
    }
    else if (!what.compare("Skin"))
    {
        if (bBlack)
            return false;
        skinFilter(in, out);
        rawSkinIndex = inIndex;
    }
    else if (!what.compare("Approx"))
    {
        //if (rawSkinIndex + rawSkinIndexAdd + 1 != inIndex || bBlack)
            //return false;
        approxFilter(in, out);
    }
    else if (!what.compare("H"))
    {
        if (bBlack) return false;
        if (outIndex < rawSkinIndex)
            ++rawSkinIndexAdd;
        cv::cvtColor(in, out, cv::COLOR_BGR2HSV);

        cv::Mat HSV[3];
        cv::split(out, HSV);
        HSV[1].setTo(255);
        HSV[2].setTo(255);
        cv::merge(HSV, 3, out);

        cv::cvtColor(out, out, cv::COLOR_HSV2BGR);

        return true;        //Przedwczesny return wymusza użycie poprzedniego obrazu do następnego filtru. Normalnie obrazu są przetwarzane jeden po drugim.
                            //Innymi słowy: normalnie [out] staję się [in] następnego filtra. Przedwczesne wyjście sprawia, że [in] pozostaje [in] następnego filtra.
    }
    else if (!what.compare("S"))
    {
        if (outIndex < rawSkinIndex)
            ++rawSkinIndexAdd;
        if (bBlack) return false;
        cv::cvtColor(in, out, cv::COLOR_BGR2HSV);

        cv::Mat HSV[3];
        cv::split(out, HSV);
        out = HSV[1];
        bGreyOnce = true;
        //Draw() wyświetli monochromatyczny obraz, a potem z powrotem będzie chciał wyświetlać BGR
        return true;
    }
    else if (!what.compare("V"))
    {
        if (outIndex < rawSkinIndex)
            ++rawSkinIndexAdd;
        if (bBlack) return false;
        cv::cvtColor(in, out, cv::COLOR_BGR2HSV);

        cv::Mat HSV[3];
        cv::split(out, HSV);
        out = HSV[2];
        bGreyOnce = true;
        return true;
    }
    else if (!what.compare("Grey"))
    {
        bBlack = true;
        cv::cvtColor(in, out, cv::COLOR_BGR2GRAY);
    }
    else if (!what.compare("Contours"))
    {
        std::vector<cv::Vec4i> hierarchy;
        cv::threshold(in, out, 0, 255, cv::THRESH_BINARY);
        cv::findContours(out, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        int best = 0;
        double best_value = 1;
        double res;
        out.setTo(0);
        if (contours.empty())
        {
            inIndex = outIndex;
            return true;
        }
        double avgL = 0.0;
        double avgP = 0.0;
        for (auto it = contours.begin(); it != contours.end(); ++it)
        {
            auto box = cv::boundingRect(*it);
            avgL += cv::arcLength(*it, false);
            avgP += box.area();
            //cv::drawContours(out, contours, -1, cv::Scalar(255, 125, 255), 1, cv::LINE_8, hierarchy, 0);
        }
        avgL /= contours.size();
        avgP /= contours.size();
        for (unsigned i = 0; i != contours.size(); )
        {
            auto box = cv::boundingRect(contours[i]);
            if (box.area() < cutoff.val/100.0 * avgP || cv::arcLength(contours[i], false) < cutoff.val / 100.0 * avgL)
                contours.erase(contours.begin() + i);
            else
            {
                cv::approxPolyDP(contours[i], contours[i], simplify.val/100000.0 * cv::arcLength(contours[i], true), true);
                ++i;
            }
        }
        cv::drawContours(out, contours, -1, cv::Scalar(255, 255, 255));
    }
    else if (!what.compare("Sign"))
    {
        if (bCalibrating || contours.empty())
            return false;
        bBlack = false;
        unsigned i = 0;
        for (; i != drawImages.size(); ++i)
            if (!drawImages[i].empty()) break;

        drawImages[i].copyTo(out);
        
        double scale = static_cast<double>(out.size().width) / in.size().width;
       
        for (auto it = contours.begin(); it != contours.end(); ++it)
            for (auto point = it->begin(); point != it->end(); ++point)
                *point *= scale;

        cv::drawContours(out, contours, -1, cv::Scalar(255, 250, 155), 1);

        for (auto it = contours.begin(); it != contours.end(); ++it)
        {
            auto box = cv::minAreaRect(*it);
            cv::Point2f rect_points[4]; 
            box.points(rect_points);
            for (int j = 0; j < 4; j++)
                line(out, rect_points[j], rect_points[(j + 1) == 4 ? 0 : (j + 1)], cv::Scalar(0, 0, 255), 1);
            
            std::vector<cv::Point> hull;
            std::vector<int> hullI;
            std::vector<cv::Vec4i> defects;
            cv::convexHull(*it, hull);
            cv::convexHull(*it, hullI, false, false);
            inIndex = outIndex;

            if (hullI.size() < 2)
                return true;

            bool rising = hullI[0] > hullI[1];
            size_t i = 1;
            for ( ;i != hullI.size(); ++i)
            {
                if (rising)
                {
                    if (hullI[i - 1] < hullI[i])
                        break;
                }
                else if (hullI[i - 1] > hullI[i])
                    break;
            }
            if (i != hullI.size())
                return true;

            cv::polylines(out, hull, true, cv::Scalar(100, 255, 255), 1);
            for (auto point : hull)
            cv::circle(out, point, 4, cv::Scalar(80, 225, 225), -2);
            
            if (!hullI.empty())
                cv::convexityDefects(*it, hullI, defects);
            
            double arcLength = 0.0;
            for (auto defect : defects)
            {
                auto startPoint = (*it)[defect[0]];
                auto endPoint = (*it)[defect[1]];
                auto farPoint = (*it)[defect[2]];

                
                cv::line(out, startPoint, farPoint, cv::Scalar(0, 255, 0), 1);
                cv::line(out, endPoint, farPoint, cv::Scalar(0, 255, 0), 1);
                cv::circle(out, farPoint, 4, cv::Scalar(0, 200, 0), -2);
            }

            std::string match = makeDecision(*it);
            box.points(rect_points);
            cv::Point cornerTopLeft = rect_points[0];
            for (int i = 1; i != 4; ++i)
            {
                if (cornerTopLeft.y > rect_points[i].y)
                    cornerTopLeft = rect_points[i];
                else if ((cornerTopLeft.y - rect_points[i].y) > (rect_points[i].x - cornerTopLeft.x))
                    cornerTopLeft = rect_points[i];
            }

            cv::putText(out, match, cornerTopLeft, cv::FONT_HERSHEY_DUPLEX, 2.2, cv::Scalar(255, 255, 255), 1, false);
        }
    }
    else if (!what.compare("Calibrate"))
    {
        if (outIndex < rawSkinIndex || contours.empty()) return false;
        bBlack = false;

        drawImages[rawSkinIndex].copyTo(out);
        double scale = static_cast<double>(out.size().width) / calibrationContourSize.width;
        std::vector<cv::Point> scaledContours = calibrationContour;

        for (auto point = scaledContours.begin(); point != scaledContours.end(); ++point)
            *point *= scale;

        calibrate(scaledContours, out);
        std::vector<std::vector<cv::Point>> drawContours;
        drawContours.push_back(scaledContours);
        cv::drawContours(out, drawContours, -1, cv::Scalar(255, 250, 155), 2);
    }
    else if (!what.compare("Dilate"))
    {
        cv::Mat dilated;
        cv::morphologyEx(in, dilated, cv::MORPH_DILATE, cv::getStructuringElement(bDilateSphere ? cv::MORPH_ELLIPSE : cv::MORPH_RECT, cv::Size(dilateKernel.val, dilateKernel.val)));
        if (rawSkinIndex != UINT_MAX)
        {
            drawImages[rawSkinIndex].copyTo(out, dilated);
            if (bBlack)  
                cv::cvtColor(out, out, cv::COLOR_BGR2GRAY);
        }
        else 
            out = dilated;
    
    }
    else if (!what.compare("Close"))
    {
        cv::Mat closed;
        cv::morphologyEx(in, closed, cv::MORPH_CLOSE, cv::getStructuringElement(bCloseSphere ? cv::MORPH_ELLIPSE : cv::MORPH_RECT, cv::Size(closeKernel.val, closeKernel.val)), cv::Point(-1, -1), closeIterations.val);
        if (rawSkinIndex != UINT_MAX)
        {
            drawImages[rawSkinIndex].copyTo(out, closed);
            if (bBlack)
                cv::cvtColor(out, out, cv::COLOR_BGR2GRAY);
        }
        else
            out = closed;
    }
 
    else
        return false;

    inIndex = outIndex;
    return true;
}