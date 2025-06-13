#include "Encryptor.h"
#include <fstream>
#include <iostream>
#include <filesystem>

Encryptor::Encryptor(const std::string& k) : key(k) {}

void Encryptor::encryptFile(const std::string& inPath,
                            const std::string& outPath) const
{
    std::ifstream in(inPath, std::ios::binary);
    std::ofstream out(outPath, std::ios::binary);
    if (!in || !out) {
        std::cerr << "File error: " << inPath << " / " << outPath << "\n";
        return;
    }

    // 1) Embed original extension
    std::filesystem::path p(inPath);
    std::string extension = p.extension().string();
    uint32_t    extLen    = static_cast<uint32_t>(extension.size());
    out.write(reinterpret_cast<char*>(&extLen), sizeof(extLen));
    out.write(extension.data(), extLen);

    // 2) Embed the key
    uint32_t keyLen = static_cast<uint32_t>(key.size());
    out.write(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
    out.write(key.data(), keyLen);

    // 3) XOR‐encrypt payload
    char   ch;
    size_t i = 0;
    while (in.get(ch)) {
        char e = ch ^ key[i % key.size()];
        out.put(e);
        ++i;
    }
}

void Encryptor::decryptFile(const std::string& inPath,
                            const std::string& outPath) const
{
    std::ifstream in(inPath, std::ios::binary);
    if (!in) {
        std::cerr << "File error: cannot open " << inPath << "\n";
        return;
    }

    // 1) Read embedded extension
    uint32_t extLen = 0;
    in.read(reinterpret_cast<char*>(&extLen), sizeof(extLen));
    std::string extension(extLen, '\0');
    in.read(&extension[0], extLen);

    // 2) Read embedded key
    uint32_t keyLen = 0;
    in.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
    std::string key_read(keyLen, '\0');
    in.read(&key_read[0], keyLen);

    // 3) Decrypt payload
    std::ofstream out(outPath, std::ios::binary);
    if (!out) {
        std::cerr << "File error: cannot create " << outPath << "\n";
        return;
    }
    char   ch;
    size_t i = 0;
    while (in.get(ch)) {
        char d = ch ^ key_read[i % key_read.size()];
        out.put(d);
        ++i;
    }
}
