#include "Migowy.h"
#include <iostream>

std::string Migowy::makeDecision(std::vector<cv::Point>& cont, int decisionMode, int *ret)
{
    double match = 10000.0;
    int besti = 0;
    //Pierwsza cecha
    for (int i = 0; i != 10; ++i)
    {
        double newMatch = cv::matchShapes(cont, contour[i], cv::CONTOURS_MATCH_I2, 0);
        //Alternatywa bez ostatniego momentu Hu
        /*
        cv::Moments momentsCon = cv::moments(cont), momentsBase = cv::moments(contour[i]);
        double momentsHuCon[7], momentsHuBase[7];
        cv::HuMoments(momentsCon, momentsHuCon);
        cv::HuMoments(momentsBase, momentsHuBase);
        double newMatch = 0;
        for (int i = 0; i != 6; ++i)
            newMatch += abs(momentsHuBase[i] - momentsHuCon[i]);
        */

        if (newMatch < match)
        {
            match = newMatch;
            besti = i;
        }
    }
    if (ret) *ret = besti;
    if (decisionMode == 0)
        return signs[besti];




    std::vector<cv::Point> hull;
    std::vector<int> hullI;
    std::vector<cv::Vec4i> defects;

    cv::convexHull(cont, hull);
    match = 10000.0;
    int bestConvexi = 0;
    //Druga cecha
    double convex = cv::contourArea(cont) / cv::contourArea(hull);
    for (int i = 0; i != 10; ++i)
    {
        double newMatch = abs(convexTrait[i] - convex);
        if (newMatch < match)
        {
            match = newMatch;
            bestConvexi = i;
        }
    }
    if (ret) *ret = bestConvexi;
    if (decisionMode == 1)
        return signs[bestConvexi];




    auto box = cv::minAreaRect(cont);
    double area = box.size.width * box.size.height;

    cv::convexHull(cont, hullI, false, false);

    if (hullI.size() < 2)
        return "";

    bool rising = hullI[0] > hullI[1];
    size_t i = 1;
    for (; i != hullI.size() - 1; ++i)
    {
        if (rising)
        {
            if (hullI[i - 1] < hullI[i])
                break;
        }
        else if (hullI[i - 1] > hullI[i])
            break;
    }
    if (i != hullI.size() - 1)
        return "";

    if (!hullI.empty())
        cv::convexityDefects(cont, hullI, defects);

    cv::convexityDefects(cont, hullI, defects);
    double total_arc = 0;
    for (auto defect : defects)
    {
        auto startPoint = (cont)[defect[0]];
        auto endPoint = (cont)[defect[1]];
        auto farPoint = (cont)[defect[2]];

        total_arc += cv::arcLength(std::vector<cv::Point>{startPoint, endPoint, farPoint}, true);
    }

    match = 10000.0;
    int bestDefecti = 0;
    double defect = total_arc / cv::arcLength(cont, true);
    for (int i = 0; i != 10; ++i)
    {
        double newMatch = abs(convexTrait[i] - defect);
        if (newMatch < match)
        {
            match = newMatch;
            bestDefecti = i;
        }
    }
    if (ret) *ret = bestDefecti;
    if (decisionMode == 2)
        return signs[bestDefecti];

    //Połączenie wszystkich metod
    //Ważność głosowania zależy od wyników w macierzy pomyłek
    int indexes[3] = {besti, bestConvexi, bestDefecti};
    int votesMax = 0, decision = besti;
    for (int i = 0; i != 10; ++i)
    {
        int votes = errorMatrix[besti][i][0] + errorMatrix[bestConvexi][i][1] + errorMatrix[bestDefecti][i][2];
        if ((i == besti || i == bestConvexi || i == bestDefecti) &&  votes > votesMax) {
            votesMax = votes;
            decision = i;
        }
    }
    return signs[decision];
}

