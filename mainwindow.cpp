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
    treeEncrypt.buildFileBasedTree();
    treeDecrypt.buildFileBasedTree();
    resetQuestionsEncrypt();
    resetQuestionsDecrypt();
}

MainWindow::~MainWindow() {
    delete ui;
}

// ─── ENCRYPT TAB ────────────────────────────────────────────

void MainWindow::resetQuestionsEncrypt() {
    keyEncrypt.clear();
    currentEncrypt = treeEncrypt.getRoot();
    ui->questionEncrypt->setText(currentEncrypt->question);
    // ensure they're visible when starting
    ui->questionEncrypt->setVisible(true);
    ui->yesEncrypt->setVisible(true);
    ui->noEncrypt->setVisible(true);
    ui->progressEncrypt->setVisible(false);
    ui->statusEncrypt->clear();
}

void MainWindow::on_selectFileEncrypt_clicked() {
    resetQuestionsEncrypt();
    const QString file = QFileDialog::getOpenFileName(
        this,
        tr("Select File to Encrypt"),
        QString(),
        tr("All Files (*.*)")
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
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }
    bool actual = currentEncrypt->check(selectedFileEncrypt);
    if (!actual) {
        QMessageBox::critical(this, tr("Wrong Answer"),
            tr("Your 'Yes' contradicts the file property.\nProcess aborted."));
        resetQuestionsEncrypt();
        return;
    }
    keyEncrypt.push_back('1');
    currentEncrypt = currentEncrypt->yes;
    ui->questionEncrypt->setText(currentEncrypt->question);

    // ** If we've reached the leaf, hide question & answers **
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->setVisible(false);
        ui->yesEncrypt->setVisible(false);
        ui->noEncrypt->setVisible(false);
    }
}

void MainWindow::on_noEncrypt_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }
    bool actual = currentEncrypt->check(selectedFileEncrypt);
    if (actual) {
        QMessageBox::critical(this, tr("Wrong Answer"),
            tr("Your 'No' contradicts the file property.\nProcess aborted."));
        resetQuestionsEncrypt();
        return;
    }
    keyEncrypt.push_back('0');
    currentEncrypt = currentEncrypt->no;
    ui->questionEncrypt->setText(currentEncrypt->question);

    // ** Hide on leaf **
    if (!currentEncrypt->yes && !currentEncrypt->no) {
        ui->questionEncrypt->setVisible(false);
        ui->yesEncrypt->setVisible(false);
        ui->noEncrypt->setVisible(false);
    }
}

void MainWindow::on_encryptButton_clicked() {
    if (selectedFileEncrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }
    if (currentEncrypt->yes || currentEncrypt->no) {
        QMessageBox::warning(this, tr("Incomplete"),
            tr("Answer all questions before encrypting."));
        return;
    }
    ui->progressEncrypt->setVisible(true);
    ui->progressEncrypt->setRange(0, 0);
    ui->statusEncrypt->append(tr("Encrypting..."));

    Encryptor enc(keyEncrypt);
    const std::string in  = selectedFileEncrypt.toStdString();
    const std::string out = in + ".enc";
    enc.encryptFile(in, out);

    ui->progressEncrypt->setRange(0, 1);
    ui->statusEncrypt->append(
        tr("Encrypted → %1").arg(QString::fromStdString(out))
    );
}

// ─── DECRYPT TAB ────────────────────────────────────────────

void MainWindow::resetQuestionsDecrypt() {
    keyDecrypt.clear();
    currentDecrypt = treeDecrypt.getRoot();
    ui->questionDecrypt->setText(currentDecrypt->question);
    // ensure visibility
    ui->questionDecrypt->setVisible(true);
    ui->yesDecrypt->setVisible(true);
    ui->noDecrypt->setVisible(true);
    ui->progressDecrypt->setVisible(false);
    ui->statusDecrypt->clear();
}

void MainWindow::on_selectFileDecrypt_clicked() {
    resetQuestionsDecrypt();
    const QString file = QFileDialog::getOpenFileName(
        this,
        tr("Select File to Decrypt"),
        QString(),
        tr("All Files (*.*)")
    );
    if (file.isEmpty()) return;
    selectedFileDecrypt = file;
    ui->statusDecrypt->append(tr("Selected: %1").arg(file));
    treeDecrypt.buildFileBasedTree();
    currentDecrypt = treeDecrypt.getRoot();
    ui->questionDecrypt->setText(currentDecrypt->question);
}

void MainWindow::on_yesDecrypt_clicked() {
    if (selectedFileDecrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }
    bool actual = currentDecrypt->check(selectedFileDecrypt);
    if (!actual) {
        QMessageBox::critical(this, tr("Wrong Answer"),
            tr("Your 'Yes' contradicts the file property.\nProcess aborted."));
        resetQuestionsDecrypt();
        return;
    }
    keyDecrypt.push_back('1');
    currentDecrypt = currentDecrypt->yes;
    ui->questionDecrypt->setText(currentDecrypt->question);

    if (!currentDecrypt->yes && !currentDecrypt->no) {
        ui->questionDecrypt->setVisible(false);
        ui->yesDecrypt->setVisible(false);
        ui->noDecrypt->setVisible(false);
    }
}

void MainWindow::on_noDecrypt_clicked() {
    if (selectedFileDecrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }
    bool actual = currentDecrypt->check(selectedFileDecrypt);
    if (actual) {
        QMessageBox::critical(this, tr("Wrong Answer"),
            tr("Your 'No' contradicts the file property.\nProcess aborted."));
        resetQuestionsDecrypt();
        return;
    }
    keyDecrypt.push_back('0');
    currentDecrypt = currentDecrypt->no;
    ui->questionDecrypt->setText(currentDecrypt->question);

    if (!currentDecrypt->yes && !currentDecrypt->no) {
        ui->questionDecrypt->setVisible(false);
        ui->yesDecrypt->setVisible(false);
        ui->noDecrypt->setVisible(false);
    }
}

void MainWindow::on_decryptButton_clicked() {
    if (selectedFileDecrypt.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }
    if (currentDecrypt->yes || currentDecrypt->no) {
        QMessageBox::warning(this, tr("Incomplete"),
            tr("Answer all questions before decrypting."));
        return;
    }

    ui->progressDecrypt->setVisible(true);
    ui->progressDecrypt->setRange(0, 0);
    ui->statusDecrypt->append(tr("Decrypting..."));

    QString inQ  = selectedFileDecrypt;
    QString outQ = inQ.endsWith(".enc", Qt::CaseInsensitive)
                   ? inQ.left(inQ.length() - 4)
                   : inQ + ".dec";

    Encryptor enc(keyDecrypt);
    enc.decryptFile(inQ.toStdString(), outQ.toStdString());

    ui->progressDecrypt->setRange(0, 1);
    ui->statusDecrypt->append(tr("Decrypted → %1").arg(outQ));
}
