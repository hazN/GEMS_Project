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
            //if (c == 'o')
            //{
            //    cMeshObject* pFloor = new cMeshObject();
            //    pFloor->meshName = "Floor";
            //    pFloor->friendlyName = "Floor: " + std::to_string(iRow) + ", " + std::to_string(i);
            //    pFloor->bUse_RGBA_colour = false;
            //    pFloor->position = glm::vec3(iCol, 0.f, iRow);
            //    pFloor->setRotationFromEuler(glm::vec3(0));
            //    pFloor->isWireframe = false;
            //    pFloor->SetUniformScale(0.002f);
            //    pFloor->textures[0] = "Dungeons_2_Texture_01_A.bmp";
            //    pFloor->textureRatios[0] = 1.0f;
            //    meshRow.push_back(pFloor);
            //}
            //else 
            //{
            //    cMeshObject* pFloor = new cMeshObject();
            //    pFloor->meshName = "Floor";
            //    pFloor->friendlyName = "Floor: " + std::to_string(iRow) + ", " + std::to_string(i);
            //    pFloor->bUse_RGBA_colour = false;
            //    pFloor->position = glm::vec3(iCol, 1.f, iRow);
            //    pFloor->setRotationFromEuler(glm::vec3(0));
            //    pFloor->isWireframe = false;
            //    pFloor->SetUniformScale(0.002f);
            //    pFloor->textures[0] = "Dungeons_2_Texture_01_A.bmp";
            //    pFloor->textureRatios[0] = 1.0f;
            //    
            //    cMeshObject* pWallLeft = new cMeshObject();
            //    pWallLeft->meshName = "Wall";
            //    pWallLeft->friendlyName = "WallLeft";
            //    pWallLeft->bUse_RGBA_colour = false;
            //    pWallLeft->position = glm::vec3(-1.f, 1.f, 0.0f);
            //    pWallLeft->setRotationFromEuler(glm::vec3(0.0f, glm::radians(180.f), 0.0f));
            //    pWallLeft->isWireframe = false;
            //    pWallLeft->SetUniformScale(0.002f);
            //    pWallLeft->textures[0] = "Dungeons_2_Texture_01_A.bmp";
            //    pWallLeft->textureRatios[0] = 1.0f;

            //    cMeshObject* pWallRight = new cMeshObject();
            //    pWallRight->meshName = "Wall";
            //    pWallRight->friendlyName = "WallRight";
            //    pWallRight->bUse_RGBA_colour = false;
            //    pWallRight->position = glm::vec3(0.f, 1.f, -1.0f);
            //    pWallRight->setRotationFromEuler(glm::vec3(0.0f, glm::radians(0.f), 0.0f));
            //    pWallRight->isWireframe = false;
            //    pWallRight->SetUniformScale(0.002f);
            //    pWallRight->textures[0] = "Dungeons_2_Texture_01_A.bmp";
            //    pWallRight->textureRatios[0] = 1.0f;

            //    cMeshObject* pWallTop = new cMeshObject();
            //    pWallTop->meshName = "Wall";
            //    pWallTop->friendlyName = "WallTop";
            //    pWallTop->bUse_RGBA_colour = false;
            //    pWallTop->position = glm::vec3(0.f, 1.f, 0.f);
            //    pWallTop->setRotationFromEuler(glm::vec3(0.0f, glm::radians(270.f), 0.0f));
            //    pWallTop->isWireframe = false;
            //    pWallTop->SetUniformScale(0.002f);
            //    pWallTop->textures[0] = "Dungeons_2_Texture_01_A.bmp";
            //    pWallTop->textureRatios[0] = 1.0f;

            //    cMeshObject* pWallBottom = new cMeshObject();
            //    pWallBottom->meshName = "Wall";
            //    pWallBottom->friendlyName = "WallTop";
            //    pWallBottom->bUse_RGBA_colour = false;
            //    pWallBottom->position = glm::vec3(-1.f, 1.f, -1.f);
            //    pWallBottom->setRotationFromEuler(glm::vec3(0.0f, glm::radians(90.f), 0.0f));
            //    pWallBottom->isWireframe = false;
            //    pWallBottom->SetUniformScale(0.002f);
            //    pWallBottom->textures[0] = "Dungeons_2_Texture_01_A.bmp";
            //    pWallBottom->textureRatios[0] = 1.0f;

            //    pFloor->vecChildMeshes.push_back(pWallLeft);
            //    pFloor->vecChildMeshes.push_back(pWallRight);
            //    pFloor->vecChildMeshes.push_back(pWallTop);
            //    pFloor->vecChildMeshes.push_back(pWallBottom);
            //    meshRow.push_back(pFloor);
            //}
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