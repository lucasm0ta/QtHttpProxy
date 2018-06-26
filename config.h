#ifndef CONFIG_H
#define CONFIG_H

#include <QDialog>

namespace Ui {
class Config;
}

class Config : public QDialog
{
    Q_OBJECT

public:
    explicit Config(QWidget *parent = 0);
    ~Config();

private slots:
    void on_btn_accepted();

    void on_btn_rejected();

signals:
    void done(int port);

private:
    Ui::Config *ui;
};

#endif // CONFIG_H
