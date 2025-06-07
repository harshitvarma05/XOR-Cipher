#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      currentEncrypt(nullptr),
      currentDecrypt(nullptr)
{
    ui->setupUi(this);

    // Build the two decision trees
    treeEncrypt.buildSampleTree();
    treeDecrypt.buildSampleTree();

    // Initialize both flows
    resetEncryptFlow();
    resetDecryptFlow();
}

MainWindow::~MainWindow() {
    delete ui;
}

// ─── ENCRYPT FLOW ─────────────────────────────────────────────

void MainWindow::resetEncryptFlow() {
    currentEncrypt = treeEncrypt.getRoot();
    keyEncrypt.clear();
    ui->questionEncrypt->setText(QString::fromStdString(currentEncrypt->question));
    ui->progressEncrypt->setVisible(false);
    ui->statusEncrypt->clear();
    selectedFileEncrypt.clear();
}

void MainWindow::updateQuestionEncrypt() {
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->setText("Key Complete: " + QString::fromStdString(keyEncrypt));
    } else {
        ui->questionEncrypt->setText(QString::fromStdString(currentEncrypt->question));
    }
}

void MainWindow::on_selectFileEncrypt_clicked() {
    resetEncryptFlow();
    selectedFileEncrypt = QFileDialog::getOpenFileName(
        this,
        tr("Select File to Encrypt"),
        QString(),
        tr("All Files (*.*)")
    );
    if (selectedFileEncrypt.isEmpty())
        return;
    ui->statusEncrypt->append("Selected: " + selectedFileEncrypt);
}

void MainWindow::on_yesEncrypt_clicked() {
    if (!currentEncrypt->yes) return;
    keyEncrypt += '1';
    currentEncrypt = currentEncrypt->yes;
    updateQuestionEncrypt();
}

void MainWindow::on_noEncrypt_clicked() {
    if (!currentEncrypt->no) return;
    keyEncrypt += '0';
    currentEncrypt = currentEncrypt->no;
    updateQuestionEncrypt();
}

void MainWindow::on_encryptButton_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a file first.");
        return;
    }

    ui->progressEncrypt->setVisible(true);
    ui->statusEncrypt->append("Encrypting...");
    ui->progressEncrypt->setRange(0, 0); // spin

    Encryptor enc(keyEncrypt);
    std::string in  = selectedFileEncrypt.toStdString();
    std::string out = in + ".enc";
    enc.encryptFile(in, out);

    ui->progressEncrypt->setRange(0, 1); // stop spin
    ui->statusEncrypt->append("Encrypted → " + QString::fromStdString(out));
}

// ─── DECRYPT FLOW ─────────────────────────────────────────────

void MainWindow::resetDecryptFlow() {
    currentDecrypt = treeDecrypt.getRoot();
    keyDecrypt.clear();
    ui->questionDecrypt->setText(QString::fromStdString(currentDecrypt->question));
    ui->progressDecrypt->setVisible(false);
    ui->statusDecrypt->clear();
    selectedFileDecrypt.clear();
}

void MainWindow::updateQuestionDecrypt() {
    if (!currentDecrypt->yes && !currentDecrypt->no) {
        ui->questionDecrypt->setText("Key Complete: " + QString::fromStdString(keyDecrypt));
    } else {
        ui->questionDecrypt->setText(QString::fromStdString(currentDecrypt->question));
    }
}

void MainWindow::on_selectFileDecrypt_clicked() {
    resetDecryptFlow();
    selectedFileDecrypt = QFileDialog::getOpenFileName(
        this,
        tr("Select File to Decrypt"),
        QString(),
        tr("All Files (*.*)")
    );
    if (selectedFileDecrypt.isEmpty())
        return;
    ui->statusDecrypt->append("Selected: " + selectedFileDecrypt);
}

void MainWindow::on_yesDecrypt_clicked() {
    if (!currentDecrypt->yes) return;
    keyDecrypt += '1';
    currentDecrypt = currentDecrypt->yes;
    updateQuestionDecrypt();
}

void MainWindow::on_noDecrypt_clicked() {
    if (!currentDecrypt->no) return;
    keyDecrypt += '0';
    currentDecrypt = currentDecrypt->no;
    updateQuestionDecrypt();
}

void MainWindow::on_decryptButton_clicked() {
    if (selectedFileDecrypt.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a file first.");
        return;
    }

    ui->progressDecrypt->setVisible(true);
    ui->statusDecrypt->append("Decrypting...");
    ui->progressDecrypt->setRange(0, 0); // spin

    Encryptor enc(keyDecrypt);
    QString inQ = selectedFileDecrypt;
    QString outQ;

    // If file ends with .enc, strip it; otherwise add .dec
    if (inQ.endsWith(".enc", Qt::CaseInsensitive)) {
        outQ = inQ.left(inQ.length() - 4);
    } else {
        outQ = inQ + ".dec";
    }

    enc.decryptFile(inQ.toStdString(), outQ.toStdString());

    ui->progressDecrypt->setRange(0, 1); // stop spin
    ui->statusDecrypt->append("Decrypted → " + outQ);
}
