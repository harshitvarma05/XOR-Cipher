
#ifndef TREE_H
#define TREE_H

#include <string>

struct Node {
    std::string question;
    Node* yes;
    Node* no;

    Node(std::string q) : question(q), yes(nullptr), no(nullptr) {}
};

class DecisionTree {
private:
    Node* root;
    void deleteTree(Node* node);

public:
    DecisionTree();
    ~DecisionTree();
    void buildSampleTree();
    std::string evaluateTree();
};

#endif
