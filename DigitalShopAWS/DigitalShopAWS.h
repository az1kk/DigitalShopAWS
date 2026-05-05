#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DigitalShopAWS.h"

class DigitalShopAWS : public QMainWindow
{
    Q_OBJECT

public:
    DigitalShopAWS(QWidget *parent = nullptr);
    ~DigitalShopAWS();

private:
    Ui::DigitalShopAWSClass ui;
};

