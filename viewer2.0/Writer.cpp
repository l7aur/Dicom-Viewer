#include "Writer.hpp"

void Writer::save(std::unique_ptr<DcmFileFormat> ff, DcmMetaInfo* mi, const char* path)
{
	if (ff == nullptr) {
		std::cerr << "DcmFileFormat does not exist!\n";
		return;
	}
	if (path == nullptr || path == "") {
		std::cerr << "Invalid path!\n";
		return;
	}
	ff->saveFile(path, EXS_LittleEndianExplicit);
}
