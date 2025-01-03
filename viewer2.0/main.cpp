#include "Reader.hpp"
#include "Editor.hpp"
#include "Writer.hpp"
#include "main.h"
#include "fstream"

int main()
{
	std::ofstream f("./dump_res.txt");
	while (1) {
		const char* paths[] = {
			"./test3.dcm",
			"./more_items_in_sq.dcm",
			"./online_test.dcm",
			"./gura.dcm",
			"C:/Users/user/Desktop/output.dcm"
		};
		char pathI = 0;

		//[NOTE] Load a file
		for (int i = 0; i < sizeof(paths) / sizeof(const char*); ++i)
			std::cout << i << ". " << paths[i] << '\n';
		std::cout << "\nInsert a number between 0 and " << sizeof(paths) / sizeof(const char*) - 1 << " or q: ";
		std::cin >> pathI;
		if (pathI == 'q' || pathI < '0' || pathI > '9')
			return 0;

		Reader fin(paths[pathI - '0']);
		if (fin.fopen() < 0) {
			std::cerr << "Unable to open file\n";
			continue;
		}

		//fin.dump(f); /*debug*/

		//[NOTE] Convert DcmFileFormat into Tree
		Tree* t = fin.loadDataset();
		Tree* tt = fin.loadMetainfo();
		fin.retrieveValues(t);
		fin.retrieveValues(tt);

		//[NOTE] Print the tree to check if the values have been retrieved
		printFileStructure(tt, t);

		Editor editor(t);

		char c = 'n';
		bool isFirstEditLoopEnter{ false };
		while (1) {
			std::cout << ((!isFirstEditLoopEnter) ? "\nEnter edit mode? [y/n/q]\n" : "\nContinue in edit mode? [y/n]\n");
			std::cin >> c;
			if (c == 'q') {
				delete t;
				delete tt;
				return 0;
			}
			if (c != 'y')
				break;
			std::cout << "Enter the tag id of node to be edited (i.e. 0123 0FFF, q to quit): ";
			std::string id;
			Uint16 g, e;
			std::cin >>std::hex >> g >> e;
			if (g == 'q') {
				delete t;
				delete tt;
				return 0;
			}
			std::cout << "Enter the new value of the node: ";
			std::string value;
			std::cin >> value;
			DcmTag tag(g, e);
			std::cout << tag.getTagName() <<'\n';
			editor.update(tag, value.c_str());
		}

		//[NOTE] Check updated tree structure
		printFileStructure(tt, t);
		std::cout << "\nSave modifications to default path? [y/n/q]\n";
		std::cin >> c;
		if (c == 'q') {
			delete t;
			delete tt;
			return 0;
		}
		if (c != 'y')
			continue;
		std::string defaultPath = "C:/Users/user/Desktop/output.dcm";
		Writer writer;
		writer.save(std::move(editor.getFileFormat()), fin.getMetaInfo(), defaultPath.c_str());
		delete t;
		delete tt;
	}
	f.close();
	return 0;
}

void printFileStructure(Tree* miset, Tree* dset)
{
	std::cout << "----------------METAINFORMATION----------------\n";
	miset->preOrderTraversalPrint();
	std::cout << "--------------METAINFORMATION END--------------\n";
	std::cout << "--------------------DATASET--------------------\n";
	dset->preOrderTraversalPrint();
	std::cout << "------------------DATASET END------------------\n";
}

/**
 * Metoda de citit value in metainfo -> findAndGetOFStringArray() functioneaza pe VR in DcmDataset, dar nu si pe VR-uri identice in DcmMetainfo (?).
 * Crearea de dataset pare ca merge perfect, dar la .saveFile() in writer crapa. Credeam ca sunt generate de compression method, i.e. jpeg, dar crapa si online_test
 */





