#pragma once

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

#include "Tree.hpp"

class Writer {
public:
	Writer() {};
	~Writer() {};
	void save(std::unique_ptr<DcmFileFormat> ff, DcmMetaInfo* mi, const char* path);
private:
};
