#include "config.h"
#include "ui_config.h"

#include <QDebug>

Config::Config(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Config)
{
    ui->setupUi(this);
}

Config::~Config()
{
    delete ui;
}

void Config::on_btn_accepted()
{
    qDebug() << "Accepted";
    emit done(1111);
}

void Config::on_btn_rejected()
{
    qDebug() << "Rejected";
    emit rejected();
}
