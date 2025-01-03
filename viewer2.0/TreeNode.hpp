#include <vector>

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

struct TreeNode {
	std::vector<TreeNode*> children{ };
	TreeNode* parent{ nullptr };
	const DcmTag tag{};
	const DcmVR vr{};
	const unsigned long vm{ 0 };
	const Uint32 length{ 0 };
	const OFString description{ "not assigned" };
	OFString value{ "root" };

	TreeNode() {};
	TreeNode(DcmTag t, DcmVR r, unsigned long m, Uint32 l, OFString d, OFString v);
	unsigned long findIndexOfChild(TreeNode* child);
	friend std::ostream& operator<<(std::ostream& os, const TreeNode& node);
};