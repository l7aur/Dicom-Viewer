#pragma once

#include <stack>
#include <string.h>
#include <memory>

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

#include "Tree.hpp"

class Editor {
public:
	Editor(Tree* miTree, Tree* dTree) { datasetTree = dTree; metainfoTree = miTree; };
	~Editor() {};
	void update(std::vector<DcmTag> tags, std::vector<int> itemNos, const char* newValue);
	std::unique_ptr<DcmFileFormat> getFileFormat();
private:
	void addTreeNodeSequenceToDataset(TreeNode* node, DcmItem* container);
	void addTreeNodeDataToDataset(TreeNode* node, DcmItem* container);
	void handleDefault(TreeNode* node, DcmItem* item);
	void handleOW(TreeNode* node, DcmItem* item);
	void handleAT(TreeNode* node, DcmItem* item);
	void handleFL_OF(TreeNode* node, DcmItem* item);
	void handleFD_OD(TreeNode* node, DcmItem* item);
	void handleSS(TreeNode* node, DcmItem* item);
	void handleOL_UL(TreeNode* node, DcmItem* item);
	void handleOB(TreeNode* node, DcmItem* item);
	void handleSL(TreeNode* node, DcmItem* item);
	void handleUS(TreeNode* node, DcmItem* item);
	Tree* datasetTree{ nullptr };
	Tree* metainfoTree{ nullptr };
};
