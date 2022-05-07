#include "migowy.h"

void Migowy::setupGuiAll()
{
    std::vector<QString> properNames = { "Original", ui.whiteBalanceCheck->text(),
        ui.downsizeCheck->text(), ui.gaussCheck->text(), ui.eqCheck->text(),
        "H", "S", "V", ui.skinCheck->text(), ui.approxCheck->text(), "Grey",
        ui.dilateCheck->text(), ui.closeCheck->text(), "Contours", "Sign", "Calibrate" };

    //Upewniamy się, że lista z filtrami posiada prawidłowe nazwy, a jak nie ma wszystkich, to dodajemy je w domyślne miejsce
    ui.listWidget->clear();
    for (unsigned u = 0; u != names.size(); )
    {
        bool found = false;
        for (auto pit = properNames.begin(); pit != properNames.end(); ++pit)
            if (!pit->compare(names[u].c_str()))
                found = true;
        if (!found)
            names.erase(names.begin() + u);
        else        ++u;
    }
    for (unsigned i = 0u; i != properNames.size(); ++i)
    {
        bool found = false;
        std::string str = properNames[i].toStdString();
        for (auto it = names.begin(); it != names.end(); ++it)
            if (!it->compare(str))
                found = true;
        if (!found) names.insert(names.begin() + i, str);
    }
    for (auto it = names.begin(); it != names.end(); ++it)
        ui.listWidget->addItem(QString(it->c_str()));

    setupGuiCheckBoxes();
    setupGuiLabels();
    setupGuiSliders();
    setupGuiButtons();
}

void Migowy::setupGuiCheckBoxes()
{
    ui.downsizeCheck        ->setup(&bDownsize,         ui.listWidget,  {           ui.downsizeScaleText,       ui.downsizeScaleSlider          });

    ui.gaussCheck           ->setup(&bGauss,            ui.listWidget,  {           ui.gaussSigmaText,          ui.gaussSigmaSlider,
                                                                                    ui.gaussKernelText,         ui.gaussKernelSlider            });

    ui.eqCheck              ->setup(&bEq,               ui.listWidget,  {           ui.eqSCheck,
                                                                                    ui.eqVClaheThresholdText,   ui.eqVClaheThresholdSlider,
                                                                                    ui.eqVClaheKernelText,      ui.eqVClaheKernelSlider         });

    ui.eqSCheck             ->setup(&bEqS,              ui.listWidget,  {                                                                       });

    ui.skinCheck            ->setup(&bSkin,             ui.listWidget,  {           ui.skinHThresholdLowText,   ui.skinHThresholdLowSlider,
                                                                                    ui.skinHThresholdHighText,  ui.skinHThresholdHighSlider,
                                                                                    ui.skinSThresholdLowText,   ui.skinSThresholdLowSlider,
                                                                                    ui.skinSThresholdHighText,  ui.skinSThresholdHighSlider,
                                                                                    ui.skinVThresholdLowText,   ui.skinVThresholdLowSlider,
                                                                                    ui.skinVThresholdHighText,  ui.skinVThresholdHighSlider     });

    ui.skinHInvertCheck     ->setup(&bSkinHInvert,      ui.listWidget,  {                                                                       }, 
        false,
        [this](bool val)->bool 
        {
            if (!val)
            {
                ui.approxHThresholdLowSlider->setMaximum(std::max((int)approxHThresholdLow.min, (int)approxHThresholdLow.max - std::max(0, ui.skinHThresholdHighSlider->value() - ui.skinHThresholdLowSlider->value())));
                ui.approxHThresholdHighSlider->setMaximum(std::max((int)approxHThresholdHigh.min, (int)approxHThresholdHigh.max - std::max(0, ui.skinHThresholdHighSlider->value() - ui.skinHThresholdLowSlider->value())));
                ui.approxHThresholdLowSlider->update();
                ui.approxHThresholdHighSlider->update();
            }
            else
            {
                ui.approxHThresholdLowSlider->setMaximum(std::max((int)approxHThresholdLow.min, ui.skinHThresholdHighSlider->value() - ui.skinHThresholdLowSlider->value()));
                ui.approxHThresholdHighSlider->setMaximum(std::max((int)approxHThresholdHigh.min, ui.skinHThresholdHighSlider->value() - ui.skinHThresholdLowSlider->value()));
                unsigned bound = !bSkinHInvert ? ui.skinHThresholdLowSlider->value() : (ui.skinHThresholdLowSlider->maximum() - ui.skinHThresholdLowSlider->value());
                if (ui.approxHThresholdHighSlider->value() > bound)
                    ui.approxHThresholdHighSlider->setValue(bound);
                bound = bSkinHInvert ? ui.skinHThresholdHighSlider->value() : (ui.skinHThresholdHighSlider->maximum() - ui.skinHThresholdHighSlider->value());
                if (ui.approxHThresholdLowSlider->value() > bound)
                    ui.approxHThresholdLowSlider->setValue(bound);
            }
            return val;
        });

    ui.approxCheck->setup(&bApprox,                     ui.listWidget,  {           ui.approxSphereCheck,
                                                                                    ui.approxKernelText,            ui.approxKernelSlider,
                                                                                    ui.approxHThresholdLowText,     ui.approxHThresholdLowSlider,
                                                                                    ui.approxHThresholdHighText,    ui.approxHThresholdHighSlider,
                                                                                    ui.approxSThresholdLowText,     ui.approxSThresholdLowSlider,
                                                                                    ui.approxSThresholdHighText,    ui.approxSThresholdHighSlider,
                                                                                    ui.approxVThresholdLowText,     ui.approxVThresholdLowSlider,
                                                                                    ui.approxVThresholdHighText,    ui.approxVThresholdHighSlider   });

    ui.approxSphereCheck    ->setup(&bApproxSphere,     ui.listWidget,  {                                                                           });

    ui.whiteBalanceCheck    ->setup(&bWhiteBalance,     ui.listWidget,  {           ui.whiteBalanceRedText,         ui.whiteBalanceRedSlider,
                                                                                    ui.whiteBalanceGreenText,       ui.whiteBalanceGreenSlider,
                                                                                    ui.whiteBalanceBlueText,        ui.whiteBalanceBlueSlider       });

    ui.dilateCheck          ->setup(&bDilate,           ui.listWidget,  {           ui.dilateSphereCheck,
                                                                                    ui.dilateKernelText,            ui.dilateKernelSlider           });

    ui.dilateSphereCheck    ->setup(&bDilateSphere,     ui.listWidget,  {                                                                           });

    ui.closeCheck           ->setup(&bClose,            ui.listWidget,  {           ui.closeSphereCheck,
                                                                                    ui.closeKernelText,             ui.closeKernelSlider,
                                                                                    ui.closeIterationsText,         ui.closeIterationsSlider        });

    ui.closeSphereCheck     ->setup(&bCloseSphere,      ui.listWidget,  {                                                                           });
}

