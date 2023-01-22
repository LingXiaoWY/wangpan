#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QTimer>
#include "cselectfile.h"
#include "csdownfile.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT
signals:
    void SIG_getFile();
    void SIG_upLoad();
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setFileNum(int num);
private slots:
    void on_pb_upload_clicked();

    void slot_GetFileName();
public:
    QStandardItemModel *m_tableModel1,*m_tableModel2,*m_tableModel3;
    CSelectFile *m_pCSelectFile;
    CSDownFile *m_pCSDownFile;
private:
    Ui::MainWindow *ui;
    QTimer m_getTimer;
};

#endif // MAINWINDOW_H
