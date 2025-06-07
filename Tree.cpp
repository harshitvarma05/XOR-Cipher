#include "Tree.h"
#include <QFileInfo>

DecisionTree::DecisionTree() : root(nullptr) {}
DecisionTree::~DecisionTree() { deleteTree(root); }

void DecisionTree::deleteTree(Node* n) {
  if (!n) return;
  deleteTree(n->yes);
  deleteTree(n->no);
  delete n;
}

void DecisionTree::buildFileBasedTree() {
  // Tear down old tree
  deleteTree(root);

  // Root: file size > 100 KB?
  root = new Node(
    "Is the file size > 100 KB?",
    [](const QString& f){
      QFileInfo fi(f);
      return fi.exists() && fi.size() > 100*1024;
    }
  );

  // Yes-branch: filename starts with vowel?
  root->yes = new Node(
    "Does the filename start with a vowel?",
    [](const QString& f){
      QFileInfo fi(f);
      if (!fi.exists()) return false;
      QChar c = fi.fileName().at(0).toLower();
      return QString("aeiou").contains(c);
    }
  );

  // No-branch: filename contains 'x'?
  root->no = new Node(
    "Does the filename contain the letter 'x'?",
    [](const QString& f){
      QFileInfo fi(f);
      return fi.exists()
          && fi.fileName().contains('x', Qt::CaseInsensitive);
    }
  );

  // Leaves
  root->yes->yes = new Node("Leaf", [](auto){ return true; });
  root->yes->no  = new Node("Leaf", [](auto){ return true; });
  root->no->yes  = new Node("Leaf", [](auto){ return true; });
  root->no->no   = new Node("Leaf", [](auto){ return true; });
}
