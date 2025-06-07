#ifndef TREE_H
#define TREE_H

#include <QString>
#include <functional>

struct Node {
    QString question;
    std::function<bool(const QString&)> check;
    Node* yes;
    Node* no;
    Node(const QString& q,
         std::function<bool(const QString&)> c)
      : question(q), check(c), yes(nullptr), no(nullptr) {}
};

class DecisionTree {
    Node* root;
    void deleteTree(Node* n);
public:
    DecisionTree();
    ~DecisionTree();
    // Build questions based on the selected file
    void buildFileBasedTree();
    Node* getRoot() const { return root; }
};

#endif // TREE_H
