#include "Beholder.h"
#include <Windows.h>
#include <iostream>
#include "quaternion_utils.h"
#include <algorithm>
#include <queue>
#include <stack>
#include <set>
#include <unordered_set>
eDirection turnAround(eDirection direction);

Beholder::Beholder(int id, glm::vec2 position, MazeHelper* _mazeHelper)
{
	this->id = id;
	this->position = position;
	this->_mazeHelper = _mazeHelper;
	this->isAlive = true;
	this->exitThread = false;
	this->target = nullptr;
	this->direction = DIR_NONE;
	this->mesh = new cMeshObject();
	{
		this->mesh->meshName = "Beholder";
		this->mesh->friendlyName = "Beholder1";
		this->mesh->bUse_RGBA_colour = false;      // Use file colours    pTerrain->RGBA_colour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		this->mesh->specular_colour_and_power = glm::vec4(1.0f, 0.0f, 0.0f, 1000.0f);
		this->mesh->position = glm::vec3(position.y - 0.5, 0.5f, position.x - 0.5);
		this->mesh->setRotationFromEuler(glm::vec3(0));
		this->mesh->isWireframe = false;
		this->mesh->SetUniformScale(0.2f);
		this->mesh->textures[0] = "Beholder_Base_color.bmp";
		this->mesh->textureRatios[0] = 1.0f;
		this->mesh->textureRatios[1] = 1.0f;
		this->mesh->textureRatios[2] = 1.0f;
		this->mesh->textureRatios[3] = 1.0f;
	}
}

void Beholder::Update()
{
	if (isAlive)
	{
		// Attack if close enough
		if (target != nullptr)
		{
			if (glm::length(target->mesh->position - mesh->position) < 0.5f)
			{
				Attack(target);
				target = nullptr;
				lastTarget = nullptr;
				return;
			}
		}
		LineOfSight();
		glm::vec3 targetPos = glm::vec3(position.y - 0.5f, 0.5f, position.x - 0.5f);
		if (this->mesh->position != targetPos)
		{
			// Move towards the target position
			glm::vec3 dir = glm::normalize(targetPos - this->mesh->position);
			this->mesh->position += dir * 0.1f; // adjust the speed here
			glm::quat rotateDir = q_utils::LookAt(-dir, glm::vec3(0, 1, 0));
			this->mesh->qRotation = q_utils::RotateTowards(this->mesh->qRotation, rotateDir, 3.14f * 0.05f);

			// Force position once distance is close enough
			if (glm::length(this->mesh->position - targetPos) < 0.1f)
				this->mesh->position = targetPos;
		}
		else if (target != nullptr)
		{
			//this->mesh->bUse_RGBA_colour = true;
			//this->mesh->RGBA_colour = glm::vec4(0.1f, 0.7f, 0.1f, 1.f);
			state = CHASING;
			Chase();
		}
		else
		{
			//this->mesh->bUse_RGBA_colour = false;
			state = EXPLORING;
			Explore();
		}
	}
	else
	{
		mesh->bUse_RGBA_colour = true;
		mesh->RGBA_colour = glm::vec4(1.f, 0.f, 0.f, 1.f);

		// Rotate and scale down
		float rotation_speed = 10.f;
		float scale_speed = 0.0000001f;
		while (mesh->scaleXYZ.x > 0.05f) {
			// Rotate
			glm::quat rotateDir = q_utils::LookAt(glm::vec3(mesh->position.x, 50.f, mesh->position.y), glm::vec3(0, 1, 0));
			this->mesh->qRotation = q_utils::RotateTowards(this->mesh->qRotation, -rotateDir, 3.14f * 0.5f);

			// Scale down
			float current_scale = mesh->scaleXYZ.x;
			mesh->SetUniformScale(current_scale - scale_speed);
		}
	}
	if (mesh->scaleXYZ.x < 0.05f)
	{
		this->exitThread = true;
	}
	Sleep(50);
}


