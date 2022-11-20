#ifndef COLUMNS_H
#define COLUMNS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class Columns;
}

class Columns : public QDialog
{
    Q_OBJECT

public:
    explicit Columns(QWidget *parent = nullptr);
    ~Columns();

private:
    Ui::Columns *ui;
    QSettings the_settings;

private slots:
  void OnOk();

};

#endif // COLUMNS_H
