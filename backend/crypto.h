#pragma once
#include <string>

// Encrypt 'inPath' → 'outPath' with AES-256-CBC + RSA-OAEP (public key at 'pubKeyPath')
// Returns elapsed time in milliseconds.
double hybridEncrypt(
    const std::string& inPath,
    const std::string& outPath,
    const std::string& pubKeyPath
);

// Decrypt 'inPath' → 'outPath' with RSA-OAEP + AES-256-CBC (private key at 'privKeyPath')
// Returns elapsed time in milliseconds.
double hybridDecrypt(
    const std::string& inPath,
    const std::string& outPath,
    const std::string& privKeyPath
);
