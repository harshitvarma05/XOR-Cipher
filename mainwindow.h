#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "Tree.h"      // still used for encryption questions
#include "Encryptor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void resetQuestionsEncrypt();

private slots:
    // Encryption
    void on_selectFileEncrypt_clicked();
    void on_yesEncrypt_clicked();
    void on_noEncrypt_clicked();
    void on_encryptButton_clicked();

    // Decryption
    void on_selectFileDecrypt_clicked();
    void on_decryptButton_clicked();

private:
    Ui::MainWindow* ui;

    // Encryption state
    QString       selectedFileEncrypt;
    DecisionTree  treeEncrypt;
    Node*         currentEncrypt;
    std::string   keyEncrypt;

    // Decryption state
    QString       selectedFileDecrypt;
};

#endif // MAINWINDOW_H
