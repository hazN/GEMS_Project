#include "MazeHelper.h"
#include <fstream>
#include <iostream>
#include <thread>

MazeHelper::MazeHelper(std::string filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::vector<std::vector<char>> tempMaze;
    std::vector<std::vector<cMeshObject*>> tempMeshes;

    std::string line;
    int iRow = 0;
    while (std::getline(file, line)) {
        iRow++;
        std::vector<char> row;
        std::vector<cMeshObject*> meshRow;
        int iCol = 0;
        for (int i = 0; i < line.size(); i++) {
            char c = line[i];
            row.push_back(c);
            iCol++;
        }
        tempMaze.push_back(row);
        tempMeshes.push_back(meshRow);
    }
    this->maze = tempMaze;
    this->mazeMeshes = tempMeshes;

    file.close();
}

std::vector<cMeshObject*> MazeHelper::getMazeMeshesAt(glm::vec2 position, int size)
{
    std::vector<cMeshObject*> meshes;
    int halfSize = size / 2;

    // Swap coordinates since we want y as rows and x as columns
    for (int y = position.y - halfSize; y <= position.y + halfSize; y++)
    {
        for (int x = position.x - halfSize; x <= position.x + halfSize; x++)
        {
            if (x < 0 || x >= maze.size() || y < 0 || y >= maze[x].size())
                continue;
            else
                meshes.push_back(mazeMeshes[y][x]);
        }
    }
    return meshes;
}

void getMazeAtThread(std::vector<std::vector<char>>& mazeAt, const std::vector<std::vector<char>>& maze, const glm::vec2& position, int size)
{
    int halfSize = size / 2;

    // Swap coordinates since we want y as rows and x as columns
    for (int y = position.y - halfSize; y <= position.y + halfSize; y++)
    {
        for (int x = position.x - halfSize; x <= position.x + halfSize; x++)
        {
            if (x < 0 || x >= maze.size() || y < 0 || y >= maze[x].size())
                continue;
            else
                mazeAt[y - position.y + halfSize][x - position.x + halfSize] = maze[y][x];
        }
    }
}

std::vector<std::vector<char>> MazeHelper::getMazeAt(glm::vec2 position, int size)
{
    std::vector<std::vector<char>> mazeAt(size + 2, std::vector<char>(size + 2, '\0'));

    // Create a thread and pass the necessary arguments
    std::thread t(getMazeAtThread, std::ref(mazeAt), std::cref(maze), position, size);

    // Wait for the thread to finish and join it
    t.join();

    return mazeAt;
}
glm::vec2 MazeHelper::getRandomMazeCell()
{
    int y, x;
    do {
        y = rand() % maze.size();
        x = rand() % maze[y].size();
    } while (maze[y][x] != 'o');

    return glm::vec2(y, x);
}