void Migowy::setupGuiLabels()
{
    ui.downsizeScaleText        ->setup("Scale: ");
    ui.gaussSigmaText           ->setup("Sigma: ", 1);
    ui.gaussKernelText          ->setup("Kernel: ");
    ui.eqVClaheThresholdText    ->setup("V CLAHE threshold: ");
    ui.eqVClaheKernelText       ->setup("V CLAHE kernel: ");
    ui.simplifyText             ->setup("Epsilon: ", 5);
    ui.cutoffText               ->setup("Cutoff: ", 2);

    ui.skinHThresholdLowText    ->setup("H threshold low: ");
    ui.skinHThresholdHighText   ->setup("H threshold high: ");
    ui.skinSThresholdLowText    ->setup("S threshold low: ");
    ui.skinSThresholdHighText   ->setup("S threshold high: ");
    ui.skinVThresholdLowText    ->setup("V threshold low: ");
    ui.skinVThresholdHighText   ->setup("V threshold high: ");

    ui.approxKernelText         ->setup("Kernel: ");
    ui.approxHThresholdLowText  ->setup("H threshold low: ");
    ui.approxHThresholdHighText ->setup("H threshold high: ");
    ui.approxSThresholdLowText  ->setup("S threshold low: ");
    ui.approxSThresholdHighText ->setup("S threshold high: ");
    ui.approxVThresholdLowText  ->setup("V threshold low: ");
    ui.approxVThresholdHighText ->setup("V threshold high: ");

    ui.whiteBalanceRedText      ->setup("Red: ");
    ui.whiteBalanceGreenText    ->setup("Green: ");
    ui.whiteBalanceBlueText     ->setup("Blue: ");
    ui.dilateKernelText         ->setup("Kernel: ");
    ui.closeKernelText          ->setup("Kernel: ");
    ui.closeIterationsText      ->setup("Iterations: ");
}