void Beholder::Explore()
{
	glm::vec2 nextPos;
	std::vector<eDirection> possibleDirections;

	// Check the potential directions
	if (position.x > 0)
	{
		if (direction != DIR_DOWN && _mazeHelper->maze[position.x - 1][position.y] == 'o')
			possibleDirections.push_back(DIR_UP);
	}
	if (position.x < _mazeHelper->maze.size() - 1)
	{
		if (direction != DIR_UP && _mazeHelper->maze[position.x + 1][position.y] == 'o')
			possibleDirections.push_back(DIR_DOWN);
	}
	if (position.y > 0)
	{
		if (direction != DIR_RIGHT && _mazeHelper->maze[position.x][position.y - 1] == 'o')
			possibleDirections.push_back(DIR_LEFT);
	}
	if (position.y < _mazeHelper->maze[0].size() - 1)
	{
		if (direction != DIR_LEFT && _mazeHelper->maze[position.x][position.y + 1] == 'o')
			possibleDirections.push_back(DIR_RIGHT);
	}
	// If its empty then turn around
	if (possibleDirections.empty())
		direction = turnAround(direction);
	// Otherwise move in a random direction
	else
	{
		direction = possibleDirections[rand() % possibleDirections.size()];
		switch (direction)
		{
		case DIR_UP:
			nextPos = position + glm::vec2(-1.f, 0.f);
			break;
		case DIR_DOWN:
			nextPos = position + glm::vec2(1.f, 0.f);
			break;
		case DIR_LEFT:
			nextPos = position + glm::vec2(0.f, -1.f);
			break;
		case DIR_RIGHT:
			nextPos = position + glm::vec2(0.f, 1.f);
			break;
		default:
			break;
		}
		position = nextPos;
	}
}

void Beholder::LineOfSight()
{
	// Get the beholder's direction
	glm::vec2 dir;
	switch (direction)
	{
	case DIR_UP:
		dir = glm::vec2(-1.f, 0.f);
		break;
	case DIR_DOWN:
		dir = glm::vec2(1.f, 0.f);
		break;
	case DIR_LEFT:
		dir = glm::vec2(0.f, -1.f);
		break;
	case DIR_RIGHT:
		dir = glm::vec2(0.f, 1.f);
		break;
	case DIR_NONE:
		return;
	default:
		break;
	}

	glm::vec2 positionToCheck = position;
	// Check each open tile in the direction until a wall is hit
	while (positionToCheck.x >= 0 && positionToCheck.x < _mazeHelper->maze.size() &&
		positionToCheck.y >= 0 && positionToCheck.y < _mazeHelper->maze[0].size())
	{
		// Check if the current tile is a wall
		if (_mazeHelper->maze[positionToCheck.x][positionToCheck.y] == 'x')
			break;

		// Check if a beholder is in the open tile
		for (Beholder* beholder : *allBeholders)
		{
			if (beholder != nullptr)
			{
				if (beholder != this && beholder->position == positionToCheck && beholder->isAlive)
				{
					// Change target to the beholder if it's in line of sight
					target = beholder;
					return;
				}
			}
		}

		// Set the new position
		positionToCheck += dir;
	}
}

void Beholder::Chase()
{
	// Check if there is a path to follow
	if (!path.empty())
	{
		this->position = glm::vec2(path[0]->x, path[0]->y);
		delete path[0];
		path.erase(path.begin());
	}
	else
	{
		// No path exists, find a new one
		Node* start = new Node;
		start->x = this->position.x;
		start->y = this->position.y;
		Node* end = new Node;
		if (target != nullptr)
		{
			end->x = target->position.x;
			end->y = target->position.y;
		}
		path = DepthFirstSearch(start, end, _mazeHelper->maze, 40);
	}
}


void Beholder::Attack(Beholder* other)
{
	std::cout << "Beholder " << this->id << " has killed " << other->id << std::endl;
	if (this->isAlive)
	{
		other->isAlive = false;
	}
}
// Manhattan distance
int getDistance(Node* a, Node* b) {
	int dx = abs(a->x - b->x);
	int dy = abs(a->y - b->y);
	return dx + dy;
}

