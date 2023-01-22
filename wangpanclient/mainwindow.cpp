#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QDebug>

#include <QString>


MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //设置文件列表表头
    m_tableModel1 = new  QStandardItemModel;
    m_tableModel2 = new  QStandardItemModel;
    m_tableModel3 = new  QStandardItemModel;
    ui->tb_V1->setModel(m_tableModel1);
    ui->tb_V2->setModel(m_tableModel2);
    ui->tb_V3->setModel(m_tableModel3);
    m_tableModel1->setHorizontalHeaderItem(0, new QStandardItem("文件名"));
    m_tableModel1->setHorizontalHeaderItem(1, new QStandardItem("文件大小(B)"));
    m_tableModel1->setHorizontalHeaderItem(2, new QStandardItem("上传时间"));
    m_tableModel2->setHorizontalHeaderItem(0, new QStandardItem("文件名"));
    m_tableModel2->setHorizontalHeaderItem(1, new QStandardItem("已上传文件大小(B)"));
    m_tableModel2->setHorizontalHeaderItem(2, new QStandardItem("上传时间"));
    m_tableModel3->setHorizontalHeaderItem(0, new QStandardItem("文件名"));
    m_tableModel3->setHorizontalHeaderItem(1, new QStandardItem("已下载文件大小(B)"));
    m_tableModel3->setHorizontalHeaderItem(2, new QStandardItem("下载时间"));
    m_pCSelectFile = new CSelectFile;
    m_pCSDownFile = new CSDownFile;
    //绑定获取文件列表超时信号
    connect(&m_getTimer,SIGNAL(timeout()),this,SLOT(slot_GetFileName()));
    connect(ui->pb_delete,&QPushButton::clicked,[&](bool){m_pCSelectFile->show();});
    connect(ui->pb_download,&QPushButton::clicked,[&](bool){m_pCSDownFile->show();});
    m_getTimer.start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_getTimer.stop();
}

void MainWindow::setFileNum(int num)
{
    ui->le_num->setText(QString("%1").arg(num));
}

void MainWindow::on_pb_upload_clicked()
{
    Q_EMIT SIG_upLoad();
}

void MainWindow::slot_GetFileName()
{
    Q_EMIT SIG_getFile();
}
