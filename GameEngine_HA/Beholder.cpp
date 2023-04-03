#include "Beholder.h"
#include <Windows.h>
#include <iostream>
#include "quaternion_utils.h"
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
	glm::vec2 nextPos;
	std::vector<eDirection> possibleDirections;

	// Check the potential directions
	if (target->position.x > position.x)
	{
		if (_mazeHelper->maze[position.x + 1][position.y] != 'x')
			possibleDirections.push_back(DIR_DOWN);
	}
	else if (target->position.x < position.x)
	{
		if (_mazeHelper->maze[position.x - 1][position.y] != 'x')
			possibleDirections.push_back(DIR_UP);
	}
	if (target->position.y > position.y)
	{
		if (_mazeHelper->maze[position.x][position.y + 1] != 'x')
			possibleDirections.push_back(DIR_RIGHT);
	}
	else if (target->position.y < position.y)
	{
		if (_mazeHelper->maze[position.x][position.y - 1] != 'x')
			possibleDirections.push_back(DIR_LEFT);
	}
	// If there are no valid directions, just explore instead
	if (possibleDirections.empty())
	{
		Explore();
		return;
	}
	// Otherwise move in the direction closest to the target
	else
	{
		float minDistance = -1.f;
		for (eDirection dir : possibleDirections)
		{
			switch (dir)
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
			float distance = glm::length(nextPos - target->position);
			if (minDistance < 0.f || distance < minDistance)
			{
				minDistance = distance;
				direction = dir;
			}
		}
		position = nextPos;
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

