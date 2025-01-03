#pragma once

#include <stack>

#include "Tree.hpp"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

class Reader {
public:
	Reader() {};
	Reader(const char * path);
	~Reader() {};
	int fopen();
	Tree* loadMetainfo();
	Tree* loadDataset();
	void dump(std::ofstream& fout);
	void retrieveValues(Tree* tree);
	DcmFileFormat getFileFormat() const { return this->fileFormat; };
	DcmMetaInfo* getMetaInfo() const { return this->metainfo; };
private:
	const char* filePath{ nullptr };
	DcmFileFormat fileFormat;
	DcmDataset* dataset{ nullptr };
	DcmMetaInfo* metainfo{ nullptr };

	Tree* load(DcmItem* container);
	void retrieveValue(Tree* tree, TreeNode* node);
	DcmItem* findContainerOfNode(Tree* tree, TreeNode* node);
};
