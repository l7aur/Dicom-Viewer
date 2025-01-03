#include "TreeNode.hpp"

TreeNode::TreeNode(DcmTag t, DcmVR r, unsigned long m, Uint32 l, OFString d, OFString v)
	: tag(t), vr(r), vm(m), length(l), description(d), value(v)
{
}

unsigned long TreeNode::findIndexOfChild(TreeNode* child)
{
	unsigned long index = 0;
	for (auto i : this->children) {
		if (i == child)
			return index;
		index++;
	}
	return -1;
}

std::ostream& operator<<(std::ostream& os, const TreeNode& node)
{
	os << std::setw(12) << node.tag
		<< std::setw(1) << "  "
		<< std::setw(2) << node.vr.getVRName()
		<< std::setw(1) << "  "
		<< std::setw(5) << node.vm
		<< std::setw(1) << "  "
		<< std::setw(5) << node.length
		<< std::setw(1) << "\t\t"
		<< std::setw(40) << node.description
		<< std::setw(1) << "\t";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	os << std::setw(15) << ((node.value.size() > 150) ? node.value.substr(0, 140) + "..." : node.value)
		<< std::setw(1);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	return os;
}