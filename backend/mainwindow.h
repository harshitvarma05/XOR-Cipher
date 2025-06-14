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
    // Encrypt tab slots
    void on_selectFileEncrypt_clicked();
    void on_yesEncrypt_clicked();
    void on_noEncrypt_clicked();
    void on_encryptButton_clicked();

    // Decrypt tab slots
    void on_selectFileDecrypt_clicked();
    void on_decryptButton_clicked();

    // Compare tab slots
    void on_selectFileCompare_clicked();
    void on_runCompare_clicked();

private:
    Ui::MainWindow *ui;

    // Encryption state
    DecisionTree treeEncrypt;
    Node* currentEncrypt;
    std::string keyEncrypt;
    QString selectedFileEncrypt;

    // Decryption state
    QString selectedFileDecrypt;

    // Comparison state
    QString compareFile;

    void resetQuestionsEncrypt();
};

#endif // MAINWINDOW_H
