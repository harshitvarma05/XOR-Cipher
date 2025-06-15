

#include <QApplication>
#include <QElapsedTimer>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdio>

#include "mainwindow.h"

#include "Encryptor.h"

#include "Crypto.h"

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>

static void generateKeypairFiles(const std::string& pubPath,
                                 const std::string& privPath)
{
   
    RSA* rsa = RSA_new();
    BIGNUM* bn = BN_new();
    if (!BN_set_word(bn, RSA_F4))
        throw std::runtime_error("BN_set_word failed");
    if (!RSA_generate_key_ex(rsa, 2048, bn, nullptr))
        throw std::runtime_error("RSA_generate_key_ex failed");
    BN_free(bn);

   
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!EVP_PKEY_assign_RSA(pkey, rsa))
        throw std::runtime_error("EVP_PKEY_assign_RSA failed");
   

   
    {
        FILE* fp = fopen(pubPath.c_str(), "wb");
        if (!fp) throw std::runtime_error("Cannot open pub.pem for writing");
        if (!PEM_write_PUBKEY(fp, pkey))
            throw std::runtime_error("PEM_write_PUBKEY failed");
        fclose(fp);
    }
   
    {
        FILE* fp = fopen(privPath.c_str(), "wb");
        if (!fp) throw std::runtime_error("Cannot open priv.pem for writing");
        if (!PEM_write_PrivateKey(fp, pkey, nullptr, nullptr, 0, nullptr, nullptr))
            throw std::runtime_error("PEM_write_PrivateKey failed");
        fclose(fp);
    }

    EVP_PKEY_free(pkey);
}

int main(int argc, char* argv[]) {
   
    if (argc > 1) {
        std::string mode = argv[1];

       
        if (mode == "encrypt" && argc == 5) {
            std::string inFile  = argv[2];
            std::string outFile = argv[3];
            std::string key     = argv[4];
            try {
                QElapsedTimer t; t.start();
                Encryptor enc(key);
                enc.encryptFile(inFile, outFile);
                std::cout << t.elapsed();
                return 0;
            } catch (const std::exception &e) {
                std::cerr << "Encrypt error: " << e.what();
                return 1;
            }
        }

       
        if (mode == "decrypt" && argc == 4) {
            std::string inFile  = argv[2];
            std::string outFile = argv[3];
            try {
                QElapsedTimer t; t.start();
                Encryptor dec("");
                dec.decryptFile(inFile, outFile);
                std::cout << t.elapsed();
                return 0;
            } catch (const std::exception &e) {
                std::cerr << "Decrypt error: " << e.what();
                return 1;
            }
        }

       
        if (mode == "aesrsa-encrypt" && argc == 5) {
            std::string inFile  = argv[2];
            std::string outFile = argv[3];
            std::string pubPem  = argv[4];
            try {
                double ms = hybridEncrypt(inFile, outFile, pubPem);
                std::cout << ms;
                return 0;
            } catch (const std::exception &e) {
                std::cerr << "AES+RSA encrypt error: " << e.what();
                return 1;
            }
        }

       
        if (mode == "aesrsa-decrypt" && argc == 5) {
            std::string inFile   = argv[2];
            std::string outFile  = argv[3];
            std::string privPem  = argv[4];
            try {
                double ms = hybridDecrypt(inFile, outFile, privPem);
                std::cout << ms;
                return 0;
            } catch (const std::exception &e) {
                std::cerr << "AES+RSA decrypt error: " << e.what();
                return 1;
            }
        }

       
        if (mode == "aesrsa-keygen" && argc == 4) {
            std::string pubPem  = argv[2];
            std::string privPem = argv[3];
            try {
                QElapsedTimer t; t.start();
                generateKeypairFiles(pubPem, privPem);
                std::cout << t.elapsed();
                return 0;
            } catch (const std::exception &e) {
                std::cerr << "Keygen error: " << e.what();
                return 1;
            }
        }

       
        std::cerr
            << "Usage:\n"
            << "  " << argv[0] << " encrypt <in> <out> <key>\n"
            << "  " << argv[0] << " decrypt <in> <out>\n"
            << "  " << argv[0] << " aesrsa-encrypt <in> <out> <pub.pem>\n"
            << "  " << argv[0] << " aesrsa-decrypt <in> <out> <priv.pem>\n"
            << "  " << argv[0] << " aesrsa-keygen <pub.pem> <priv.pem>\n";
        return 1;
    }

   
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
