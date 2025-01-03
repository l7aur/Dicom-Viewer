#include "Tree.hpp"

Tree::Tree()
{
	root = new TreeNode();
}

Tree::~Tree()
{
	makeEmpty();
}

void Tree::makeEmpty()
{
	makeEmpty(root);
}

void Tree::insert(TreeNode*& parent, TreeNode*& newChild)
{
	parent->children.push_back(newChild);
	newChild->parent = parent;
	size++;
}

TreeNode* Tree::findNode(DcmTag searchedTag) const
{
	std::queue<TreeNode*> q{};
	q.push(root);
	while (!q.empty()) {
		TreeNode* currentNode = q.front();
		q.pop();
		if (currentNode->tag == searchedTag)
			return currentNode;
		for (auto& i : currentNode->children)
			q.push(i);
	}
	return nullptr;
}

/**
 * Traverses and stores the tree starting from the given node over the 'parent' link until the root.
 * 
 * \param n The node in the tree we need the path from 
 * \return 
 */
std::vector<TreeNode*> Tree::findPathToRootFrom(TreeNode* n)
{
	std::vector<TreeNode*> path;
	path.push_back(n);
	while (n->parent != nullptr) {
		n = n->parent;
		path.push_back(n);
	}
	return path;
}

void Tree::preOrderTraversalPrint() const
{
	preOrderTraversalPrint(root);
}

void Tree::preOrderTraversalPrint(TreeNode* n, int level) const
{
	if (n == nullptr)
		return;
	for (int i = 0; i < level; i++, std::cout << ' ');
	std::cout << *n << '\n';
	for (auto& i : n->children)
		preOrderTraversalPrint(i, level + 1);
}

void Tree::makeEmpty(TreeNode*& n)
{
	if (n != nullptr) {
		for (auto& i : n->children)
			makeEmpty(i);
		delete n;
	}
	n = nullptr;
}
