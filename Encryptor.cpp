
#include "Encryptor.h"
#include <fstream>
#include <iostream>

Encryptor::Encryptor(std::string k) : key(k) {}

void Encryptor::encryptFile(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream input(inputPath, std::ios::binary);
    std::ofstream output(outputPath, std::ios::binary);

    if (!input || !output) {
        std::cerr << "File error during encryption!" << std::endl;
        return;
    }

    char ch;
    size_t i = 0;
    while (input.get(ch)) {
        char encrypted = ch ^ key[i % key.size()];
        output.put(encrypted);
        i++;
    }

    input.close();
    output.close();
    std::cout << "Encryption complete." << std::endl;
}

void Encryptor::decryptFile(const std::string& inputPath, const std::string& outputPath) {
    encryptFile(inputPath, outputPath); // XOR is symmetric
    std::cout << "Decryption complete." << std::endl;
}