std::vector<Node*> Beholder::DepthFirstSearch(Node* start, Node* end, std::vector<std::vector<char>> maze, int maxDepth)
{
	std::stack<Node*> open;
	std::unordered_set<std::string> closed;
	std::vector<Node*> _path;

	open.push(start);

	while (!open.empty())
	{
		// Pop the top node from frontier stack
		Node* currentNode = open.top();
		open.pop();

		// Check if the current node is the goal node
		if (currentNode->x == end->x && currentNode->y == end->y)
		{
			// Reconstruct the path from start to goal node
			if (currentNode->parent == nullptr)
			{
				_path.push_back(currentNode);
				return _path;
			}
			while (currentNode != nullptr)
			{
				if (currentNode == start)
					break;
				_path.push_back(currentNode);
				currentNode = currentNode->parent;
			}
			std::reverse(_path.begin(), _path.end());
			return _path;
		}

		std::vector<eDirection> possibleDirections;

		// Check the potential directions
		if (currentNode->x > 0)
		{
			if (direction != DIR_DOWN && _mazeHelper->maze[currentNode->x - 1][currentNode->y] == 'o')
				possibleDirections.push_back(DIR_UP);
		}
		if (currentNode->x < _mazeHelper->maze.size() - 1)
		{
			if (direction != DIR_UP && _mazeHelper->maze[currentNode->x + 1][currentNode->y] == 'o')
				possibleDirections.push_back(DIR_DOWN);
		}
		if (currentNode->y > 0)
		{
			if (direction != DIR_RIGHT && _mazeHelper->maze[currentNode->x][currentNode->y - 1] == 'o')
				possibleDirections.push_back(DIR_LEFT);
		}
		if (currentNode->y < _mazeHelper->maze[0].size() - 1)
		{
			if (direction != DIR_LEFT && _mazeHelper->maze[currentNode->x][currentNode->y + 1] == 'o')
				possibleDirections.push_back(DIR_RIGHT);
		}
		// Generate child nodes
		for (int direction : possibleDirections)
		{
			glm::vec2 dir;
			switch (direction)
			{
			case DIR_UP:
				dir = glm::vec2(-1.f, 0.f);
				break;
			case DIR_DOWN:
				dir = glm::vec2(1.f, 0.f);
				break;
			case DIR_LEFT:
				dir = glm::vec2(0.f, -1.f);
				break;
			case DIR_RIGHT:
				dir = glm::vec2(0.f, 1.f);
				break;
			default:
				break;
			}
			int newX = currentNode->x + (int)dir.x;
			int newY = currentNode->y + (int)dir.y;

			// Check if the child node is inside the maze and not a wall
			if (newX >= 0 && newX < maze.size() && newY >= 0 && newY < maze[newX].size() && maze[newX][newY] != '#')
			{
				std::string nodeString = std::to_string(newX) + "," + std::to_string(newY);

				// Check if the child node has not been explored yet
				if (closed.find(nodeString) == closed.end())
				{
					// Create new child node
					Node* childNode = new Node;
					childNode->x = newX;
					childNode->y = newY;
					childNode->parent = currentNode;

					// Add child node to frontier stack
					open.push(childNode);

					// Add child node to explored set
					closed.insert(nodeString);
				}
			}
		}
	}

	// No path found
	////std::cout << "Failed to find a path" << std::endl;
	// Explore to avoid getting stuck
	Explore();
	//_path = DepthFirstSearch(start, end, maze, 40);
	return _path;
}

eDirection turnAround(eDirection direction)
{
	switch (direction)
	{
	case DIR_UP:
		direction = DIR_DOWN;
		break;
	case DIR_DOWN:
		direction = DIR_UP;
		break;
	case DIR_LEFT:
		direction = DIR_RIGHT;
		break;
	case DIR_RIGHT:
		direction = DIR_LEFT;
		break;
	default:
		break;
	}
	return direction;
}