void Migowy::setupGuiSliders() 
{
    ui.downsizeScaleSlider      ->setup(ui.downsizeScaleText,       &downsizeScale);
    ui.gaussSigmaSlider         ->setup(ui.gaussSigmaText,          &gaussSigma);
    ui.gaussKernelSlider        ->setup(ui.gaussKernelText,         &gaussKernel,    [](unsigned val)->unsigned {if (!(val % 2)) ++val; return val; });
    ui.simplifySlider           ->setup(ui.simplifyText,            &simplify);
    ui.cutoffSlider             ->setup(ui.cutoffText,              &cutoff);



    ui.approxKernelSlider       ->setup(ui.approxKernelText,        &approxKernel);

    auto setHApprox = [this](unsigned val)->unsigned 
    {
        unsigned maxVal = std::max((int)approxHThresholdLow.min, !bSkinHInvert ? 
            ((int)approxHThresholdLow.max - std::max(0, ui.skinHThresholdHighSlider->value() - ui.skinHThresholdLowSlider->value())) 
            : (ui.skinHThresholdHighSlider->value() - ui.skinHThresholdLowSlider->value()));
        ui.approxHThresholdLowSlider->setMaximum(maxVal);
        ui.approxHThresholdHighSlider->setMaximum(maxVal);
        ui.approxHThresholdHighSlider->refresh();
        return val; 
    };

    ui.skinHThresholdLowSlider  ->setup(ui.skinHThresholdLowText,  &skinHThresholdLow,  setHApprox);
    ui.skinHThresholdHighSlider ->setup(ui.skinHThresholdHighText, &skinHThresholdHigh, setHApprox);
   
    auto setSApprox = [this](unsigned val)->unsigned
    {
        unsigned maxVal = std::max(0, ui.skinSThresholdHighSlider->maximum() - ui.skinSThresholdHighSlider->value() + ui.skinSThresholdLowSlider->value());
        ui.approxSThresholdLowSlider->setMaximum(maxVal);
        ui.approxSThresholdHighSlider->setMaximum(maxVal);
        ui.approxSThresholdHighSlider->refresh();
        return val; 
    };
    ui.skinSThresholdLowSlider  ->setup(ui.skinSThresholdLowText,  &skinSThresholdLow,  setSApprox);
    ui.skinSThresholdHighSlider ->setup(ui.skinSThresholdHighText, &skinSThresholdHigh, setSApprox);
    
    auto setVApprox = [this](unsigned val)->unsigned 
    {
        unsigned maxVal = std::max((int)approxVThresholdLow.min, ui.skinVThresholdHighSlider->maximum() - ui.skinVThresholdHighSlider->value() + ui.skinVThresholdLowSlider->value());
        ui.approxVThresholdLowSlider->setMaximum(maxVal);
        ui.approxVThresholdHighSlider->setMaximum(maxVal);
        ui.approxVThresholdHighSlider->refresh();
        return val; 
    };
    ui.skinVThresholdLowSlider  ->setup(ui.skinVThresholdLowText,  &skinVThresholdLow,  setVApprox);
    ui.skinVThresholdHighSlider ->setup(ui.skinVThresholdHighText, &skinVThresholdHigh, setVApprox);

    unsigned hLow = approxHThresholdLow.val;
    ui.approxHThresholdLowSlider->setup(ui.approxHThresholdLowText, &approxHThresholdLow, 
        [this](unsigned val)->unsigned 
        {
            unsigned bound = !bSkinHInvert ? ui.skinHThresholdLowSlider->value() : (ui.skinHThresholdLowSlider->maximum() - ui.skinHThresholdLowSlider->value());
            if (val > bound) return bound;
            ui.approxHThresholdHighSlider->setValue(std::min(ui.approxHThresholdHighSlider->maximum() - (int)val, ui.approxHThresholdHighSlider->value()));
            return val;
        });

    ui.approxHThresholdHighSlider->setup(ui.approxHThresholdHighText, &approxHThresholdHigh, 
        [this](unsigned val)->unsigned 
        {
            unsigned bound = bSkinHInvert ? ui.skinHThresholdHighSlider->value() : (ui.skinHThresholdHighSlider->maximum() - ui.skinHThresholdHighSlider->value());
            if (val > bound) return bound;
            ui.approxHThresholdLowSlider->setValue(std::min(ui.approxHThresholdLowSlider->maximum() - (int)val, ui.approxHThresholdLowSlider->value()));
            return val;
        });

    ui.approxSThresholdLowSlider->setup(ui.approxSThresholdLowText, &approxSThresholdLow, 
        [this](unsigned val)->unsigned
        {
            unsigned bound = ui.skinSThresholdLowSlider->value();
            if (val > bound) return bound;
            ui.approxSThresholdHighSlider->setValue(std::min(ui.approxSThresholdHighSlider->maximum() - (int)val, ui.approxSThresholdHighSlider->value()));
            return val;
        });

    ui.approxSThresholdHighSlider->setup(ui.approxSThresholdHighText, &approxSThresholdHigh, 
        [this](unsigned val)->unsigned 
        {
            unsigned bound = ui.skinSThresholdHighSlider->maximum() - ui.skinSThresholdHighSlider->value();
            if (val > bound) return bound;
            ui.approxSThresholdLowSlider->setValue(std::min(ui.approxSThresholdLowSlider->maximum() - (int)val, ui.approxSThresholdLowSlider->value()));
            return val;
        });

    ui.approxVThresholdLowSlider->setup(ui.approxVThresholdLowText, &approxVThresholdLow, 
        [this](unsigned val)->unsigned 
        {
            unsigned bound = ui.skinVThresholdLowSlider->value();
            if (val > bound) return bound;
            ui.approxVThresholdHighSlider->setValue(std::min(ui.approxVThresholdHighSlider->maximum() - (int)val, ui.approxVThresholdHighSlider->value()));
            return val;
        });

    ui.approxVThresholdHighSlider->setup(ui.approxVThresholdHighText, &approxVThresholdHigh, 
        [this](unsigned val)->unsigned 
        {
            unsigned bound = ui.skinVThresholdHighSlider->maximum() - ui.skinVThresholdHighSlider->value();
            if (val > bound) return bound;
            ui.approxVThresholdLowSlider->setValue(std::min(ui.approxVThresholdLowSlider->maximum() - (int)val, ui.approxVThresholdLowSlider->value()));
            return val;
        });

    ui.skinHThresholdLowSlider  ->refresh();
    ui.skinHThresholdHighSlider ->refresh();
    ui.skinSThresholdLowSlider  ->refresh();
    ui.skinSThresholdHighSlider ->refresh();
    ui.skinVThresholdLowSlider  ->refresh();
    ui.skinVThresholdHighSlider ->refresh();
    ui.eqVClaheThresholdSlider  ->setup(ui.eqVClaheThresholdText,  &eqVClaheThreshold);
    ui.eqVClaheKernelSlider     ->setup(ui.eqVClaheKernelText,     &eqVClaheSize);


    ui.whiteBalanceRedSlider    ->setup(ui.whiteBalanceRedText,    &whiteBalanceRed);
    ui.whiteBalanceGreenSlider  ->setup(ui.whiteBalanceGreenText,  &whiteBalanceGreen);
    ui.whiteBalanceBlueSlider   ->setup(ui.whiteBalanceBlueText,   &whiteBalanceBlue);
    ui.dilateKernelSlider       ->setup(ui.dilateKernelText,       &dilateKernel);
    ui.closeKernelSlider        ->setup(ui.closeKernelText,        &closeKernel);
    ui.closeIterationsSlider    ->setup(ui.closeIterationsText,    &closeIterations);   
}

