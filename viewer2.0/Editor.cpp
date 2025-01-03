#include "Editor.hpp"

namespace {
	unsigned int convertHexa2Decimal(char c) {
		switch (c)
		{
		case '0':case '1':case '2':case '3':case '4': case '5':
		case '6':case '7': case '8': case '9':
			return c - '0';
		case 'a':
			return 10;
		case 'b':
			return 11;
		case 'c':
			return 12;
		case 'd':
			return 13;
		case 'e':
			return 14;
		case 'f':
			return 15;
		default:
			return 0;
		}
	}
}

void Editor::update(DcmTag tag, const char* newValue)
{
	std::queue<TreeNode*> q;
	q.push(tree->getRoot());
	while (!q.empty()) {
		TreeNode* currentNode = q.front();
		q.pop();
		if (currentNode->tag == tag) {
			if (currentNode->description == "Item" || strstr(currentNode->description.c_str(), "Sequence")) {
				std::cerr << "It is impossible to change the value for \'" << currentNode->description << "\'\n";
				return;
			}
			std::cout << "Replaced \'" << currentNode->value << "\' with \'" << newValue << "\'\n";
			currentNode->value = newValue;
		}
		for (auto& i : currentNode->children)
			q.push(i);
	}
}

std::unique_ptr<DcmFileFormat> Editor::getFileFormat()
{
	std::unique_ptr<DcmFileFormat> fileFormat = std::make_unique<DcmFileFormat>();
	DcmDataset* dataSet = fileFormat->getDataset();
	TreeNode* currentNode = tree->getRoot();
	std::queue<TreeNode*> q;
	for (auto& i : currentNode->children)
		q.push(i);
	while (!q.empty()) {
		currentNode = q.front();
		q.pop();
		if (currentNode->vr.getEVR() == EVR_SQ) {
			//addTreeNodeSequenceToDataset(currentNode, dataSet); 
			continue;
		}
		addTreeNodeDataToDataset(currentNode, dataSet);
		for (auto& i : currentNode->children)
			q.push(i);
	}
	fileFormat->print(COUT);
	return fileFormat;
}

void Editor::addTreeNodeSequenceToDataset(TreeNode* node, DcmItem* container) {
	for (auto& itemNode : node->children) {
		DcmItem* item = new DcmItem(itemNode->tag);
		for (auto& n : itemNode->children) {
			//[NOTE] recursive callback
			if (itemNode->vr.getEVR() == EVR_SQ)
				addTreeNodeSequenceToDataset(n, item);
			addTreeNodeDataToDataset(n, item);
		}
		/*DcmStack stack; //debug
		while (item->nextObject(stack, OFTrue).good())
			COUT << OFString(stack.card(), '>') << stack.top()->getTag() << " " << DcmVR(stack.top()->getVR()).getVRName() << OFendl;
		std::cout << '\n';*/
		OFCondition status = container->insertSequenceItem(node->tag, item);
		if (status.bad()) 
			std::cerr << "insertSequenceItem() failed for \'" << node->description << "\'\n";
		delete item;
	}
}

void Editor::addTreeNodeDataToDataset(TreeNode* node, DcmItem* dataSet) {

	DcmItem* item = dataSet;
	if (item == nullptr) {
		std::cerr << "The container of the node with description \'" << node->description << "\' was not found!\n";
		return;
	}
	switch (node->vr.getEVR())
	{
	case EVR_AE:case EVR_AS:case EVR_CS:case EVR_DA:case EVR_DS:
	case EVR_DT:case EVR_IS:case EVR_LO:case EVR_LT:case EVR_PN:
	case EVR_SH:case EVR_ST:case EVR_TM:case EVR_UC:case EVR_UI:
	case EVR_UR:case EVR_UT: {
		handleDefault(node, item);
		return;
	}
	case EVR_UN: {
		//do nothing
		return;
	}
	case EVR_OW: {
		handleOW(node, item);
		return;
	}
	case EVR_AT: {
		handleAT(item, node);
		return;
	}
	case EVR_FL:case EVR_OF: {
		handleFL_OF(node, item);
		return;
	}
	case EVR_FD:case EVR_OD: {
		handleFD_OD(node, item);
		return;
	}
	case EVR_US: {
		handleUS(node, item);
		return;
	}
	case EVR_xs: {
		(node->value.size() > 0 && node->value[0] == '-') ? handleSS(node, item) : handleUS(node, item);
		return;
	}
	case EVR_SS: {
		handleSS(node, item);
		return;
	}
	case EVR_OL:case EVR_UL: {
		handleOL_UL(node, item);
		return;
	}
	case EVR_OB: {
		handleOB(node, item);
		return;
	}
	case EVR_SL: {
		handleSL(node, item);
		return;
	}
	case EVR_ox: {
		(node->value.size() > 2 && node->value.at(2) == '/') ? handleOB(node, item) : handleOW(node, item);
		return;
	}
	case EVR_SQ:
		return;
	case EVR_na:
		return;
	case EVR_lt:case EVR_up:case EVR_item:case EVR_metainfo:case EVR_dataset:
	case EVR_fileFormat:case EVR_dicomDir:case EVR_dirRecord:case EVR_pixelSQ:
	case EVR_pixelItem:case EVR_UNKNOWN:case EVR_OverlayData:
	case EVR_UNKNOWN2B:
		std::cerr << "Unhandled!\n";
		return;
	default:
		std::cerr << "default switch branch called!\n";
		return;
	}
}

