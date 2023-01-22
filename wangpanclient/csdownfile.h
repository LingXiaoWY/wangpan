#ifndef CSDOWNFILE_H
#define CSDOWNFILE_H

#include <QDialog>
#include <vector>
#include <QString>
#include <QDebug>
#include <QModelIndex>
#include <QStringListModel>


namespace Ui {
class CSDownFile;
}

class CSDownFile : public QDialog
{
    Q_OBJECT
signals:
    void SIG_downloadFile(QString);
public:
    explicit CSDownFile(QWidget *parent = 0);
    ~CSDownFile();
public slots:
    void slot_SelectFile(std::vector<QString> &filenameVec);
    void slot_listView_clicked(const QModelIndex &index);
private:
    Ui::CSDownFile *ui;
    QStringListModel *m_model;
};

#endif // CSDOWNFILE_H
