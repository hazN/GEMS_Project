#pragma once

#include <vector>
#include <string>
#include "globalThings.h"
class cMeshObject;

class MazeHelper {
public:
	MazeHelper(std::string filename);
	std::vector<cMeshObject*> getMazeMeshesAt(glm::vec2 position, int size);
	std::vector<std::vector<char>> maze;
	std::vector<std::vector<cMeshObject*>> mazeMeshes;

private:
};
