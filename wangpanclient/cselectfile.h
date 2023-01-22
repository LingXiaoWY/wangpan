#ifndef CSELECTFILE_H
#define CSELECTFILE_H

#include <QDialog>
#include <vector>
#include <QString>
#include <QDebug>
#include <QModelIndex>
#include <QStringListModel>

namespace Ui {
class CSelectFile;
}

class CSelectFile : public QDialog
{
    Q_OBJECT
signals:
    void SIG_deleteFile(QString);
public:
    explicit CSelectFile(QWidget *parent = 0);
    ~CSelectFile();
public slots:
    void slot_SelectFile(std::vector<QString> &filenameVec);
    void slot_listView_clicked(const QModelIndex &index);
private:
    Ui::CSelectFile *ui;
    QStringListModel *m_model;
};

#endif // CSELECTFILE_H
