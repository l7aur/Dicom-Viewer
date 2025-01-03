#pragma once

#include <vector>
#include <queue>
#include <iomanip>
#include <windows.h>

#include "TreeNode.hpp"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

class Tree {
public:
	Tree();
	~Tree();
	void makeEmpty();
	void insert(TreeNode*& parent, TreeNode*& newChild);
	TreeNode* findNode(DcmTag searchedTag) const;
	std::vector<TreeNode*> findPathToRootFrom(TreeNode* n);
	void preOrderTraversalPrint() const;
	int getSize() const { return this->size; };
	TreeNode*& getRoot() { return this->root; };
private:
	TreeNode* root;
	int size{ 0 };
	void preOrderTraversalPrint(TreeNode* n, int level = 0) const;
	void makeEmpty(TreeNode*& n);
};
