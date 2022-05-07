#pragma once
#include "OptionLabel.h"

class OptionSlider : public QSlider
{
    Q_OBJECT

public:
    using QSlider::QSlider;

    void setup(OptionLabel* label, SliderVal *val,  std::function <unsigned(unsigned)> action = nullptr)
    {
        connect(this, &QSlider::valueChanged, label, &OptionLabel::setVal);
        this->setMinimum(val->min);
        this->setMaximum(val->max);
        this->val = &(val->val);
        this->action = action;
        connect(this, &QSlider::valueChanged, this, &OptionSlider::updateVal);
        this->setValue(val->val);
        label->setVal(val->val);
    }
    void refresh() { if (this->val) updateVal(*(this->val)); }
private slots:    
    void updateVal(unsigned val)
    {
        if (action)
        {
            if ((*(this->val) = action(val)) != this->value())
                this->setValue(*(this->val));
        }
        else if (this->val)
            *(this->val) = val;
    }
private:
    unsigned *val = nullptr;
    std::function <unsigned(unsigned)> action = nullptr;
};