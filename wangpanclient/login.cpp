#include "login.h"
#include "ui_login.h"
#include <QMessageBox>


login::login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
}

login::~login()
{
    delete ui;
}

void login::closeEvent(QCloseEvent *e)
{
    if(QMessageBox::question(this,"退出","是否退出") == QMessageBox::Yes)
    {
        //发信号
        Q_EMIT SIG_close();
        //同意关闭事件
        e->accept();
    }
    else
    {
        //忽略关闭事件
        e->ignore();
    }
}

void login::on_pb_clear_clicked()
{
    ui->le_tel->setText("");
    ui->le_password->setText("");
}

void login::on_pb_login_clicked()
{
    //注册信息的过滤
    QString tel = ui->le_tel->text();
    QString password = ui->le_password->text();

    //上述每一个不能为空 QString isEmpty() -> ""
    if( tel.isEmpty()||password.isEmpty() )
    {
        QMessageBox::about(this,"提示","内容不能为空");
        return;
    }
    //手机号 要合法-->正则表达式 --> 1 3-8 0-9 9位 ^开头 $结尾
    //创建正则对象
    QRegExp exp("^1[3-8][0-9]\{9\}$"); //表达式传参
    if(!exp.exactMatch(tel))
    {
        QMessageBox::about(this,"提示","手机号不合法,11位手机号");
        return;
    }
    //密码不能过长
    if(password.length() > 20)
    {
        QMessageBox::about(this,"提示","密码过长，长度不能超过20");
        return;
    }
    Q_EMIT SIG_Login(tel,password);
}

void login::on_pb_clear_register_clicked()
{
    ui->le_confirm_register->setText("");
    ui->le_name_register->setText("");
    ui->le_password_register->setText("");
    ui->le_tel_register->setText("");
}

void login::on_pb_register_clicked()
{
    QString pwd,pwd_confirm,tel;
    pwd = ui->le_password_register->text();
    pwd_confirm = ui->le_confirm_register->text();
    tel = ui->le_tel_register->text();

    QRegExp exp("^1[3-8][0-9]\{9\}$"); //表达式传参
    if(!exp.exactMatch(tel))
    {
       QMessageBox::about(this,"提示","手机号不合法,11位手机号");
       ui->le_tel_register->setText("");
       return;
    }
    //密码不能过长
    if(pwd.length() > 20)
    {
       QMessageBox::about(this,"提示","密码过长，长度不能超过20");
       ui->le_password_register->setText("");
       ui->le_confirm_register->setText("");
       return;
    }
    //昵称 不能全是空格 不能过长
    QString tmpName = ui->le_name_register->text();
    tmpName = tmpName.remove(' '); //remove 操作会改变原字符串
    if(tmpName.length() > 20 || tmpName.isEmpty())
    {
       QMessageBox::about(this,"提示","昵称为空或者昵称过长，长度不能超过20");
       ui->le_name_register->setText("");
       return;
    }
    if(pwd != pwd_confirm)
    {
        QMessageBox::critical(this,"错误","输入的两次密码不一致，请重新输入");
        ui->le_password_register->setText("");
        ui->le_confirm_register->setText("");
        return;
    }

    Q_EMIT SIG_Register(ui->le_name_register->text(),ui->le_tel_register->text(),ui->le_password_register->text());
}
