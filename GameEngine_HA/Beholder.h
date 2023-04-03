#pragma once
#include <thread>
#include <vector>
#include <glm/vec2.hpp>
#include "cMeshObject.h"
#include "MazeHelper.h"
enum eDirection {
    DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
};
struct Coord
{
    int x;
    int y;
    bool operator== (const Coord& rhs) const
    {
        if (this->x != rhs.x)
            return false;
        if (this->y != rhs.y)
            return false;
        return true;
    }
    bool operator< (const Coord& rhs) const
    {
        return this->x < rhs.x || (this->x == rhs.x && this->y < rhs.y);
    }
};
struct Node {
    int x;
    int y;
    glm::vec2 dir;
    float h;
    float g;
    int f() const {
        return h + g;
    }
    Node* parent;

    bool operator<(const Node& rhs) const {
        return this->f() < rhs.f();
    }
    std::vector<Node*> neighbours;
};

struct CompareNode
{
public:
    bool operator()(Node* a, Node* b) const
    {
        return a->f() > b->f();
    }
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
    float deltaTime = std::clock();
    float duration = 0;
    int pathIndex = 0;
    std::vector<Node*> path;
    std::vector<Node*> AStarSearch(Node* start, Node* end, std::vector<std::vector<char>> maze);
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
