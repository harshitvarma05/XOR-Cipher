#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QElapsedTimer>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTableWidget>

#include "Tree.h"      
#include "Encryptor.h" 
#include "Crypto.h"    

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
   
    void on_selectFileEncrypt_clicked();
    void on_yesEncrypt_clicked();
    void on_noEncrypt_clicked();
    void on_encryptButton_clicked();

   
    void on_selectFileDecrypt_clicked();
    void on_decryptButton_clicked();

   
    void on_selectFileCompare_clicked();
    void on_runCompare_clicked();

private:
    Ui::MainWindow *ui;

   
    DecisionTree treeEncrypt;
    Node*        currentEncrypt = nullptr;
    QString      selectedFileEncrypt;
    QString      keyEncrypt;
    void         resetQuestionsEncrypt();

   
    QString selectedFileDecrypt;

   
    QString compareFile;
};

#endif
