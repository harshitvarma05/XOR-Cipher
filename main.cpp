
#include <iostream>
#include "Tree.h"
#include "Encryptor.h"
using namespace std;

int main() {
    cout << "Secure File Encryption using Decision Path" << endl;

    // Step 1: Build Decision Tree
    DecisionTree tree;
    tree.buildSampleTree();

    // Step 2: Derive encryption key from decision path
    string binaryKey = tree.evaluateTree();

    // Step 3: Encrypt the file
    Encryptor encryptor(binaryKey);
    encryptor.encryptFile("untitled document.docx", "encrypted.dat");

    // Step 4: Decrypt the file
    encryptor.decryptFile("encrypted.dat", "dec.docx");

    return 0;
}
