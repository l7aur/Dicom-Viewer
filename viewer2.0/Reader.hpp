#pragma once

#include <stack>

#include "Tree.hpp"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

enum CONTAINER_TYPE {
	DATASET,
	METAINFO
};

class Reader {
public:
	Reader() {};
	Reader(const char * path);
	~Reader() {};
	int openFile();
	Tree* loadMetainfo() { return load(metainfo); };
	Tree* loadDataset() { return load(dataset); };
	void dump(std::ofstream& fout) { fileFormat.print(fout); };
	void retrieveDataset(Tree* tree) { retrieveValues(tree, DATASET); };
	void retrieveMetainfo(Tree* tree) { retrieveValues(tree, METAINFO); };
	DcmFileFormat getFileFormat() const { return this->fileFormat; };
	DcmMetaInfo* getMetaInfo() const { return this->metainfo; };
private:
	const char* filePath{ nullptr };
	DcmFileFormat fileFormat;
	DcmDataset* dataset{ nullptr };
	DcmMetaInfo* metainfo{ nullptr };

	Tree* load(DcmItem* container);
	void retrieveValues(Tree* tree, CONTAINER_TYPE c);
	void retrieveValue(Tree* tree, TreeNode* node, DcmItem* c);
	DcmItem* findContainerOfNode(Tree* tree, TreeNode* node, DcmItem* initialContainer);
	DcmItem* getContainer(CONTAINER_TYPE c);
};
