#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <filesystem>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      currentEncrypt(nullptr)
{
    ui->setupUi(this);

    treeEncrypt.buildFileBasedTree();
    resetQuestionsEncrypt();

    ui->questionDecrypt->hide();
    ui->yesDecrypt     ->hide();
    ui->noDecrypt      ->hide();
    ui->progressDecrypt->hide();
    ui->statusDecrypt  ->clear();
}

MainWindow::~MainWindow() {
    delete ui;
}

// ─── ENCRYPTION ─────────────────────────────────────────────────

void MainWindow::resetQuestionsEncrypt() {
    keyEncrypt.clear();
    currentEncrypt = treeEncrypt.getRoot();
    ui->questionEncrypt->setText(currentEncrypt->question);
    ui->questionEncrypt->show();
    ui->yesEncrypt     ->show();
    ui->noEncrypt      ->show();
    ui->progressEncrypt->hide();
    ui->statusEncrypt  ->clear();
}

void MainWindow::on_selectFileEncrypt_clicked() {
    resetQuestionsEncrypt();
    auto file = QFileDialog::getOpenFileName(
        this, tr("Select File to Encrypt"),
        QString(), tr("All Files (*.*)")
    );
    if (file.isEmpty()) return;
    selectedFileEncrypt = file;
    ui->statusEncrypt->append(tr("Selected: %1").arg(file));

    treeEncrypt.buildFileBasedTree();
    currentEncrypt = treeEncrypt.getRoot();
    ui->questionEncrypt->setText(currentEncrypt->question);
}

void MainWindow::on_yesEncrypt_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }
    if (!currentEncrypt->check(selectedFileEncrypt)) {
        QMessageBox::critical(this, tr("Wrong Answer"),
            tr("Your 'Yes' contradicts the file property.\nProcess aborted."));
        resetQuestionsEncrypt();
        return;
    }
    keyEncrypt.push_back('1');
    currentEncrypt = currentEncrypt->yes;
    ui->questionEncrypt->setText(currentEncrypt->question);
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->hide();
        ui->yesEncrypt     ->hide();
        ui->noEncrypt      ->hide();
    }
}

void MainWindow::on_noEncrypt_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }
    if (currentEncrypt->check(selectedFileEncrypt)) {
        QMessageBox::critical(this, tr("Wrong Answer"),
            tr("Your 'No' contradicts the file property.\nProcess aborted."));
        resetQuestionsEncrypt();
        return;
    }
    keyEncrypt.push_back('0');
    currentEncrypt = currentEncrypt->no;
    ui->questionEncrypt->setText(currentEncrypt->question);
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->hide();
        ui->yesEncrypt     ->hide();
        ui->noEncrypt      ->hide();
    }
}

void MainWindow::on_encryptButton_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }
    if (currentEncrypt->yes || currentEncrypt->no) {
        QMessageBox::warning(this, tr("Incomplete"),
            tr("Answer all questions before encrypting."));
        return;
    }

        QString savePath = QFileDialog::getSaveFileName(
        this, tr("Save Encrypted File As"),
        selectedFileEncrypt + ".enc",
        tr("Encrypted Files (*.enc);;All Files (*.*)")
    );
    if (savePath.isEmpty()) return;

    ui->progressEncrypt->show();
    ui->progressEncrypt->setRange(0, 0);
    ui->statusEncrypt->append(tr("Encrypting to: %1").arg(savePath));

    Encryptor enc(keyEncrypt);
    enc.encryptFile(
        selectedFileEncrypt.toStdString(),
        savePath.toStdString()
    );

    ui->progressEncrypt->setRange(0, 1);
    ui->statusEncrypt->append(
        tr("Done! Saved to %1").arg(savePath)
    );
}

// ─── DECRYPTION ─────────────────────────────────────────────────

void MainWindow::on_selectFileDecrypt_clicked() {
    auto file = QFileDialog::getOpenFileName(
        this, tr("Select File to Decrypt"),
        QString(), tr("Encrypted Files (*.enc);;All Files (*.*)")
    );
    if (file.isEmpty()) return;
    selectedFileDecrypt = file;
    ui->statusDecrypt->clear();
    ui->statusDecrypt->append(tr("Selected: %1").arg(file));
}

void MainWindow::on_decryptButton_clicked() {
    if (selectedFileDecrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }


    std::filesystem::path p(selectedFileDecrypt.toStdString());
    QString defaultName = QString::fromStdString(p.stem().string());

    QString savePath = QFileDialog::getSaveFileName(
        this, tr("Save Decrypted File As"),
        defaultName, tr("All Files (*.*)")
    );
    if (savePath.isEmpty()) return;

    ui->progressDecrypt->show();
    ui->progressDecrypt->setRange(0, 0);
    ui->statusDecrypt->append(tr("Decrypting to: %1").arg(savePath));

    Encryptor enc("");
    enc.decryptFile(
        selectedFileDecrypt.toStdString(),
        savePath.toStdString()
    );

    ui->progressDecrypt->setRange(0, 1);
    ui->statusDecrypt->append(
        tr("Done! Saved to %1").arg(savePath)
    );
}
