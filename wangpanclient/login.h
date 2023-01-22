#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QDebug>
#include "packdef.h"
#include <QString>
#include <QRegExp>
#include <QCloseEvent>

namespace Ui {
class login;
}

class login : public QWidget
{
    Q_OBJECT
signals:
    void SIG_Login(QString,QString);
    void SIG_Register(QString,QString,QString);
    void SIG_close();
public:
    explicit login(QWidget *parent = 0);
    ~login();

    void closeEvent(QCloseEvent *e);

    void on_pb_clear_register_clicked();
private slots:
    void on_pb_clear_clicked();

    void on_pb_login_clicked();

    void on_pb_register_clicked();

private:
    Ui::login *ui;
};

#endif // LOGIN_H
