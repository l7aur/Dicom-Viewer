#include "Reader.hpp"
#include "Editor.hpp"
#include "Writer.hpp"

const char* paths[] = {
	"dicoms/public/test1.dcm",
	"dicoms/public/test2.dcm",
	"dicoms/public/test3.dcm",
	"dicoms/public/test4.dcm",
	"C:/Users/user/Desktop/output.dcm" // default path to save file 
};
const char* defaultPath = "C:/Users/user/Desktop/output.dcm";

static Reader initReader();
static Tree* buildTree(Reader& r, CONTAINER_TYPE c);
static void printFileStructure(Tree* miset, Tree* dset);
static char doYouWantToExit(Tree* t1, Tree* t2);
static std::string fetchNewValue();
static std::pair<std::vector<DcmTag>, std::vector<int>> fetchPathToNode();
static void printPathToAffectedNode(std::pair<std::vector<DcmTag>, std::vector<int>> path);
static void cleanup(Tree* t1, Tree* t2);
static void exit(Tree* t1, Tree* t2);

int main()
{
	while (1) {
		Reader reader = initReader();
		if (reader.openFile() < 0) {
			std::cerr << "Unable to open file\n";
			continue;
		}

		Tree* datasetTree = buildTree(reader, CONTAINER_TYPE::DATASET);
		Tree* metainfoTree = buildTree(reader, CONTAINER_TYPE::METAINFO);

		// [NOTE] Print the tree to check if the values have been retrieved
		printFileStructure(metainfoTree, datasetTree);

		Editor editor(metainfoTree, datasetTree);

		// [NOTE] Stay in edit mode as long as needed
		bool isFirstEditLoopEnter{false};
		while (1) {
			std::cout << ((!isFirstEditLoopEnter) ? "\nEnter edit mode? [y/n/q]\n" : "\nContinue in edit mode? [y/n/q]\n");
			isFirstEditLoopEnter = true;
			char c = doYouWantToExit(datasetTree, metainfoTree);
			if (c != 'y')
				break;

			std::pair<std::vector<DcmTag>, std::vector<int>> pathToNode = fetchPathToNode();
			editor.update(pathToNode.first, pathToNode.second, fetchNewValue().c_str());
			printPathToAffectedNode(pathToNode);
		}

		std::cout << "\nSave modifications to default path? [y/n/q]\n";
		char c = doYouWantToExit(datasetTree, metainfoTree);
		if (c != 'y')
			continue;
		Writer writer;
		writer.save(std::move(editor.getFileFormat()), defaultPath);

		// [NOTE] release acquired resources
		// [NOTE] end of one pass in normal execution
		cleanup(datasetTree, metainfoTree);
	}
	return 0;
}

static Reader initReader() {
	char index = 0;
	for (int i = 0; i < sizeof(paths) / sizeof(const char*); ++i)
		std::cout << i << ". " << paths[i] << '\n';
	std::cout << "\nInsert a number between 0 and " << min(9, sizeof(paths) / sizeof(const char*) - 1) << " or q: ";
	index = doYouWantToExit(nullptr, nullptr);
	if (index < '0' || index > '9')
		return 0;
	return Reader(paths[(unsigned int)index - '0']);
}

static Tree* buildTree(Reader& r, CONTAINER_TYPE c) {
	Tree* t = nullptr;
	switch (c)
	{
	case DATASET:
		t = r.loadDataset();
		r.retrieveDataset(t);
		return t;
	case METAINFO:
		t = r.loadMetainfo();
		r.retrieveMetainfo(t);
		return t;
	}
	std::cerr << "Tree was not initialized!\n";
	return t;
}

/**
 * If the node is at depth 1, just read its tag and expect a 0x0 (end of sequence).
 * If the node is part of a sequence, read the sequence tag, read the item tag and the item number, i.e. 0 -> n.
 * (The item number input is required to differentiate between items because they all have the same tag.)
 * \return a pair of arrays, the first one holding a tag path, and the second one holding the item numbers for their corresponding sequence.
 */
static std::pair<std::vector<DcmTag>, std::vector<int>> fetchPathToNode() {
	std::cout << "Enter the tag sequence id of node to be edited (i.e. 0x0123 0x0FFF, 0xfffe 0xe0000, ...) and enter 0x0 when done\n";
	Uint16 g, e;
	std::vector<DcmTag> tags;
	std::vector<int> itemNos;
	while (std::cin >> std::hex >> g) {
		if (g == 0)
			break;
		std::cin >> std::hex >> e;
		tags.push_back(DcmTag(g, e));
		if (g == 0xfffe && e == 0xe000) {
			std::cout << "Which item (0, 1, 2, 3, ...): ";
			int itemNo{ 0 };
			std::cin >> itemNo;
			itemNos.push_back(itemNo);
		}
	}
	return { tags, itemNos };
}

static std::string fetchNewValue() {
	std::cout << "Enter the new value of the node:\n";
	std::cin.get();
	std::string value(200, '\0');
	std::cin.getline(&value[0], value.size());
	return value;
}

static void printPathToAffectedNode(std::pair<std::vector<DcmTag>, std::vector<int>> path) {
	std::vector<DcmTag> tags = path.first;
	std::vector<int> itemNos = path.second;
	std::cout << "Path to affected node: ";
	int i = 0;
	for (auto& tag : tags) {
		std::cout << tag.getTagName() << ' ';
		if (tag == DCM_ItemTag)
			std::cout << "Item #" << itemNos.at(i) << ' ';
		std::cout  << "-> ";
	}
	std::cout << '\n';
}

static void printFileStructure(Tree* miset, Tree* dset)
{
	std::cout << "----------------METAINFORMATION----------------\n";
	miset->preOrderTraversalPrint();
	std::cout << "--------------METAINFORMATION END--------------\n";
	std::cout << "--------------------DATASET--------------------\n";
	dset->preOrderTraversalPrint();
	std::cout << "------------------DATASET END------------------\n";
}

static char doYouWantToExit(Tree* t1, Tree* t2) {
	char c = 'n';
	std::cin >> c;
	if (c == 'q')
		exit(t1, t2);
	return c;
}

static void cleanup(Tree* t1, Tree* t2)
{
	if(t1)
		delete t1;
	if(t2)
		delete t2;
}

static void exit(Tree* t1, Tree* t2)
{
	cleanup(t1, t2);
	std::exit(0);
}