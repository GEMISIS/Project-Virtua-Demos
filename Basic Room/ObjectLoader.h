#ifndef _OBJECT_LOADER_
#define _OBJECT_LOADER_

#include "tiny_obj_loader.h"

using namespace tinyobj;

class ObjectModel
{
public:
	ObjectModel(const char* filename);
	void Draw();
private:
	unsigned int totalShapes;
	std::vector<unsigned int> verticesHandles;
	std::vector<unsigned int> uvHandles;
	std::vector<unsigned int> normalsHandles;
	std::vector<unsigned int> indicesHandles;
	std::vector<unsigned int> meshSizes;
};

#endif