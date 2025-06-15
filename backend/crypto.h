#pragma once
#include <string>

double hybridEncrypt(
    const std::string& inPath,
    const std::string& outPath,
    const std::string& pubKeyPath
);

double hybridDecrypt(
    const std::string& inPath,
    const std::string& outPath,
    const std::string& privKeyPath
);