void Editor::handleDefault(TreeNode* node, DcmItem* item)
{
	OFString value = node->value;
	OFCondition status = item->putAndInsertOFStringArray(node->tag, value);
	if (status.bad())
		std::cerr << "putAndInsertOFStringArray() failed for \'" << node->description << "\'\n";
}

void Editor::handleOW(TreeNode* node, DcmItem* item)
{
	Uint16* data = new Uint16[node->value.size() / 4];
	unsigned long c = 0;
	for (size_t i = 0; i < node->value.size(); i += 5) {
		if (c > node->value.size() / 4 - 1) {
			std::cerr << "Buffer overflow!\n";
			break;
		}
		data[c++] =
			convertHexa2Decimal(node->value[i]) * 16 * 16 * 16 +
			convertHexa2Decimal(node->value[i + 1]) * 16 * 16 +
			convertHexa2Decimal(node->value[i + 2]) * 16 +
			convertHexa2Decimal(node->value[i + 3]);
	}
	OFCondition status = item->putAndInsertUint16Array(node->tag, data, c);
	if (status.bad())
		std::cerr << "putAndInsertUint16Array() failed for \'" << node->description << "\'\n";
	delete[] data;
}

void Editor::handleAT(DcmItem* item, TreeNode* node)
{
	OFCondition status = item->putAndInsertString(node->tag, node->value.c_str());
	if (status.bad())
		std::cerr << "putAndInsertString() failed for \'" << node->description << "\'\n";
}

void Editor::handleFL_OF(TreeNode* node, DcmItem* item)
{
	Float32* data = new Float32[100]{ 0 };
	unsigned long c = 0;
	Float32 power1 = 10.0f;
	Float32 power2 = 1.0f;
	bool isAfterDecimalPoint{ false };
	bool isNegative{ false };
	for (auto& i : node->value) {
		if (c > 99) {
			std::cerr << "Buffer overflow!\n";
			break;
		}
		if (i == '-') {
			isNegative = true;
			continue;
		}
		if (isAfterDecimalPoint)
			power2 /= 10.0f;
		if (i == '.') {
			isAfterDecimalPoint = true;
			power1 = 1.0f;
			continue;
		}
		if (static_cast<int>(i) > static_cast<int>('9') && static_cast<int>('0') < static_cast<int>(i)) {
			data[c] *= (isNegative) ? -1.0f : 1.0f;
			isAfterDecimalPoint = false;
			isNegative = false;
			c++;
			continue;
		}
		data[c] = data[c] * power1 + (static_cast<Float32>(i) - static_cast<Float32>('0')) * power2;
	}
	OFCondition status = item->putAndInsertFloat32Array(node->tag, data, c + 1);
	if (status.bad())
		std::cerr << "putAndInsertFloat32Array() failed for \'" << node->description << "\'\n";
	delete[] data;
}

void Editor::handleFD_OD(TreeNode* node, DcmItem* item)
{
	Float64* data = new Float64[100]{ 0 };
	unsigned long c = 0;
	double power1 = 10.0;
	double power2 = 1.0;
	bool isAfterDecimalPoint{ false };
	bool isNegative{ false };
	for (auto& i : node->value) {
		if (c > 99) {
			std::cerr << "Buffer overflow!\n";
			break;
		}
		if (i == '-') {
			isNegative = true;
			continue;
		}
		if (isAfterDecimalPoint)
			power2 /= 10.0;
		if (i == '.') {
			isAfterDecimalPoint = true;
			power1 = 1.0;
			continue;
		}
		if (static_cast<int>(i) > static_cast<int>('9') && static_cast<int>('0') < static_cast<int>(i)) {
			data[c] *= (isNegative) ? -1.0 : 1.0;
			isAfterDecimalPoint = false;
			isNegative = false;
			c++;
			continue;
		}
		data[c] = data[c] * power1 + (static_cast<Float64>(i) - static_cast<Float64>('0')) * power2;
	}
	OFCondition status = item->putAndInsertFloat64Array(node->tag, data, c + 1);
	if (status.bad())
		std::cerr << "putAndInsertFloat64Array() failed for \'" << node->description << "\'\n";
	delete[] data;
}

