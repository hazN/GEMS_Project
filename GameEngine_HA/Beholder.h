#pragma once
#include <thread>
#include <vector>
#include <glm/vec2.hpp>

class Beholder {
public:
    Beholder(int id, glm::vec2 position);

    void start();
    void stop(); 
    void join(); 

    void Explore();
    void Attack(Beholder* other); 

    int id; 
    glm::vec2 position;
    bool isAlive; 
    std::thread m_thread; 
    std::vector<Beholder*>* allBeholders; 
};