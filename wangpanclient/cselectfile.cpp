#include "cselectfile.h"
#include "ui_cselectfile.h"




CSelectFile::CSelectFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSelectFile)
{
    ui->setupUi(this);
    connect(ui->ls_1,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(slot_listView_clicked(QModelIndex)));
    m_model = new QStringListModel(this);

}

CSelectFile::~CSelectFile()
{
    delete ui;
}

void CSelectFile::slot_SelectFile(std::vector<QString> &filenameVec)
{
    QStringList list;
    for(auto i:filenameVec)
        list<<i;
    m_model->setStringList(list);
    ui->ls_1->setModel(m_model);
}

void CSelectFile::slot_listView_clicked(const QModelIndex &index)
{
    Q_EMIT SIG_deleteFile(m_model->data(index,Qt::DisplayRole).toString());
}
