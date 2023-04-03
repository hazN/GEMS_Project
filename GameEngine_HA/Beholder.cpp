#include "Beholder.h"
#include <Windows.h>
#include <iostream>
#include "quaternion_utils.h"
#include <algorithm>
#include <queue>
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
			if (glm::length(target->position - position) < 0.5f)
			{
				Attack(target);
				target = nullptr;
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
			this->mesh->bUse_RGBA_colour = true;
			this->mesh->RGBA_colour = glm::vec4(0.1f, 0.7f, 0.1f, 1.f);
			Chase();
		}
		else
		{
			this->mesh->bUse_RGBA_colour = false;
			Explore();
		}
	}
	else
	{
		mesh->bUse_RGBA_colour = true;
		mesh->RGBA_colour = glm::vec4(1.f, 0.f, 0.f, 1.f);

		// Rotate and scale down
		float rotation_speed = 10.f;
		float scale_speed = 0.000001f;
		while (mesh->scaleXYZ.x > 0.05f) {
			// Rotate
			glm::quat rotateDir = q_utils::LookAt(glm::vec3(position.x, 10.f, position.y), glm::vec3(0, 1, 0));
			this->mesh->qRotation = q_utils::RotateTowards(this->mesh->qRotation, -rotateDir, 3.14f * 0.05f);

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
		pathIndex++;
		if (pathIndex >= path.size())
		{
			for (size_t i = 0; i < path.size(); i++)
			{
				delete path[i];
			}
			path.clear();
			return;
		}
		this->position + path[pathIndex]->dir;
	}
	else
	{
		// Call A* to find a new path
		Node* start = new Node;
		start->x = this->position.y;
		start->y = this->position.x;
		Node* end = new Node;
		end->x = target->position.y;
		end->y = target->position.x;
		std::vector<std::vector<char>> portion = _mazeHelper->getMazeAt(this->position, 40);
		path = AStarSearch(start, end, portion);
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

std::vector<Node*> Beholder::AStarSearch(Node* start, Node* end, std::vector<std::vector<char>> maze)
{
	// Open and closed sets
	std::priority_queue<Node*, std::vector<Node*>, CompareNode> open;
	std::map<Coord, Node*> closed;

	// Add the start node
	start->g = 0;
	start->h = getDistance(start, end);
	start->parent = nullptr;
	open.push(start);

	// Loop until end is found
	while (!open.empty())
	{
		// Pop lowest distance node
		Node* curNode = open.top();
		open.pop();

		// Check if it has reached the end
		if (curNode == end)
		{
			// Create the path for the agent to follow
			std::vector<Node*> rPath;
			while (curNode != nullptr)
			{
				rPath.push_back(curNode);
				rPath[rPath.size() - 1]->dir = glm::vec2(rPath[rPath.size() - 1]->x, rPath[rPath.size() - 1]->y) - glm::vec2(curNode->x, curNode->y);
				curNode = curNode->parent;
			}
			// Reverse the order so it's start to finish
			std::vector<Node*> path(rPath.rbegin(), rPath.rend());
			return path;
		}

		// Find cardinal neighbors on the fly
		const int dx[4] = { 0, 0, 1, -1 };
		const int dy[4] = { 1, -1, 0, 0 };
		for (int i = 0; i < 4; ++i)
		{
			// Calculate coordinates of potential neighbor
			int nx = curNode->x + dx[i];
			int ny = curNode->y + dy[i];

			// Check if neighbor is within bounds
			if (nx < 0 || nx > maze[0].size() - 1 || ny < 0 || ny > maze.size() - 1)
			{
				continue;
			}

			// Check if neighbor is a wall
			if (maze[ny][nx] == 'x')
			{
				continue;
			}

			// Create neighbor node if it hasn't been closed already
			if (closed.find(Coord{nx, ny}) != closed.end())
			{
				continue;
			}
			Node* neighbour = new Node();
			neighbour->x = nx;
			neighbour->y = ny;
			neighbour->parent = curNode;

			// Calculate distance from start and finish
			neighbour->g = curNode->g + getDistance(curNode, neighbour);
			neighbour->h = getDistance(neighbour, end);

			// Add neighbor to open set
			open.push(neighbour);
		}

		// Add the current node to the closed set
		closed.emplace(Coord{curNode->x, curNode->y}, curNode);
	}

	// Return an empty path incase there is no end node
	return std::vector<Node*>();
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