void Migowy::createErrorMatrix()
{
    bDoNotDraw = true;
    for (int i = 0; i != 10; ++i)
        for (int j = 0; j != 10; ++j)
            for (int k = 0; k != 3; ++k)
                errorMatrix[i][j][k] = 0;
    
    for (int i = 0; i != 10; ++i)
    {
        for (int j = 1; j != 4; ++j)
        {
            cv::Mat img = cv::imread("test/" + signs[i] + "/" + std::to_string(j) + ".png", cv::IMREAD_GRAYSCALE);
            
            unsigned inIndex = 1, outIndex = 1;
            cv::threshold(img, img, 0, 255, cv::THRESH_BINARY);
            std::vector<std::vector<cv::Point>> cont;
            cv::findContours(img, cont, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            int ret = 0;
            std::string decision = makeDecision(cont[0], 0, &ret);
            ++errorMatrix[i][ret][0];

            decision = makeDecision(cont[0], 1, &ret);
            ++errorMatrix[i][ret][1];

            decision = makeDecision(cont[0], 2, &ret);
            ++errorMatrix[i][ret][2];
        }
    }

    std::cout << "Macierz pomylek 1. cechy: \n";
    for (int i = 0; i != 10; ++i)
        std::cout << signs[i] << " ";
    std::cout << std::endl;
    for (int i = 0; i != 10; ++i)
    {
        std::cout << signs[i] << " ";
        for (int j = 0; j != 10; ++j)
            std::cout << errorMatrix[i][j][0] << " ";
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << "Macierz pomylek 2. cechy: \n";
    for (int i = 0; i != 10; ++i)
        std::cout << signs[i] << " ";
    std::cout << std::endl;
    for (int i = 0; i != 10; ++i)
    {
        std::cout << signs[i] << " ";
        for (int j = 0; j != 10; ++j)
            std::cout << errorMatrix[i][j][1] << " ";
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << "Macierz pomylek 3. cechy: \n";
    for (int i = 0; i != 10; ++i)
        std::cout << signs[i] << " ";
    std::cout << std::endl;
    for (int i = 0; i != 10; ++i)
    {
        std::cout << signs[i] << " ";
        for (int j = 0; j != 10; ++j)
            std::cout << errorMatrix[i][j][2] << " ";
        std::cout << "\n";
    }
    bDoNotDraw = false;
}

void Migowy::loadSigns()
{
    for (int i = 0; i != 10; ++i)
    {
        auto& current = contour[i];
        cv::Mat img = cv::imread("sign/" + signs[i] + ".png", cv::IMREAD_GRAYSCALE);
        cv::threshold(img, img, 0, 255, cv::THRESH_BINARY);
        std::vector<std::vector<cv::Point>> cont;
        cv::findContours(img, cont, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        current = cont[0];
        contourSize[i] = img.size();

        auto box = cv::minAreaRect(current);
        double area = box.size.width * box.size.height;
        std::vector<cv::Point> hull;
        std::vector<int> hullI;
        std::vector<cv::Vec4i> defects;

        cv::convexHull(current, hull);
        cv::convexHull(current, hullI, false, false);

        //Sprawdzenie monotonicznośći - warunek konieczny, żeby móc obliczyć 3. cechę
        bool rising = hullI[0] > hullI[1];

        for (int j = 1; j != hullI.size() - 1; ++j)
        {
            if (rising)
            {
                if (hullI[j - 1] < hullI[j])
                    break;
            }
            else if (hullI[j - 1] > hullI[j])
                break;
        }

        convexTrait[i] = cv::contourArea(current) / cv::contourArea(hull);
        cv::convexityDefects(current, hullI, defects);
        double total_arc = 0;
        for (auto defect : defects)
        {
            auto startPoint = (current)[defect[0]];
            auto endPoint = (current)[defect[1]];
            auto farPoint = (current)[defect[2]];

            total_arc += cv::arcLength(std::vector<cv::Point>{startPoint, endPoint, farPoint}, true);
        } 
        defectTrait[i] = total_arc / cv::arcLength(current, true);
    }
}