void Editor::handleSS(TreeNode* node, DcmItem* item)
{
	Sint16* data = new Sint16[100]{ 0 };
	unsigned long c = 0;
	bool isNegative{ false };
	for (auto& i : node->value) {
		if (c > 99) {
			std::cerr << "Buffer overflow!\n";
			break;
		}
		if (i == '-') {
			isNegative = true;
			continue;
		}
		if (static_cast<int>(i) > static_cast<int>('9') && static_cast<int>('0') < static_cast<int>(i)) {
			data[c] *= (isNegative) ? -1 : 1;
			isNegative = false;
			c++;
			continue;
		}
		data[c] = data[c] * 10 + (static_cast<Sint16>(i) - static_cast<Sint16>('0'));
	}
	OFCondition status = item->putAndInsertSint16Array(node->tag, data, c + 1);
	if (status.bad())
		std::cerr << "putAndInsertSint16Array() failed for \'" << node->description << "\'\n";
	delete[] data;
}

void Editor::handleOL_UL(TreeNode* node, DcmItem* item)
{
	Uint32* data = new Uint32[100]{ 0 };
	unsigned long c = 0;
	for (auto& i : node->value) {
		if (c > 99) {
			std::cerr << "Buffer overflow!\n";
			break;
		}
		if (static_cast<int>(i) > static_cast<int>('9') && static_cast<int>('0') < static_cast<int>(i)) {
			c++;
			continue;
		}
		data[c] = data[c] * 10 + (static_cast<Uint32>(i) - static_cast<Uint32>('0'));
	}
	std::cout << node->value << ' ' << data[0] << ' ' << node->description << ' ' << c << '\n';
	OFCondition status = item->putAndInsertUint32Array(node->tag, data, c + 1);
	if (status.bad())
		std::cerr << "putAndInsertUint32Array() failed for \'" << node->description << "\'\n";
	delete[] data;
}

void Editor::handleOB(TreeNode* node, DcmItem* item)
{
	Uint8* data = new Uint8[100'000]{ 0 };
	unsigned long c = 0;
	for (auto& i : node->value) {
		if (c > 99'999) {
			std::cerr << "Buffer overflow!\n";
			break;
		}
		if (static_cast<int>(i) > static_cast<int>('9') && static_cast<int>('0') < static_cast<int>(i)) {
			c++;
			continue;
		}
		data[c] = data[c] * 10 + (static_cast<Uint8>(i) - static_cast<Uint8>('0'));
	}
	OFCondition status = item->putAndInsertUint8Array(node->tag, data, c + 1);
	if (status.bad())
		std::cerr << "putAndInsertUint8Array() failed for \'" << node->description << "\'\n";
	delete[] data;
}

void Editor::handleSL(TreeNode* node, DcmItem* item)
{
	int x = 0;
	bool isNegative{ false };
	for (auto& i : node->value) {
		if (x == '-') {
			isNegative = true;
			continue;
		}
		x = x * 10 + (static_cast<int>(i) - static_cast<int>('0'));
	}
	x *= (isNegative) ? -1 : 1;
	OFCondition status = item->putAndInsertSint32(node->tag, x);
	if (status.bad())
		std::cerr << "putAndInsertSint32() failed for \'" << node->description << "\'\n";
}

void Editor::handleUS(TreeNode* node, DcmItem* item)
{
	Uint16* data = new Uint16[100]{ 0 };
	unsigned long c = 0;
	for (auto& i : node->value) {
		if (c > 99) {
			std::cerr << "Buffer overflow!\n";
			break;
		}
		if (static_cast<int>(i) > static_cast<int>('9') && static_cast<int>('0') < static_cast<int>(i)) {
			c++;
			continue;
		}
		data[c] = data[c] * 10 + (static_cast<Uint16>(i) - static_cast<Uint16>('0'));
	}
	OFCondition status = item->putAndInsertUint16Array(node->tag, data, c + 1);
	if (status.bad())
		std::cerr << "putAndInsertSint16Array() failed for \'" << node->description << "\'\n";
	delete[] data;
}
