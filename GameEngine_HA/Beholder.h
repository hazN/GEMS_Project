#pragma once
#include <thread>
#include <vector>
#include <glm/vec2.hpp>
#include "cMeshObject.h"
#include "MazeHelper.h"
enum eDirection {
    DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
};
enum eState {
    NO_STATE, EXPLORING, CHASING 
};

struct Node {
    int x;
    int y;
    Node* parent;
    int depth;
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
    eState state;
    eDirection direction;
    cMeshObject* mesh;
    glm::vec2 position;
    glm::vec2 prevPosition;
    bool isAlive; 
    bool exitThread; 
    std::vector<Beholder*> *allBeholders; 
    MazeHelper* _mazeHelper;
    Beholder* target;
    float deltaTime = std::clock();
    float duration = 0;
    int pathIndex = 0;
    std::vector<Node*> path;
    std::vector<Node*> DepthFirstSearch(Node* startNode, Node* goalNode, std::vector<std::vector<char>> maze, int maxDepth);
   // std::vector<Node*> AStarSearch(Node* start, Node* end, std::vector<std::vector<char>> maze);
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
