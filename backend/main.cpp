// backend/main.cpp
#include "mainwindow.h"
#include "Encryptor.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[]) {
    // CLI mode: secur_file_encrypt.exe encrypt <in> <out> <key>
    if (argc > 1) {
        std::string mode = argv[1];
        if (mode == "encrypt" && argc == 5) {
            std::string inFile  = argv[2];
            std::string outFile = argv[3];
            std::string key     = argv[4];
            try {
                Encryptor enc(key);
                enc.encryptFile(inFile, outFile);
                return 0;
            } catch (const std::exception &e) {
                std::cerr << "Encryption error: " << e.what() << std::endl;
                return 1;
            }
        }
        if (mode == "decrypt" && argc == 4) {
            std::string inFile  = argv[2];
            std::string outFile = argv[3];
            try {
                Encryptor enc("");
                enc.decryptFile(inFile, outFile);
                return 0;
            } catch (const std::exception &e) {
                std::cerr << "Decryption error: " << e.what() << std::endl;
                return 1;
            }
        }
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " encrypt <in> <out> <key>\n"
                  << "  " << argv[0] << " decrypt <in> <out>\n";
        return 1;
    }

    // GUI mode
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
