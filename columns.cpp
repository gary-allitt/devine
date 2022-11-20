
#include "columns.h"
#include "ui_columns.h"

// the column selection dialog

Columns::Columns(QWidget *parent) :
    QDialog(parent),
  the_settings("oxenic", "devine"),
  ui(new Ui::Columns)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Columns::OnOk);
    ui->ckInstanceId->setChecked(the_settings.value("columns/instance_id", true).toBool());
    ui->ckHardwareId->setChecked(the_settings.value("columns/first_hardware_id", false).toBool());
    ui->ckManufacturer->setChecked(the_settings.value("columns/manufacturer", false).toBool());
    ui->ckProvider->setChecked(the_settings.value("columns/provider", false).toBool());
    ui->ckClass->setChecked(the_settings.value("columns/class", false).toBool());
}

Columns::~Columns()
{
    delete ui;
}

void Columns::OnOk()
{
  the_settings.setValue("columns/instance_id", ui->ckInstanceId->isChecked());
  the_settings.setValue("columns/first_hardware_id", ui->ckHardwareId->isChecked());
  the_settings.setValue("columns/manufacturer", ui->ckManufacturer->isChecked());
  the_settings.setValue("columns/provider", ui->ckProvider->isChecked());
  the_settings.setValue("columns/class", ui->ckClass->isChecked());
  accept(); 
}
