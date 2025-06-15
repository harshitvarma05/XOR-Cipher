#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QHeaderView>
#include <QElapsedTimer>
#include <QFile>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ── Encrypt setup ──
    treeEncrypt.buildFileBasedTree();
    resetQuestionsEncrypt();

    // ── Hide decrypt progress ──
    ui->progressDecrypt->hide();
    ui->statusDecrypt->clear();

    // ── Prepare Compare table ──
    ui->compareTable->setRowCount(2);
    ui->compareTable->setColumnCount(3);
    ui->compareTable->setHorizontalHeaderLabels(
      QStringList{"Algorithm","Encrypt (ms)","Decrypt (ms)"});
    ui->compareTable->horizontalHeader()
       ->setSectionResizeMode(QHeaderView::Stretch);

    // **Pre-populate** all cells so item(r,c) is never null
    for(int r = 0; r < 2; ++r) {
      for(int c = 0; c < 3; ++c) {
        ui->compareTable->setItem(r, c, new QTableWidgetItem());
      }
    }

    ui->compareTable->hide();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::resetQuestionsEncrypt() {
    keyEncrypt.clear();
    currentEncrypt = treeEncrypt.getRoot();
    ui->questionEncrypt->setText(currentEncrypt->question);
    ui->questionEncrypt->show();
    ui->yesEncrypt   ->show();
    ui->noEncrypt    ->show();
    ui->progressEncrypt->hide();
    ui->statusEncrypt  ->clear();
}

// ── ENCRYPT ────────────────────────────────────────────

void MainWindow::on_selectFileEncrypt_clicked() {
    QString file = QFileDialog::getOpenFileName(
      this, tr("Select File to Encrypt"), {}, tr("All Files (*.*)"));
    if (file.isEmpty()) return;
    selectedFileEncrypt = file;
    ui->statusEncrypt->append("Selected: " + file);
    resetQuestionsEncrypt();
}

void MainWindow::on_yesEncrypt_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }
    if (!currentEncrypt->check(selectedFileEncrypt)) {
        QMessageBox::critical(this, tr("Wrong Answer"),
            tr("Your ‘Yes’ contradicts the file property.\nRestarting."));
        resetQuestionsEncrypt();
        return;
    }
    keyEncrypt += '1';
    currentEncrypt = currentEncrypt->yes;
    ui->questionEncrypt->setText(currentEncrypt->question);
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->hide();
        ui->yesEncrypt   ->hide();
        ui->noEncrypt    ->hide();
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
            tr("Your ‘No’ contradicts the file property.\nRestarting."));
        resetQuestionsEncrypt();
        return;
    }
    keyEncrypt += '0';
    currentEncrypt = currentEncrypt->no;
    ui->questionEncrypt->setText(currentEncrypt->question);
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->hide();
        ui->yesEncrypt   ->hide();
        ui->noEncrypt    ->hide();
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
    ui->progressEncrypt->setRange(0,0);
    ui->statusEncrypt->append("Encrypting…");

    Encryptor xorEnc(keyEncrypt.toStdString());
    xorEnc.encryptFile(
      selectedFileEncrypt.toStdString(),
      savePath.toStdString()
    );

    ui->progressEncrypt->setRange(0,1);
    ui->statusEncrypt->append("Done: " + savePath);
}

// ── DECRYPT ────────────────────────────────────────────

void MainWindow::on_selectFileDecrypt_clicked() {
    selectedFileDecrypt = QFileDialog::getOpenFileName(
      this, tr("Select File to Decrypt"), {}, tr("Encrypted Files (*.enc)"));
    if (selectedFileDecrypt.isEmpty()) return;
    ui->statusDecrypt->append("Selected: " + selectedFileDecrypt);
}

void MainWindow::on_decryptButton_clicked() {
    if (selectedFileDecrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }
    QFileInfo fi(selectedFileDecrypt);
    QString base = fi.completeBaseName();
    QString savePath = QFileDialog::getSaveFileName(
      this, tr("Save Decrypted File As"),
      fi.absolutePath() + "/" + base,
      tr("All Files (*.*)")
    );
    if (savePath.isEmpty()) return;

    ui->progressDecrypt->show();
    ui->progressDecrypt->setRange(0,0);
    ui->statusDecrypt->append("Decrypting…");

    Encryptor xorDec("");
    xorDec.decryptFile(
      selectedFileDecrypt.toStdString(),
      savePath.toStdString()
    );

    ui->progressDecrypt->setRange(0,1);
    ui->statusDecrypt->append("Done: " + savePath);
}

// ── COMPARE ────────────────────────────────────────────

void MainWindow::on_selectFileCompare_clicked() {
    compareFile = QFileDialog::getOpenFileName(
      this, tr("Select File for Comparison"), {}, tr("All Files (*.*)"));
    if (compareFile.isEmpty()) return;
    QFileInfo fi(compareFile);
    ui->filePathCompare->setText(fi.fileName());
    ui->fileSizeCompare->setText(
      QString::number(fi.size()/1024.0,'f',1) + " KB");
    ui->compareTable->hide();
}

void MainWindow::on_runCompare_clicked() {
    if (compareFile.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Please select a file first."));
        return;
    }

    QElapsedTimer t; t.start();
    QString tmpXorEnc = QDir::temp().filePath("cmp_xor.enc");
    QString tmpXorDec = QDir::temp().filePath("cmp_xor.dec");
    Encryptor xorEnc("0");
    xorEnc.encryptFile(compareFile.toStdString(), tmpXorEnc.toStdString());
    qint64 xorEncMs = t.elapsed();
    t.restart();
    xorEnc.decryptFile(tmpXorEnc.toStdString(), tmpXorDec.toStdString());
    qint64 xorDecMs = t.elapsed();
    QFile::remove(tmpXorEnc);
    QFile::remove(tmpXorDec);

    QString tmpArEnc = QDir::temp().filePath("cmp_ar.enc");
    QString tmpArDec = QDir::temp().filePath("cmp_ar.dec");
    double arEncMs = hybridEncrypt(
      compareFile.toStdString(),
      tmpArEnc.toStdString(),
      "keys/public.pem"
    );
    double arDecMs = hybridDecrypt(
      tmpArEnc.toStdString(),
      tmpArDec.toStdString(),
      "keys/private.pem"
    );
    QFile::remove(tmpArEnc);
    QFile::remove(tmpArDec);

    ui->compareTable->item(0,0)->setText("XOR");
    ui->compareTable->item(0,1)->setText(QString::number(xorEncMs));
    ui->compareTable->item(0,2)->setText(QString::number(xorDecMs));
    ui->compareTable->item(1,0)->setText("AES+RSA");
    ui->compareTable->item(1,1)->setText(QString::number(arEncMs,'f',1));
    ui->compareTable->item(1,2)->setText(QString::number(arDecMs,'f',1));

    ui->compareTable->show();
}
