#pragma once
#include "Global.h"

class OptionLabel : public QLabel
{
    Q_OBJECT

public:
    using QLabel::QLabel;

    void setup(QString str = "", int prec = 0)
    { 
        this->prec = prec;
        this->baseValue = str;
    }

public slots:
    void setVal(unsigned val) 
    {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(std::max(0, prec)) << static_cast<double>(val)/pow(10,prec);
        this->setText(baseValue + QString::fromStdString(stream.str()));
    }

private:
    int prec = 0;
    QString baseValue;
};