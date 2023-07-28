#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QDialog>

struct device; 

namespace Ui {
class Properties;
}

class Properties : public QDialog
{
    Q_OBJECT

public:
    explicit Properties(QWidget *parent, const device& a_device);
    ~Properties();

private:
    Ui::Properties *ui;
    const device& the_device; 

    QString Status(uint32_t a_code);
};

#endif // PROPERTIES_H
