
#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <string>

class Encryptor {
private:
    std::string key;
public:
    Encryptor(std::string k);
    void encryptFile(const std::string& inputPath, const std::string& outputPath);
    void decryptFile(const std::string& inputPath, const std::string& outputPath);
};

#endif
