// backend/main.cpp

#include <QApplication>
#include <QElapsedTimer>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdio>

// GUI
#include "mainwindow.h"

// XOR
#include "Encryptor.h"

// AES+RSA hybrid
#include "Crypto.h"

// OpenSSL for key generation
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>

// ─────────────────────────────────────────────────────────────
// Generate a 2048-bit RSA keypair and write to PEM files
static void generateKeypairFiles(const std::string& pubPath,
                                 const std::string& privPath)
{
    // 1) Create the RSA key
    RSA* rsa = RSA_new();
    BIGNUM* bn = BN_new();
    if (!BN_set_word(bn, RSA_F4))
        throw std::runtime_error("BN_set_word failed");
    if (!RSA_generate_key_ex(rsa, 2048, bn, nullptr))
        throw std::runtime_error("RSA_generate_key_ex failed");
    BN_free(bn);

    // 2) Wrap RSA in an EVP_PKEY
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!EVP_PKEY_assign_RSA(pkey, rsa))
        throw std::runtime_error("EVP_PKEY_assign_RSA failed");
    // Note: pkey now owns rsa, so do not call RSA_free(rsa)

    // 3) Write public key (SubjectPublicKeyInfo)
    {
        FILE* fp = fopen(pubPath.c_str(), "wb");
        if (!fp) throw std::runtime_error("Cannot open pub.pem for writing");
        if (!PEM_write_PUBKEY(fp, pkey))
            throw std::runtime_error("PEM_write_PUBKEY failed");
        fclose(fp);
    }
    // 4) Write private key (PKCS#1)
    {
        FILE* fp = fopen(privPath.c_str(), "wb");
        if (!fp) throw std::runtime_error("Cannot open priv.pem for writing");
        if (!PEM_write_PrivateKey(fp, pkey, nullptr, nullptr, 0, nullptr, nullptr))
            throw std::runtime_error("PEM_write_PrivateKey failed");
        fclose(fp);
    }

    EVP_PKEY_free(pkey);
}
// ─────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    // CLI mode
    if (argc > 1) {
        std::string mode = argv[1];

        // XOR encrypt: encrypt <in> <out> <key>
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

        // XOR decrypt: decrypt <in> <out>
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

        // AES+RSA encrypt: aesrsa-encrypt <in> <out> <pub.pem>
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

        // AES+RSA decrypt: aesrsa-decrypt <in> <out> <priv.pem>
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

        // RSA key‐generation: aesrsa-keygen <pub.pem> <priv.pem>
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

        // Fallback usage
        std::cerr
            << "Usage:\n"
            << "  " << argv[0] << " encrypt <in> <out> <key>\n"
            << "  " << argv[0] << " decrypt <in> <out>\n"
            << "  " << argv[0] << " aesrsa-encrypt <in> <out> <pub.pem>\n"
            << "  " << argv[0] << " aesrsa-decrypt <in> <out> <priv.pem>\n"
            << "  " << argv[0] << " aesrsa-keygen <pub.pem> <priv.pem>\n";
        return 1;
    }

    // Launch Qt GUI
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
