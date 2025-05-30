
#include "Tree.h"
#include <iostream>

DecisionTree::DecisionTree() {
    root = nullptr;
}

DecisionTree::~DecisionTree() {
    deleteTree(root);
}

void DecisionTree::deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->yes);
    deleteTree(node->no);
    delete node;
}

void DecisionTree::buildSampleTree() {
    root = new Node("Is the file size > 100KB?");
    root->yes = new Node("Does filename start with vowel?");
    root->no = new Node("Does filename contain 'x'?");

    root->yes->yes = new Node("Leaf");
    root->yes->no = new Node("Leaf");

    root->no->yes = new Node("Leaf");
    root->no->no = new Node("Leaf");
}

std::string DecisionTree::evaluateTree() {
    std::string path = "";
    Node* current = root;

    while (current->yes && current->no) {
        std::cout << current->question << " (y/n): ";
        char ans;
        std::cin >> ans;

        if (ans == 'y' || ans == 'Y') {
            path += "1";
            current = current->yes;
        } else {
            path += "0";
            current = current->no;
        }
    }

    std::cout << "Derived binary key: " << path << std::endl;
    return path;
}
