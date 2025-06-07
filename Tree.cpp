#include "Tree.h"

DecisionTree::DecisionTree() : root(nullptr) {}
DecisionTree::~DecisionTree() { deleteTree(root); }

void DecisionTree::deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->yes);
    deleteTree(node->no);
    delete node;
}

void DecisionTree::buildSampleTree() {
    root = new Node("Is the file size > 100KB?");
    root->yes = new Node("Does filename start with a vowel?");
    root->no  = new Node("Does filename contain 'x'?");

    root->yes->yes = new Node("Leaf");
    root->yes->no  = new Node("Leaf");
    root->no->yes  = new Node("Leaf");
    root->no->no   = new Node("Leaf");
}
