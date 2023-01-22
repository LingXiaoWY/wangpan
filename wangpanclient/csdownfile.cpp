#include "csdownfile.h"
#include "ui_csdownfile.h"

CSDownFile::CSDownFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDownFile)
{
    ui->setupUi(this);
    connect(ui->listView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(slot_listView_clicked(QModelIndex)));
    m_model = new QStringListModel(this);
}

CSDownFile::~CSDownFile()
{
    delete ui;
}

void CSDownFile::slot_SelectFile(std::vector<QString> &filenameVec)
{
    QStringList list;
    for(auto i:filenameVec)
        list<<i;
    m_model->setStringList(list);
    ui->listView->setModel(m_model);
}

void CSDownFile::slot_listView_clicked(const QModelIndex &index)
{
    Q_EMIT SIG_downloadFile(m_model->data(index,Qt::DisplayRole).toString());
}
