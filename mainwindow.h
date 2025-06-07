#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "Tree.h"
#include "Encryptor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Encrypt tab
    void on_selectFileEncrypt_clicked();
    void on_yesEncrypt_clicked();
    void on_noEncrypt_clicked();
    void on_encryptButton_clicked();

    // Decrypt tab
    void on_selectFileDecrypt_clicked();
    void on_yesDecrypt_clicked();
    void on_noDecrypt_clicked();
    void on_decryptButton_clicked();

private:
    Ui::MainWindow *ui;

    QString selectedFileEncrypt;
    QString selectedFileDecrypt;

    DecisionTree treeEncrypt;
    DecisionTree treeDecrypt;
    Node *currentEncrypt;
    Node *currentDecrypt;
    std::string keyEncrypt;
    std::string keyDecrypt;

    void updateQuestionEncrypt();
    void updateQuestionDecrypt();
    void resetEncryptFlow();
    void resetDecryptFlow();
};

#endif // MAINWINDOW_H
