#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <string>

class Encryptor {
    std::string key;
public:
    explicit Encryptor(const std::string& k);
    void encryptFile(const std::string& inPath,
                     const std::string& outPath) const;
    void decryptFile(const std::string& inPath,
                     const std::string& outPath) const;
};

#endif // ENCRYPTOR_H
