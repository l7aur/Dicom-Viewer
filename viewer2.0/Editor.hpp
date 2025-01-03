#pragma once

#include <stack>
#include <string.h>
#include <memory>

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

#include "Tree.hpp"

class Editor {
public:
	Editor() {};
	Editor(Tree* t) : tree(t) {};
	~Editor() {};
	void update(DcmTag tag, const char* newValue);
	std::unique_ptr<DcmFileFormat> getFileFormat();
private:
	void addTreeNodeDataToDataset(TreeNode* node, DcmItem* dataSet);
	void handleDefault(TreeNode* node, DcmItem* item);
	void handleOW(TreeNode* node, DcmItem* item);
	void handleAT(DcmItem* item, TreeNode* node);
	void handleFL_OF(TreeNode* node, DcmItem* item);
	void handleFD_OD(TreeNode* node, DcmItem* item);
	void handleSS(TreeNode* node, DcmItem* item);
	void handleOL_UL(TreeNode* node, DcmItem* item);
	void handleOB(TreeNode* node, DcmItem* item);
	void handleSL(TreeNode* node, DcmItem* item);
	void handleUS(TreeNode* node, DcmItem* item);
	void addTreeNodeSequenceToDataset(TreeNode* node, DcmItem* dataSet);
	Tree* tree{ nullptr };
};
