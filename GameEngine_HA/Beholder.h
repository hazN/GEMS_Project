#pragma once
#include <thread>
#include <vector>
#include <glm/vec2.hpp>
#include "cMeshObject.h"
#include "MazeHelper.h"
enum eDirection {
    DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
};
class Beholder {
public:
    Beholder(int id, glm::vec2 position, MazeHelper* _mazeHelper);

    void Update();
    void Explore();
    void LineOfSight();
    void Chase();
    void Attack(Beholder* other); 

    int id; 
    eDirection direction;
    cMeshObject* mesh;
    glm::vec2 position;
    bool isAlive; 
    bool exitThread; 
    std::vector<Beholder*> *allBeholders; 
    MazeHelper* _mazeHelper;
    Beholder* target;
};

struct sBeholderThreadData
{
    sBeholderThreadData()
    {
        pTheBeholder = NULL;
        bExitThread = false;
        bSuspendThread = false;
        // Pause for one frame at 60Hz (16 ms give or take)
        suspendTime_ms = 16;
    }
    Beholder* pTheBeholder;
    // Setting to true exits thread
    bool bExitThread;
    // Setting to true will stop the update
    bool bSuspendThread;
    unsigned int suspendTime_ms;
};
