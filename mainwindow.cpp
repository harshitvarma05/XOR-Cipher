#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileInfo>
#include <filesystem>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentEncrypt(nullptr)
{
    ui->setupUi(this);

    // Initialize encryption decision tree
    treeEncrypt.buildFileBasedTree();
    resetQuestionsEncrypt();

    // Decrypt tab: hide until file selected
    ui->progressDecrypt->hide();
    ui->statusDecrypt->clear();

    // Prepare compare table
    ui->compareTable->setRowCount(2);
    ui->compareTable->setColumnCount(2);
    ui->compareTable->setHorizontalHeaderLabels(
      QStringList{"Algorithm", "Time (ms)"}
    );
    ui->compareTable->horizontalHeader()
      ->setSectionResizeMode(QHeaderView::Stretch);
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
    ui->yesEncrypt->show();
    ui->noEncrypt->show();
    ui->progressEncrypt->hide();
    ui->statusEncrypt->clear();
}

void MainWindow::on_selectFileEncrypt_clicked() {
    selectedFileEncrypt = QFileDialog::getOpenFileName(
      this, tr("Select File to Encrypt")
    );
    if (selectedFileEncrypt.isEmpty()) return;
    ui->statusEncrypt->append("Selected: " + selectedFileEncrypt);
    resetQuestionsEncrypt();
}

void MainWindow::on_yesEncrypt_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }
    if (!currentEncrypt->check(selectedFileEncrypt)) {
        QMessageBox::critical(this, tr("Error"),
            tr("Wrong answer; restarting questions."));
        resetQuestionsEncrypt();
        return;
    }
    keyEncrypt.push_back('1');
    currentEncrypt = currentEncrypt->yes;
    ui->questionEncrypt->setText(currentEncrypt->question);
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->hide();
        ui->yesEncrypt->hide();
        ui->noEncrypt->hide();
    }
}

void MainWindow::on_noEncrypt_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }
    if (currentEncrypt->check(selectedFileEncrypt)) {
        QMessageBox::critical(this, tr("Error"),
            tr("Wrong answer; restarting questions."));
        resetQuestionsEncrypt();
        return;
    }
    keyEncrypt.push_back('0');
    currentEncrypt = currentEncrypt->no;
    ui->questionEncrypt->setText(currentEncrypt->question);
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->hide();
        ui->yesEncrypt->hide();
        ui->noEncrypt->hide();
    }
}

void MainWindow::on_encryptButton_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Select a file first."));
        return;
    }
    QString savePath = QFileDialog::getSaveFileName(
      this, tr("Save Encrypted File As"),
      selectedFileEncrypt + ".enc"
    );
    if (savePath.isEmpty()) return;

    ui->progressEncrypt->show();
    ui->progressEncrypt->setRange(0, 0);
    Encryptor enc(keyEncrypt);
    enc.encryptFile(selectedFileEncrypt.toStdString(),
                    savePath.toStdString());
    ui->progressEncrypt->setRange(0, 1);
    ui->statusEncrypt->append("Done: " + savePath);
}

// ─── DECRYPTION ─────────────────────────────────────────────────

void MainWindow::on_selectFileDecrypt_clicked() {
    selectedFileDecrypt = QFileDialog::getOpenFileName(
      this, tr("Select File to Decrypt"), "", tr("*.enc")
    );
    if (selectedFileDecrypt.isEmpty()) return;
    ui->statusDecrypt->append("Selected: " + selectedFileDecrypt);
}

void MainWindow::on_decryptButton_clicked() {
    if (selectedFileDecrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }

    // Use QFileInfo to extract directory and base name (without all extensions)
    QFileInfo fi(selectedFileDecrypt);
    QString baseName = fi.completeBaseName();           // “file.txt” from “file.txt.enc”
    QString initialPath = fi.absolutePath() + "/" + baseName;

    // Suggest the original filename (minus “.enc”) in the save dialog
    QString savePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Decrypted File As"),
        initialPath,
        tr("All Files (*.*)")
    );
    if (savePath.isEmpty()) return;

    ui->progressDecrypt->show();
    ui->progressDecrypt->setRange(0, 0);
    ui->statusDecrypt->append(tr("Decrypting to: %1").arg(savePath));

    Encryptor enc("");  // key read from header
    enc.decryptFile(
        selectedFileDecrypt.toStdString(),
        savePath.toStdString()
    );

    ui->progressDecrypt->setRange(0, 1);
    ui->statusDecrypt->append(
        tr("Done! Saved to %1").arg(savePath)
    );
}

// ─── COMPARISON ─────────────────────────────────────────────────

void MainWindow::on_selectFileCompare_clicked() {
    compareFile = QFileDialog::getOpenFileName(
      this, tr("Select File for Comparison")
    );
    if (compareFile.isEmpty()) return;
    QFileInfo info(compareFile);
    ui->filePathCompare->setText(info.absoluteFilePath());
    ui->fileSizeCompare->setText(QString::number(info.size()) + " bytes");
}

void MainWindow::on_runCompare_clicked() {
    if (compareFile.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Select a file first."));
        return;
    }
    QElapsedTimer timer; timer.start();
    Encryptor enc("101");
    std::string in = compareFile.toStdString();
    std::string tmp = in + ".tmp";
    enc.encryptFile(in, tmp);
    qint64 xorMs = timer.elapsed();
    QFile::remove(QString::fromStdString(tmp));

    qint64 rsaMs = xorMs * 5;  // crude estimate

    ui->compareTable->setItem(0, 0,
        new QTableWidgetItem("Decision-Tree XOR"));
    ui->compareTable->setItem(0, 1,
        new QTableWidgetItem(QString::number(xorMs)));
    ui->compareTable->setItem(1, 0,
        new QTableWidgetItem("RSA (est.)"));
    ui->compareTable->setItem(1, 1,
        new QTableWidgetItem(QString::number(rsaMs)));
}
