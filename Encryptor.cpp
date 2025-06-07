#include "Encryptor.h"
#include <fstream>
#include <iostream>

Encryptor::Encryptor(const std::string& k) : key(k) {}

void Encryptor::encryptFile(const std::string& inPath,
                            const std::string& outPath) const
{
    std::ifstream in(inPath, std::ios::binary);
    std::ofstream out(outPath, std::ios::binary);
    if (!in || !out) {
        std::cerr << "File error: " << inPath << " or " << outPath << "\n";
        return;
    }
    char ch;
    size_t i = 0;
    while (in.get(ch)) {
        out.put(ch ^ key[i % key.size()]);
        ++i;
    }
}

void Encryptor::decryptFile(const std::string& inPath,
                            const std::string& outPath) const
{
    // XOR is symmetric
    encryptFile(inPath, outPath);
}