void Migowy::setupGuiButtons()
{
    connect(ui.saveOptionsButton, &QPushButton::clicked, this, 
        [this]()
        {
            names.clear();
            std::ofstream save("options.dat", std::ios::trunc);
            boost::archive::binary_oarchive oa(save);

            for (int i = 0; i < ui.listWidget->count(); ++i)
                names.push_back(ui.listWidget->item(i)->text().toStdString());

            oa << *this;
        });

    connect(ui.saveImageButton, &QPushButton::clicked, this, 
        [this]()
        {
            for (int i = ui.listWidget->count() - 1; i != -1; --i)
            {
                auto item = ui.listWidget->item(i);
                if (item->textColor().red() != 255 && ui.listWidget->item(i)->flags() & Qt::ItemFlag::ItemIsEnabled)
                {
                    cv::Mat img = drawImages[i];
                    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Save Image"), "",
                        tr("Image PNG (*.png)"));

                    if (!fileName.isEmpty())
                        cv::imwrite(fileName.toStdString(), img);
                    break;
                }
            }
        });

    connect(ui.camDefaultButton, &QPushButton::clicked, this, 
        [this]()
        {
            bCalibrating = !bCalibrating;
            if (bCalibrating)
            {
                ui.skinHThresholdLowSlider->setValue(0);
                ui.skinHThresholdHighSlider->setValue(180);
                ui.skinSThresholdLowSlider->setValue(0);
                ui.skinSThresholdHighSlider->setValue(256);
                ui.skinVThresholdLowSlider->setValue(0);
                ui.skinVThresholdHighSlider->setValue(256);
                
                ui.approxKernelSlider->setValue(5);
                ui.approxHThresholdLowSlider->setValue(0);
                ui.approxHThresholdHighSlider->setValue(0);
                ui.approxSThresholdLowSlider->setValue(0);
                ui.approxSThresholdHighSlider->setValue(0);
                ui.approxVThresholdLowSlider->setValue(0);
                ui.approxVThresholdHighSlider->setValue(0);

                ui.camDefaultButton->setText("Stop");
                //bCalibrateApproxHLow = true, bCalibrateApproxHHigh = true, bCalibrateApproxSLow = true, bCalibrateApproxSHigh = true, bCalibrateApproxVLow = true, bCalibrateApproxVHigh = true;
                if (bWhiteBalance)
                    bcamWhiteBalanceCalibrate = true;
                else
                    bcamWhiteBalanceCalibrate = false;
            }
            else ui.camDefaultButton->setText("Calibrate");

        });
}