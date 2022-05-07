#pragma once
#include "Global.h"

class OptionCheck : public QCheckBox
{
    Q_OBJECT

public:
    using QCheckBox::QCheckBox;

    void setup(bool *checked, QListWidget *list, std::vector<QWidget*> vec, bool bFlipped = false, std::function <bool(bool)> action = nullptr)
    {
        this->list = list;
        this->vec = vec;
        this->bFlipped = bFlipped;
        this->checked = checked;
        this->action = action;
        this->setChecked(*checked);
        connect(this, &QCheckBox::stateChanged, this, &OptionCheck::updateState);
        this->updateState(*checked ? 2 : 0);
    }

private slots:
    void updateState(int val)
    {
        bool checked = (val == 2);
        if (action)
        {
            if ((*(this->checked) = action(val)) != this->isChecked())
                this->setChecked(*(this->checked));
            return;
        }
        else
            *(this->checked) = val;

        for (auto it = vec.begin(); it != vec.end(); ++it)
            (*it)->setEnabled(bFlipped ? !checked : checked);

        for (int i = 0; i < list->count(); ++i)
        {
            QListWidgetItem* item = list->item(i);
            if (!item->text().compare(this->text())) //jeżeli pole w liście ma taki sam text jak checkbox
                item->setFlags((item->flags() | Qt::ItemFlag::ItemIsEnabled) & ~(!checked << 5)); //5 bit to Qt::ItemFlag::ItemIsEnabled
        }
    }
private:
    std::function <bool(bool)> action = nullptr;
    QListWidget* list = nullptr;
    std::vector<QWidget*> vec;
    bool *checked = nullptr, bFlipped = false;
};