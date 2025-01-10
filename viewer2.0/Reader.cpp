#include "Reader.hpp"

Reader::Reader(const char* path)
	: filePath(path)
{
}

int Reader::openFile()
{
	OFCondition status = fileFormat.loadFile(filePath);
	if (status.bad()) {
		std::cerr << status.text() << '\n';
		return -1;
	}
	metainfo = fileFormat.getMetaInfo();
	dataset = fileFormat.getDataset();
	return 0;
}

Tree* Reader::load(DcmItem* container)
{
	Tree* t = new Tree();
	TreeNode* prev = t->getRoot();
	std::stack<TreeNode*> nodeStack;

	//[NOTE] the iterating base (default) level is 2
	int currentLevel = 2;
	nodeStack.push(t->getRoot());
	DcmStack s;
	while (container->nextObject(s, OFTrue).good()) {
		DcmObject* current = s.top();
		int level = s.card();
		
		//[NOTE] create node and insert in tree
		DcmTag tag = current->getTag();
		DcmVR vr = current->getVR();
		unsigned long vm = current->getVM();
		Uint32 length = current->getLength();
		OFString description = tag.getTagName();
		OFString value = "init::value_not_retrieved";
		TreeNode* node = new TreeNode(tag, vr, vm, length, description, value);

		//[NOTE] update node stack and level if required
		if (level == currentLevel) {
			t->insert(nodeStack.top(), node);
		}
		else if (level > currentLevel) {
			t->insert(prev, node);
			currentLevel++;
			nodeStack.push(prev);
		}
		else {
			while (nodeStack.size() > 1 && currentLevel > level) {
				nodeStack.pop();
				currentLevel--;
			}
			t->insert(nodeStack.top(), node);
		}
		prev = node;
	}
	return t;
}

void Reader::retrieveValues(Tree* tree, CONTAINER_TYPE c)
{
	DcmItem* container = getContainer(c);
	std::stack<TreeNode*> nodes{};
	for (auto& i : tree->getRoot()->children)
		nodes.push(i);
	while (!nodes.empty()) {
		TreeNode* currentNode = nodes.top();
		nodes.pop();
		retrieveValue(tree, currentNode, container);
		for (auto& i : currentNode->children)
			nodes.push(i);
	}
}

void Reader::retrieveValue(Tree* tree, TreeNode* node, DcmItem* c)
{
	DcmItem* item = findContainerOfNode(tree, node, c);
	if (item == nullptr) {
		std::cerr << "The container of the node with description \'" << node->description << "\' was not found!\n";
		return;
	}

	switch (node->vr.getEVR())
	{
	case EVR_AE:case EVR_AS:case EVR_AT:case EVR_CS:case EVR_DA:case EVR_DS:
	case EVR_DT:case EVR_FL:case EVR_FD:case EVR_IS:case EVR_LO:case EVR_LT:
	case EVR_OB:case EVR_OD:case EVR_OF:case EVR_OL:case EVR_OW:case EVR_PN:
	case EVR_SH:case EVR_SL:case EVR_SS:case EVR_ST:case EVR_TM:case EVR_UC:
	case EVR_UI:case EVR_UR:case EVR_US:case EVR_UT:case EVR_UN:case EVR_UL:
	case EVR_ox:case EVR_xs: {
		OFString value{};
		OFCondition status = item->findAndGetOFStringArray(node->tag, value);
		if (status.good())
			node->value = value;
		else
			std::cerr << "findAndGetOFStringAray() failed for \'" << node->description << "\'\n";
		return;
	}
	case EVR_SQ:case EVR_na:
		node->value = "";
		return;
	case EVR_lt:case EVR_up:case EVR_item:case EVR_metainfo:
	case EVR_dataset:case EVR_fileFormat:case EVR_dicomDir:case EVR_dirRecord:
	case EVR_pixelSQ:case EVR_pixelItem:case EVR_UNKNOWN:case EVR_OverlayData:
	case EVR_UNKNOWN2B:
		node->value = "!!not handled!!";
		break;
	default:
		break;
	}
}

DcmItem* Reader::findContainerOfNode(Tree* tree, TreeNode* node, DcmItem* initialContainer) {
	std::vector<TreeNode*> path = tree->findPathToRootFrom(node);
	if (path.empty()) {
		std::cerr << "No path found from root to \'" << node->description << '\'\n';
		return nullptr;
	}

	//[NOTE] remove virtual node root
	path.pop_back();

	//[NOTE] the node is a direct child of the root, the container is the dataset itself
	if (path.size() <= 1)
		return initialContainer;

	DcmItem* item = initialContainer;

	//[ASSUMPTION] sequence nodes are paired with item nodes
	//[UNTESTED] [POSSIBLE CRASH CAUSE] sequences that contain sequence delimiters (unspecified length)
	auto i = path.rbegin();
	TreeNode* sqNode = *(i++);
	TreeNode* itNode = *(i++);

	//[NOTE] findIndexOfChild() must return valid index because the node was stored in the path to the root
	OFCondition status = initialContainer->findAndGetSequenceItem(sqNode->tag, item, sqNode->findIndexOfChild(itNode));
	while (i != path.rend() && i + 1 != path.rend()) {
		sqNode = *(i++);
		itNode = *(i++);
		status = item->findAndGetSequenceItem(sqNode->tag, item, sqNode->findIndexOfChild(itNode));
		if (status.bad()) {
			std::cerr << "The sequence parsing process failed for \'" << node->description << "\'\n";
			return nullptr;
		}
	}
	return item;
}

DcmItem* Reader::getContainer(CONTAINER_TYPE c)
{
	switch (c)
	{
	case DATASET:
		return dataset;
	case METAINFO:
		return metainfo;
	}
	return nullptr;
}