#ifndef TREE_H
#define TREE_H

#include <string>

struct Node {
    std::string question;
    Node* yes;
    Node* no;
    Node(const std::string& q) : question(q), yes(nullptr), no(nullptr) {}
};

class DecisionTree {
    Node* root;
    void deleteTree(Node* node);
public:
    DecisionTree();
    ~DecisionTree();
    void buildSampleTree();
    Node* getRoot() const { return root; }
};
#endif // TREE_H
