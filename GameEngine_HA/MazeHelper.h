#pragma once

#include <vector>
#include <string>
#include "globalThings.h"
#include "cMeshObject.h";

class MazeHelper {
public:
	MazeHelper() = default;
	~MazeHelper() = default;
	MazeHelper(std::string filename);
	std::vector<cMeshObject*> getMazeMeshesAt(glm::vec2 position, int size);
	std::vector<std::vector<char>> getMazeAt(glm::vec2 position, int size);
	glm::vec2 getRandomMazeCell();
	std::vector<std::vector<char>> maze;
	std::vector<std::vector<cMeshObject*>> mazeMeshes;

private:
};
