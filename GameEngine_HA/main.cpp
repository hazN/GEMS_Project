﻿//#include <glad/glad.h>
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>
#include "globalOpenGL.h"
#include "cLuaBrain.h"
#include "Animation.h"
//#include "linmath.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3        (x,y,z)
#include <glm/vec4.hpp> // glm::vec4        (x,y,z,w)
#include <glm/mat4x4.hpp> // glm::mat4
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>  // File streaming thing (like cin, etc.)
#include <sstream>  // "string builder" type thing

// Some STL (Standard Template Library) things
#include <vector>           // aka a "smart array"

#include "AIBrain.h"
#include "CarAnimations.h"
#include "globalThings.h"
#include <iterator>
#include "cShaderManager.h"
#include "cVAOManager/cVAOManager.h"
#include "cLightHelper.h"
#include "cVAOManager/c3DModelFileLoader.h"
#include "GUI.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "cBasicTextureManager.h"
#include "GenerateDungeon.h"
#include "MazeHelper.h"#include <Windows.h>
#include <assert.h>
#include "Beholder.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <assert.h>
glm::vec3 g_cameraEye = glm::vec3(0.0, 15, -100.0f);
glm::vec3 g_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
cBasicTextureManager* g_pTextureManager = NULL;
cCommandScheduler* g_scheduler = new cCommandScheduler;
DWORD WINAPI UpdateBeholderThread(LPVOID pVOIDBeholder);

// Call back signatures here
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void DrawObject(cMeshObject* pCurrentMeshObject,
	glm::mat4x4 mat_PARENT_Model,               // The "parent's" model matrix
	GLuint shaderID,                            // ID for the current shader
	cBasicTextureManager* pTextureManager,
	cVAOManager* pVAOManager,
	GLint mModel_location,                      // Uniform location of mModel matrix
	GLint mModelInverseTransform_location);      // Uniform location of mView location
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

// From here: https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats/5289624

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

bool LoadModelTypesIntoVAO(std::string fileTypesToLoadName,
	cVAOManager* pVAOManager,
	GLuint shaderID)
{
	std::ifstream modelTypeFile(fileTypesToLoadName.c_str());
	if (!modelTypeFile.is_open())
	{
		// Can't find that file
		return false;
	}

	// At this point, the file is open and ready for reading

	std::string PLYFileNameToLoad;     // = "assets/models/MOTO/Blender (load from OBJ export) - only Moto_xyz_n_rgba_uv.ply";
	std::string friendlyName;   // = "MOTO";

	bool bKeepReadingFile = true;

	const unsigned int BUFFER_SIZE = 1000;  // 1000 characters
	char textBuffer[BUFFER_SIZE];       // Allocate AND clear (that's the {0} part)
	// Clear that array to all zeros
	memset(textBuffer, 0, BUFFER_SIZE);

	// Or if it's integers, you can can do this short cut:
	// char textBuffer[BUFFER_SIZE] = { 0 };       // Allocate AND clear (that's the {0} part)

	while (bKeepReadingFile)
	{
		// Reads the entire line into the buffer (including any white space)
		modelTypeFile.getline(textBuffer, BUFFER_SIZE);

		PLYFileNameToLoad.clear();  // Erase whatever is already there (from before)
		PLYFileNameToLoad.append(textBuffer);

		// Is this the end of the file (have I read "EOF" yet?)?
		if (PLYFileNameToLoad == "EOF")
		{
			// All done
			bKeepReadingFile = false;
			// Skip to the end of the while loop
			continue;
		}

		// Load the "friendly name" line also

		memset(textBuffer, 0, BUFFER_SIZE);
		modelTypeFile.getline(textBuffer, BUFFER_SIZE);
		friendlyName.clear();
		friendlyName.append(textBuffer);

		sModelDrawInfo motoDrawInfo;

		c3DModelFileLoader fileLoader;
		//if (LoadThePLYFile(PLYFileNameToLoad, motoDrawInfo))
		std::string errorText = "";
		if (fileLoader.LoadPLYFile_Format_XYZ_N_RGBA_UV(PLYFileNameToLoad, motoDrawInfo, errorText))
		{
			std::cout << "Loaded the file OK" << std::endl;
		}
		else
		{
			std::cout << errorText;
		}

		if (pVAOManager->LoadModelIntoVAO(friendlyName, motoDrawInfo, shaderID))
		{
			std::cout << "Loaded the " << friendlyName << " model" << std::endl;
		}
	}//while (modelTypeFile

	return true;
}

bool CreateObjects(std::string fileName)
{
	std::ifstream objectFile(fileName.c_str());
	if (!objectFile.is_open())
	{
		// Can't find that file
		return false;
	}

	// Basic variables
	std::string meshName;
	std::string friendlyName;
	glm::vec3 position;
	//glm::vec3 rotation;
	glm::quat qRotation;
	glm::vec3 scale;
	// Advanced
	bool useRGBA;
	glm::vec4 colour;
	bool isWireframe;
	bool doNotLight;
	// skip first line
	std::getline(objectFile, meshName);
	std::getline(objectFile, meshName);
	meshName = "";
	for (std::string line; std::getline(objectFile, line);)
	{
		std::istringstream in(line);
		std::string type;
		in >> type;
		if (type == "basic")
		{
			in >> meshName >> friendlyName >> position.x >> position.y >> position.z >> qRotation.x >> qRotation.y >> qRotation.z >> scale.x >> scale.y >> scale.z;
			cMeshObject* pObject = new cMeshObject();
			pObject->meshName = meshName;
			pObject->friendlyName = friendlyName;
			pObject->position = position;
			pObject->qRotation = qRotation;
			pObject->scaleXYZ = scale;
			g_pMeshObjects.push_back(pObject);
		}
		//else if (type == "advanced")
		//{
		//	in >> meshName >> friendlyName >> useRGBA >> colour.x >> colour.y >> colour.z >> colour.w >> isWireframe >> scale >> doNotLight;
		//	cMeshObject* pObject = new cMeshObject();
		//	pObject->meshName = meshName;
		//	pObject->friendlyName = friendlyName;
		//	pObject->bUse_RGBA_colour = useRGBA;
		//	pObject->RGBA_colour = colour;
		//	pObject->isWireframe = isWireframe;
		//	pObject->scale = 1.0f;
		//	pObject->bDoNotLight = doNotLight;
		//}
	}

	return true;
}
bool SaveTheVAOModelTypesToFile(std::string fileTypesToLoadName,
	cVAOManager* pVAOManager);

void DrawConcentricDebugLightObjects(void);

// HACK: These are the light spheres we will use for debug lighting
cMeshObject* pDebugSphere_1 = NULL;// = new cMeshObject();
cMeshObject* pDebugSphere_2 = NULL;// = new cMeshObject();
cMeshObject* pDebugSphere_3 = NULL;// = new cMeshObject();
cMeshObject* pDebugSphere_4 = NULL;// = new cMeshObject();
cMeshObject* pDebugSphere_5 = NULL;// = new cMeshObject();

int main(int argc, char* argv[])
{
	//srand
	srand(time(NULL));

	std::cout << "starting up..." << std::endl;

	GLuint vertex_buffer = 0;

	GLuint shaderID = 0;

	GLint vpos_location = 0;
	GLint vcol_location = 0;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Project #1 - Hassan Assaf", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	std::cout << "Window created." << std::endl;

	glfwMakeContextCurrent(window);
	//   gladLoadGL( (GLADloadproc)glfwGetProcAddress );
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	//glfwSwapInterval(1);

	// IMGUI
	//initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	//platform / render bindings
	if (!ImGui_ImplGlfw_InitForOpenGL(window, true) || !ImGui_ImplOpenGL3_Init("#version 420"))
	{
		return 3;
	}

	// Create a shader thingy
	cShaderManager* pTheShaderManager = new cShaderManager();

	cShaderManager::cShader vertexShader01;
	cShaderManager::cShader fragmentShader01;

	vertexShader01.fileName = "assets/shaders/vertexShader01.glsl";
	fragmentShader01.fileName = "assets/shaders/fragmentShader01.glsl";

	if (!pTheShaderManager->createProgramFromFile("Shader_1", vertexShader01, fragmentShader01))
	{
		vertexShader01.fileName = "vertexShader01.glsl";
		fragmentShader01.fileName = "fragmentShader01.glsl";
		if (!pTheShaderManager->createProgramFromFile("Shader_1", vertexShader01, fragmentShader01))
		{
			std::cout << "Didn't compile shaders" << std::endl;
			std::string theLastError = pTheShaderManager->getLastError();
			std::cout << "Because: " << theLastError << std::endl;
			return -1;
		}
	}
	else
	{
		std::cout << "Compiled shader OK." << std::endl;
	}

	pTheShaderManager->useShaderProgram("Shader_1");

	shaderID = pTheShaderManager->getIDFromFriendlyName("Shader_1");

	glUseProgram(shaderID);

	::g_pTheLightManager = new cLightManager();

	cLightHelper* pLightHelper = new cLightHelper();

	// Set up the uniform variable (from the shader
	::g_pTheLightManager->LoadLightUniformLocations(shaderID);

	// Make this a spot light
//    vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
//                    // 0 = pointlight
//                    // 1 = spot light
//                    // 2 = directional light
	::g_pTheLightManager->vecTheLights[0].name = "MainLight";
	::g_pTheLightManager->vecTheLights[0].param1.x = 2.0f;
	::g_pTheLightManager->vecTheLights[0].position = glm::vec4(0.0f, 300.0f, -1000.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[0].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	// inner and outer angles
	::g_pTheLightManager->vecTheLights[0].atten = glm::vec4(0.1f, 0.001f, 0.0000001f, 1.0f);
	::g_pTheLightManager->vecTheLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[0].param1.y = 10.0f;     // Degrees
	::g_pTheLightManager->vecTheLights[0].param1.z = 20.0f;     // Degrees
	::g_pTheLightManager->vecTheLights[0].TurnOn();

	// Make light #2 a directional light
	// BE CAREFUL about the direction and colour, since "colour" is really brightness.
	// (i.e. there NO attenuation)
	::g_pTheLightManager->vecTheLights[1].name = "MoonLight";
	::g_pTheLightManager->vecTheLights[1].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[1].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[1].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[1].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[1].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[1].TurnOn();
	//::g_pTheLightManager->vecTheLights[2].name = "notSure";

	::g_pTheLightManager->vecTheLights[3].name = "Torch2";
	::g_pTheLightManager->vecTheLights[3].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[3].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[3].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[3].diffuse = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[3].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//::g_pTheLightManager->vecTheLights[3].TurnOn();

	::g_pTheLightManager->vecTheLights[4].name = "Torch3";
	::g_pTheLightManager->vecTheLights[4].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[4].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[4].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[4].diffuse = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[4].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//::g_pTheLightManager->vecTheLights[4].TurnOn();

	::g_pTheLightManager->vecTheLights[5].name = "Torch4";
	::g_pTheLightManager->vecTheLights[5].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[5].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//::g_pTheLightManager->vecTheLights[5].TurnOn();

	::g_pTheLightManager->vecTheLights[5].name = "Torch5";
	::g_pTheLightManager->vecTheLights[5].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[5].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//::g_pTheLightManager->vecTheLights[5].TurnOn();

	::g_pTheLightManager->vecTheLights[6].name = "Torch6";
	::g_pTheLightManager->vecTheLights[6].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[6].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[6].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[6].diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[6].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//::g_pTheLightManager->vecTheLights[6].TurnOn();

	for (int i = 7; i < 16; i++)
	{
		::g_pTheLightManager->vecTheLights[i].param1.x = 1.0f;
		::g_pTheLightManager->vecTheLights[i].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
		::g_pTheLightManager->vecTheLights[i].atten = glm::vec4(0.1f, 0.001f, 0.1f, 1.0f);
		::g_pTheLightManager->vecTheLights[i].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		::g_pTheLightManager->vecTheLights[i].direction = glm::vec4(0.f, 0.f, 1.f, 1.f);
		::g_pTheLightManager->vecTheLights[i].qDirection = glm::quat(glm::vec3(0));
		::g_pTheLightManager->vecTheLights[i].param1.y = 10.0f;     // Degrees
		::g_pTheLightManager->vecTheLights[i].param1.z = 20.0f;     // Degrees
	}
	::g_pTheLightManager->vecTheLights[7].name = "Beholder1L";
	::g_pTheLightManager->vecTheLights[8].name = "Beholder1M";
	::g_pTheLightManager->vecTheLights[9].name = "Beholder1R";
	::g_pTheLightManager->vecTheLights[10].name = "Beholder2L";
	::g_pTheLightManager->vecTheLights[11].name = "Beholder2M";
	::g_pTheLightManager->vecTheLights[12].name = "Beholder2R";
	::g_pTheLightManager->vecTheLights[13].name = "Beholder3L";
	::g_pTheLightManager->vecTheLights[14].name = "Beholder3M";
	::g_pTheLightManager->vecTheLights[15].name = "Beholder3R";

	// Load the models

	if (!LoadModelTypesIntoVAO("assets/PLYFilesToLoadIntoVAO.txt", pVAOManager, shaderID))
	{
		if (!LoadModelTypesIntoVAO("PLYFilesToLoadIntoVAO.txt", pVAOManager, shaderID))
		{
			std::cout << "Error: Unable to load list of models to load into VAO file" << std::endl;
		}
		// Do we exit here?
		// (How do we clean up stuff we've made, etc.)
	}//if (!LoadModelTypesIntoVAO...

// **************************************************************************************
// START OF: LOADING the file types into the VAO manager:
/*
*/
// END OF: LOADING the file types into the VAO manager
// **************************************************************************************

	// On the heap (we used new and there's a pointer)

	if (!CreateObjects("assets/createObjects.txt"))
	{
		if (!CreateObjects("createObjects.txt"))
		{
			std::cout << "Error: Unable to load list of objects to create" << std::endl;
		}
	}

	cMeshObject* pSkyBox = new cMeshObject();
	pSkyBox->meshName = "Skybox_Sphere";
	pSkyBox->friendlyName = "skybox";
	// I'm NOT going to add it to the list of objects to draw

	//	// ISLAND Terrain (final exam 2021 example)
	//cMeshObject* pIslandTerrain = new cMeshObject();
	//pIslandTerrain->meshName = "Terrain";     //  "Terrain";
	//pIslandTerrain->friendlyName = "Ground";
	//pIslandTerrain->position = glm::vec3(0.0f, -25.0f, 0.0f);
	//pIslandTerrain->isWireframe = false;
	//pIslandTerrain->textures[0] = "grass.bmp";
	//pIslandTerrain->textureRatios[0] = 1.f;
	//pIslandTerrain->SetUniformScale(5);
	////g_pMeshObjects.push_back(pIslandTerrain);

	cMeshObject* pTerrain = new cMeshObject();
	pTerrain->meshName = "Terrain";     //  "Terrain";
	pTerrain->friendlyName = "Ground";
	//pTerrain->bUse_RGBA_colour = false;      // Use file colours    pTerrain->RGBA_colour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	//pTerrain->specular_colour_and_power
	//	= glm::vec4(1.0f, 1.0f, 1.0f, 1000.0f);
	// Make it REALY transparent
	//pTerrain->RGBA_colour.w = 1.f;
	pTerrain->position = glm::vec3(-500.0f, -20.0f, 500.0f);
	pTerrain->setRotationFromEuler(glm::vec3(4.7, 0, 0));
	pTerrain->isWireframe = false;
	pTerrain->SetUniformScale(10);
	pTerrain->textures[0] = "grass.bmp";
	pTerrain->textures[1] = "grass.bmp";
	pTerrain->textures[2] = "grass.bmp";
	pTerrain->textures[3] = "grass.bmp";
	pTerrain->textureRatios[0] = 0.5f;
	pTerrain->textureRatios[1] = 0.5f;
	pTerrain->textureRatios[2] = 0.5f;
	pTerrain->textureRatios[3] = 0.5f;

	g_pMeshObjects.push_back(pTerrain);
	//basic Terrain Ground 0 0 0 0 0 0 1

	// MOON
	cMeshObject* pMoon = new cMeshObject();
	pMoon->meshName = "Moon";     //  "Terrain";
	pMoon->friendlyName = "Moon";
	//pTerrain->bUse_RGBA_colour = false;      // Use file colours    pTerrain->RGBA_colour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pTerrain->specular_colour_and_power = glm::vec4(1.0f, 1.0f, 1.0f, 1000.0f);
	// Make it REALY transparent
	pTerrain->RGBA_colour.w = 0.5f;

	// DEBUG SPHERES
	pDebugSphere_1 = new cMeshObject();
	pDebugSphere_1->meshName = "ISO_Sphere_1";
	pDebugSphere_1->friendlyName = "Debug_Sphere_1";
	pDebugSphere_1->bUse_RGBA_colour = true;
	pDebugSphere_1->RGBA_colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pDebugSphere_1->isWireframe = true;
	pDebugSphere_1->SetUniformScale(1);

	pDebugSphere_1->bDoNotLight = true;

	g_pMeshObjects.push_back(pDebugSphere_1);

	pDebugSphere_2 = new cMeshObject();
	pDebugSphere_2->meshName = "ISO_Sphere_1";
	pDebugSphere_2->friendlyName = "Debug_Sphere_2";
	pDebugSphere_2->bUse_RGBA_colour = true;
	pDebugSphere_2->RGBA_colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	pDebugSphere_2->isWireframe = true;
	pDebugSphere_2->SetUniformScale(1);
	pDebugSphere_2->bDoNotLight = true;
	g_pMeshObjects.push_back(pDebugSphere_2);

	pDebugSphere_3 = new cMeshObject();
	pDebugSphere_3->meshName = "ISO_Sphere_1";
	pDebugSphere_3->friendlyName = "Debug_Sphere_3";
	pDebugSphere_3->bUse_RGBA_colour = true;
	pDebugSphere_3->RGBA_colour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pDebugSphere_3->isWireframe = true;
	pDebugSphere_3->SetUniformScale(1);
	pDebugSphere_3->bDoNotLight = true;
	g_pMeshObjects.push_back(pDebugSphere_3);

	pDebugSphere_4 = new cMeshObject();
	pDebugSphere_4->meshName = "ISO_Sphere_1";
	pDebugSphere_4->friendlyName = "Debug_Sphere_4";
	pDebugSphere_4->bUse_RGBA_colour = true;
	pDebugSphere_4->RGBA_colour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pDebugSphere_4->isWireframe = true;
	pDebugSphere_4->SetUniformScale(1);
	pDebugSphere_4->bDoNotLight = true;
	g_pMeshObjects.push_back(pDebugSphere_4);

	pDebugSphere_5 = new cMeshObject();
	pDebugSphere_5->meshName = "ISO_Sphere_1";
	pDebugSphere_5->friendlyName = "Debug_Sphere_5";
	pDebugSphere_5->bUse_RGBA_colour = true;
	pDebugSphere_5->RGBA_colour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pDebugSphere_5->isWireframe = true;
	pDebugSphere_5->SetUniformScale(1);
	pDebugSphere_5->bDoNotLight = true;
	g_pMeshObjects.push_back(pDebugSphere_5);

	// Beholder Beholder_Base_color.bmp
	cMeshObject* pBeholder = new cMeshObject();
	pBeholder->meshName = "Beholder";
	pBeholder->friendlyName = "Beholder1";
	pBeholder->bUse_RGBA_colour = false;      // Use file colours    pTerrain->RGBA_colour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pBeholder->specular_colour_and_power = glm::vec4(1.0f, 0.0f, 0.0f, 1000.0f);
	pBeholder->position = glm::vec3(0.f, 5.f, 0.f);
	pBeholder->setRotationFromEuler(glm::vec3(0));
	pBeholder->isWireframe = false;
	pBeholder->SetUniformScale(0.2f);
	pBeholder->textures[0] = "Beholder_Base_color.bmp";
	pBeholder->textureRatios[0] = 1.0f;
	pBeholder->textureRatios[1] = 1.0f;
	pBeholder->textureRatios[2] = 1.0f;
	pBeholder->textureRatios[3] = 1.0f;
	g_pMeshObjects.push_back(pBeholder);

	//cMeshObject* pFloorTile = new cMeshObject();
	//pFloorTile->meshName = "Floor";
	//pFloorTile->friendlyName = "Floor";
	//pFloorTile->bUse_RGBA_colour = false;
	//pFloorTile->position = glm::vec3(0.f, 5.f, 0.f);
	//pFloorTile->setRotationFromEuler(glm::vec3(0));
	//pFloorTile->isWireframe = false;
	//pFloorTile->SetUniformScale(0.01f);
	//pFloorTile->textures[0] = "Dungeons_2_Texture_01_A.bmp";
	//pFloorTile->textureRatios[0] = 1.0f;
	//g_pMeshObjects.push_back(pFloorTile);

	GLint mvp_location = glGetUniformLocation(shaderID, "MVP");       // program
	// uniform mat4 mModel;
	// uniform mat4 mView;
	// uniform mat4 mProjection;
	GLint mModel_location = glGetUniformLocation(shaderID, "mModel");
	GLint mView_location = glGetUniformLocation(shaderID, "mView");
	GLint mProjection_location = glGetUniformLocation(shaderID, "mProjection");
	// Need this for lighting
	GLint mModelInverseTransform_location = glGetUniformLocation(shaderID, "mModelInverseTranspose");

	// Textures
	::g_pTextureManager = new cBasicTextureManager();

	::g_pTextureManager->SetBasePath("assets/textures");
	if (!::g_pTextureManager->Create2DTextureFromBMPFile("aerial-drone-view-rice-field.bmp"))
	{
		std::cout << "Didn't load texture" << std::endl;
	}
	else
	{
		std::cout << "texture loaded" << std::endl;
	}
	::g_pTextureManager->Create2DTextureFromBMPFile("Dungeons_2_Texture_01_A.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("lroc_color_poles_4k.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("taylor-swift-tour-dates-fan-wedding-plans.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("water.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("crystal.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("crystal2.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("crystal3.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("flame.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("Beholder_Base_color.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("grass.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("Long_blue_Jet_Flame.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("cobblestones_stencil_mask.bmp");

	::g_pTextureManager->Create2DTextureFromBMPFile("carbon.bmp");

	// Load a skybox
	// Here's an example of the various sides: http://www.3dcpptutorials.sk/obrazky/cube_map.jpg
	std::string errorString = "";
	if (::g_pTextureManager->CreateCubeTextureFromBMPFiles("TropicalSunnyDay",
		"SpaceBox_right1_posX.bmp", /* positive X */
		"SpaceBox_left2_negX.bmp",  /* negative X */
		"SpaceBox_top3_posY.bmp",    /* positive Y */
		"SpaceBox_bottom4_negY.bmp",  /* negative Y */
		"SpaceBox_front5_posZ.bmp",/* positive Z */
		"SpaceBox_back6_negZ.bmp",/* negative Z */
		true, errorString))
	{
		std::cout << "Loaded the night sky cube map OK" << std::endl;
	}
	else
	{
		std::cout << "ERROR: Didn't load the tropical sunny day cube map. How sad." << std::endl;
		std::cout << "(because: " << errorString << std::endl;
	}
	//std::string errorString = "";
	//if (::g_pTextureManager->CreateCubeTextureFromBMPFiles("TropicalSunnyDay",
	//	"TropicalSunnyDayRight2048.bmp", /* positive X */
	//	"TropicalSunnyDayLeft2048.bmp",  /* negative X */
	//	"TropicalSunnyDayUp2048.bmp",    /* positive Y */
	//	"TropicalSunnyDayDown2048.bmp",  /* negative Y */
	//	"TropicalSunnyDayBack2048.bmp",  /* positive Z */
	//	"TropicalSunnyDayFront2048.bmp", /* negative Z */
	//	true, errorString))
	//{
	//	std::cout << "Loaded the tropical sunny day cube map OK" << std::endl;
	//}
	//else
	//{
	//	std::cout << "ERROR: Didn't load the tropical sunny day cube map. How sad." << std::endl;
	//	std::cout << "(because: " << errorString << std::endl;
	//}
	std::cout.flush();
	// GUI
	ImGui::StyleColorsDark();
	GUI EditGUI;
	//// Physics
	g_engine.Initialize();
	float deltaTime = std::clock();
	float duration = 0;
	//bool increase = true;
	int increase = 1;
	//pVAOManager->Load();
	MazeHelper* _mazeHelper = new MazeHelper("maze1000.txt");
	//glm::vec2 portionPosition = glm::vec2((int)g_cameraEye.x, (int)g_cameraEye.z);
	//	std::vector<cMeshObject*> portionMaze = _mazeHelper->getMazeMeshesAt(portionPosition, portionSize);

	std::vector<cMeshObject*> blockTiles;
	for (size_t i = 0; i < 400; i++)
	{
		cMeshObject* pFloor = new cMeshObject();
		pFloor->meshName = "Floor";
		pFloor->friendlyName = "Floor";
		pFloor->bUse_RGBA_colour = false;
		pFloor->position = glm::vec3(0, 1.f, 0);
		pFloor->setRotationFromEuler(glm::vec3(0));
		pFloor->isWireframe = false;
		pFloor->SetUniformScale(0.002f);
		pFloor->textures[0] = "Dungeons_2_Texture_01_A.bmp";
		pFloor->textureRatios[0] = 1.0f;

		cMeshObject* pWallLeft = new cMeshObject();
		pWallLeft->meshName = "Wall";
		pWallLeft->friendlyName = "WallLeft";
		pWallLeft->bUse_RGBA_colour = false;
		pWallLeft->position = glm::vec3(-1.f, 1.f, 0.0f);
		pWallLeft->setRotationFromEuler(glm::vec3(0.0f, glm::radians(180.f), 0.0f));
		pWallLeft->isWireframe = false;
		pWallLeft->SetUniformScale(0.002f);
		pWallLeft->textures[0] = "Dungeons_2_Texture_01_A.bmp";
		pWallLeft->textureRatios[0] = 1.0f;

		cMeshObject* pWallRight = new cMeshObject();
		pWallRight->meshName = "Wall";
		pWallRight->friendlyName = "WallRight";
		pWallRight->bUse_RGBA_colour = false;
		pWallRight->position = glm::vec3(0.f, 1.f, -1.0f);
		pWallRight->setRotationFromEuler(glm::vec3(0.0f, glm::radians(0.f), 0.0f));
		pWallRight->isWireframe = false;
		pWallRight->SetUniformScale(0.002f);
		pWallRight->textures[0] = "Dungeons_2_Texture_01_A.bmp";
		pWallRight->textureRatios[0] = 1.0f;

		cMeshObject* pWallTop = new cMeshObject();
		pWallTop->meshName = "Wall";
		pWallTop->friendlyName = "WallTop";
		pWallTop->bUse_RGBA_colour = false;
		pWallTop->position = glm::vec3(0.f, 1.f, 0.f);
		pWallTop->setRotationFromEuler(glm::vec3(0.0f, glm::radians(270.f), 0.0f));
		pWallTop->isWireframe = false;
		pWallTop->SetUniformScale(0.002f);
		pWallTop->textures[0] = "Dungeons_2_Texture_01_A.bmp";
		pWallTop->textureRatios[0] = 1.0f;

		cMeshObject* pWallBottom = new cMeshObject();
		pWallBottom->meshName = "Wall";
		pWallBottom->friendlyName = "WallTop";
		pWallBottom->bUse_RGBA_colour = false;
		pWallBottom->position = glm::vec3(-1.f, 1.f, -1.f);
		pWallBottom->setRotationFromEuler(glm::vec3(0.0f, glm::radians(90.f), 0.0f));
		pWallBottom->isWireframe = false;
		pWallBottom->SetUniformScale(0.002f);
		pWallBottom->textures[0] = "Dungeons_2_Texture_01_A.bmp";
		pWallBottom->textureRatios[0] = 1.0f;

		pFloor->vecChildMeshes.push_back(pWallLeft);
		pFloor->vecChildMeshes.push_back(pWallRight);
		pFloor->vecChildMeshes.push_back(pWallTop);
		pFloor->vecChildMeshes.push_back(pWallBottom);
		blockTiles.push_back(pFloor);
	}
	// Create the path by splitting it among 8 threads
	//ThreadPathNodes(_mazeHelper, pathfinder, 16);

	std::vector<std::vector<char>> portionMaze;
	int portionSize = 20.f;
	glm::vec2 portionPosition = glm::vec2(0, 0);
	const int NUM_BEHOLDERS = 1000;
	std::vector<Beholder*>* pTheBeholders = new std::vector<Beholder*>();
	std::vector<glm::vec2> portionToDraw;

	// Do threads in multiple steps to avoid errors where beholders are acting before the rest are even on a thread yet
	// Create the Beholders
	for (size_t i = 0; i < NUM_BEHOLDERS; i++)
	{
		glm::vec2 pos = _mazeHelper->getRandomMazeCell();
		Beholder* pBeholder = new Beholder(i, pos, _mazeHelper);
		pTheBeholders->push_back(pBeholder);
		pBeholder->allBeholders = pTheBeholders;
	}

	// Create the threas 
	std::vector<sBeholderThreadData*> threadDataList;
	for (size_t i = 0; i < NUM_BEHOLDERS; i++)
	{
		sBeholderThreadData* pBeholderData = new sBeholderThreadData();
		pBeholderData->pTheBeholder = pTheBeholders->at(i);
		threadDataList.push_back(pBeholderData);
	}

	// Start the threads
	for (size_t i = 0; i < NUM_BEHOLDERS; i++)
	{
		LPDWORD lpThreadId = 0;
		HANDLE hThread = 0;

		hThread =
			CreateThread(NULL,              // Security attributes
				0,                  // Use default stack size
				UpdateBeholderThread,   // Address of the function we are going to call
				(void*)threadDataList[i],   // Pass in the thread data object for this Beholder
				0, // 0 or CREATE_SUSPENDED
				lpThreadId);
	}
	bool isKeyPressed = false;
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_KP_0) && !isKeyPressed)
		{
			isKeyPressed = true;
			int randomIndex = -1;
			bool foundOne = false;
			while (!foundOne)
			{
				randomIndex = rand() % pTheBeholders->size();
				if (pTheBeholders->at(randomIndex)->isAlive && pTheBeholders->at(randomIndex)->state == CHASING)
				{
					foundOne = true;
					g_cameraEye.z = pTheBeholders->at(randomIndex)->position.x;
					g_cameraEye.x = pTheBeholders->at(randomIndex)->position.y;
				}
			}
		}
		else if (!glfwGetKey(window, GLFW_KEY_KP_0))
		{
			isKeyPressed = false;
		}
		glm::vec3 portion = (::g_cameraEye + 10.f * ::g_cameraTarget);
		portionPosition = glm::vec2((int)(portion.x + ::g_cameraTarget.x), (int)(portion.z + ::g_cameraTarget.z));
		//portionPosition = glm::floor(glm::vec2(::g_cameraEye.x, ::g_cameraEye.z));
		portionMaze = _mazeHelper->getMazeAt(portionPosition, portionSize);
		::g_pTheLightManager->CopyLightInformationToShader(shaderID);
		//	pBrain->Update(0.1f);
		duration = (std::clock() - deltaTime) / (double)CLOCKS_PER_SEC;
		g_engine.PhysicsUpdate(0.05);

		//ai_system.update(0.1);
		if (!menuMode)
		{
			g_engine.Update(0.1);
		}
		DrawConcentricDebugLightObjects();

		float ratio;
		int width, height;
		//        mat4x4 m, p, mvp;
		//        glm::mat4x4 matModel;
		glm::mat4x4 matProjection;
		glm::mat4x4 matView;

		//        glm::mat4x4 mvp;            // Model-View-Projection

			// Camera follow
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);

		// note the binary OR (not the usual boolean "or" comparison)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//        glClear(GL_COLOR_BUFFER_BIT);

		glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

		matView = glm::lookAt(::g_cameraEye,
			::g_cameraEye + ::g_cameraTarget,
			upVector);
		// Pass eye location to the shader
		// uniform vec4 eyeLocation;
		GLint eyeLocation_UniLoc = glGetUniformLocation(shaderID, "eyeLocation");

		glUniform4f(eyeLocation_UniLoc,
			::g_cameraEye.x, ::g_cameraEye.y, ::g_cameraEye.z, 1.0f);

		//        mat4x4_rotate_Z(m, m, (float)glfwGetTime());
		//        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		matProjection = glm::perspective(
			0.6f,       // Field of view (in degress, more or less 180)
			ratio,
			0.1f,       // Near plane (make this as LARGE as possible)
			10000.0f);   // Far plane (make this as SMALL as possible)
		// 6-8 digits of precision

//        glm::ortho(

		// Set once per scene (not per object)
		glUniformMatrix4fv(mView_location, 1, GL_FALSE, glm::value_ptr(matView));
		glUniformMatrix4fv(mProjection_location, 1, GL_FALSE, glm::value_ptr(matProjection));

		int floorIndex = 0;
		int wallIndex = 0;
		for (size_t y = 0; y < portionSize; y++)
		{
			for (size_t x = 0; x < portionSize; x++)
			{
				cMeshObject* mesh;
				glm::vec3 meshPos;
				if (portionMaze[y][x] == 'x')
				{
					mesh = blockTiles[wallIndex++];
					meshPos = glm::vec3((portionPosition.x - portionSize / 2 + x) * 1.f, 1.f, (portionPosition.y - portionSize / 2 + y) * 1.f);
					mesh->bUse_RGBA_colour = false;
				}
				else if (portionMaze[y][x] == 'o')
				{
					mesh = blockTiles[floorIndex++];
					meshPos = glm::vec3((portionPosition.x - portionSize / 2 + x) * 1.f, 0.f, (portionPosition.y - portionSize / 2 + y) * 1.f);
					mesh->position = meshPos;
					mesh->bUse_RGBA_colour = true;
					mesh->RGBA_colour = glm::vec4(0.3f, 0.3f, 1.f, 1.f);
				}
				else
				{
					mesh = nullptr;
				}

				if (mesh != nullptr && mesh->bIsVisible)
				{
					// The parent's model matrix is set to the identity
					
					glm::mat4x4 matModel = glm::mat4x4(1.0f);
					mesh->position = meshPos;
					DrawObject(mesh, matModel, shaderID, ::g_pTextureManager, pVAOManager, mModel_location, mModelInverseTransform_location);

					if (portionMaze[y][x] == 'x')
					{
						for (cMeshObject* childObject : mesh->vecChildMeshes)
						{
							glm::vec3 oldPos = childObject->position;
							childObject->position += mesh->position;
							childObject->position.y = 0.f;
							if (childObject->bIsVisible)
							{
								glm::mat4x4 matModel = glm::mat4x4(1.0f);
								DrawObject(childObject, matModel, shaderID, ::g_pTextureManager, pVAOManager, mModel_location, mModelInverseTransform_location);
							}
							childObject->position = oldPos;
						}
					}
				}
			}
		}
		for (Beholder* beholder : *pTheBeholders)
		{
			if (beholder->mesh != nullptr && beholder->mesh->bIsVisible)
			{
				glm::vec2 beholderPos = glm::vec2(beholder->position.x, beholder->position.y);
				glm::vec2 portionMin = portionPosition - glm::vec2(portionSize / 2);
				glm::vec2 portionMax = portionPosition + glm::vec2(portionSize / 2);

				//std::cout << "Beholder position: " << beholderPos.x << ", " << beholderPos.y << std::endl;
				//std::cout << "Portion min: " << portionMin.x << ", " << portionMin.y << std::endl;
				//std::cout << "Portion max: " << portionMax.x << ", " << portionMax.y << std::endl;

				if (beholderPos.y >= portionMin.x && beholderPos.y <= portionMax.x &&
					beholderPos.x >= portionMin.y && beholderPos.x <= portionMax.y)
				{
					glm::mat4x4 matModel = glm::mat4x4(1.0f);
					DrawObject(beholder->mesh, matModel, shaderID, ::g_pTextureManager, pVAOManager, mModel_location, mModelInverseTransform_location);
				}
			}
		}


		//for (cMeshObject* mesh : portionMaze)
		//{
		//	if (mesh != nullptr && mesh->bIsVisible)
		//	{
		//		// The parent's model matrix is set to the identity
		//		glm::mat4x4 matModel = glm::mat4x4(1.0f);
		//		DrawObject(mesh, matModel, shaderID, ::g_pTextureManager, pVAOManager, mModel_location, mModelInverseTransform_location);

		//		for (cMeshObject* childObject : mesh->vecChildMeshes)
		//		{
		//			glm::vec3 oldPos = childObject->position;
		//			childObject->position += mesh->position - glm::vec3(0.f, 10.f, 0.f);
		//			if (childObject->bIsVisible)
		//			{
		//				glm::mat4x4 matModel = glm::mat4x4(1.0f);
		//				DrawObject(childObject, matModel, shaderID, ::g_pTextureManager, pVAOManager, mModel_location, mModelInverseTransform_location);
		//			}
		//			childObject->position = oldPos;
		//		}
		//	}
		//}
		for (std::vector< cMeshObject* >::iterator itCurrentMesh = g_pMeshObjects.begin();
			itCurrentMesh != g_pMeshObjects.end();
			itCurrentMesh++)
		{
			cMeshObject* pCurrentMeshObject = *itCurrentMesh;
			if (!pCurrentMeshObject->bIsVisible)
				continue;

			// The parent's model matrix is set to the identity
			glm::mat4x4 matModel = glm::mat4x4(1.0f);
			DrawObject(pCurrentMeshObject,
				matModel,
				shaderID, ::g_pTextureManager,
				pVAOManager, mModel_location, mModelInverseTransform_location);
			for (cMeshObject* childObject : pCurrentMeshObject->vecChildMeshes)
			{
				glm::vec3 oldPos = childObject->position;
				childObject->position += pCurrentMeshObject->position;
				if (!childObject->bIsVisible)
					continue;
				glm::mat4x4 matModel = glm::mat4x4(1.0f);
				DrawObject(childObject,
					matModel,
					shaderID, ::g_pTextureManager,
					pVAOManager, mModel_location, mModelInverseTransform_location);
				childObject->position = oldPos;
			}
		}

		// Draw the skybox
		{
			GLint bIsSkyboxObject_UL = glGetUniformLocation(shaderID, "bIsSkyboxObject");
			glUniform1f(bIsSkyboxObject_UL, (GLfloat)GL_TRUE);

			glm::mat4x4 matModel = glm::mat4x4(1.0f);

			// move skybox to the cameras location
			pSkyBox->position = ::g_cameraEye;

			// The scale of this large skybox needs to be smaller than the far plane
			//  of the projection matrix.
			// Maybe make it half the size
			// Here, our far plane is 10000.0f...
			pSkyBox->SetUniformScale(7500.0f);

			// All the drawing code has been moved to the DrawObject function
			DrawObject(pSkyBox,
				matModel,
				shaderID, ::g_pTextureManager,
				pVAOManager, mModel_location, mModelInverseTransform_location);

			// Turn this off
			glUniform1f(bIsSkyboxObject_UL, (GLfloat)GL_FALSE);
		}//for ( unsigned int index
		//    _____           _          __
		//   | ____|_ __   __| |   ___  / _|  ___  ___ ___ _ __   ___
		//   |  _| | '_ \ / _` |  / _ \| |_  / __|/ __/ _ \ '_ \ / _ \
		        //   | |___| | | | (_| | | (_) |  _| \__ \ (_|  __/ | | |  __/
				//   |_____|_| |_|\__,_|  \___/|_|   |___/\___\___|_| |_|\___|
				//
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		EditGUI.render();
		{
			ImGui::Begin("#CH", nullptr, ImGuiWindowFlags_NoMove | ImGuiInputTextFlags_ReadOnly | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
			ImDrawList* draw = ImGui::GetBackgroundDrawList();
			draw->AddCircle(ImVec2((1920 / 2) - 10, 1009 / 2), 6, IM_COL32(255, 0, 0, 255), 100, 0.0f);
			ImGui::End();
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);

		// Set the window title
		//glfwSetWindowTitle(window, "Hey");

		std::stringstream ssTitle;
		ssTitle << "Camera (x,y,z): "
			<< ::g_cameraEye.x << ", "
			<< ::g_cameraEye.y << ", "
			<< ::g_cameraEye.z
			<< "  Light #" << currentLight << " (xyz): "
			<< ::g_pTheLightManager->vecTheLights[currentLight].position.x << ", "
			<< ::g_pTheLightManager->vecTheLights[currentLight].position.y << ", "
			<< ::g_pTheLightManager->vecTheLights[currentLight].position.z
			<< " linear: " << ::g_pTheLightManager->vecTheLights[currentLight].atten.y
			<< " quad: " << ::g_pTheLightManager->vecTheLights[currentLight].atten.z;

		std::string theText = ssTitle.str();

		glfwSetWindowTitle(window, ssTitle.str().c_str());
		// Or this:
		//std::string theText = ssTitle.str();
		//glfwSetWindowTitle(window, ssTitle.str().c_str() );
	}

	// Get rid of stuff
	delete pTheShaderManager;
	delete ::g_pTheLightManager;

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void DrawConcentricDebugLightObjects(void)
{
	extern bool bEnableDebugLightingObjects;

	if (!bEnableDebugLightingObjects)
	{
		pDebugSphere_1->bIsVisible = false;
		pDebugSphere_2->bIsVisible = false;
		pDebugSphere_3->bIsVisible = false;
		pDebugSphere_4->bIsVisible = false;
		pDebugSphere_5->bIsVisible = false;
		return;
	}

	pDebugSphere_1->bIsVisible = true;
	pDebugSphere_2->bIsVisible = true;
	pDebugSphere_3->bIsVisible = true;
	pDebugSphere_4->bIsVisible = true;
	pDebugSphere_5->bIsVisible = true;

	cLightHelper theLightHelper;

	// Move the debug sphere to where the light #0 is
	pDebugSphere_1->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	pDebugSphere_2->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	pDebugSphere_3->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	pDebugSphere_4->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	pDebugSphere_5->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);

	{
		// Draw a bunch of concentric spheres at various "brightnesses"
		float distance75percent = theLightHelper.calcApproxDistFromAtten(
			0.75f,  // 75%
			0.001f,
			100000.0f,
			::g_pTheLightManager->vecTheLights[currentLight].atten.x,
			::g_pTheLightManager->vecTheLights[currentLight].atten.y,
			::g_pTheLightManager->vecTheLights[currentLight].atten.z);

		pDebugSphere_2->SetUniformScale(distance75percent);
		pDebugSphere_2->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	}

	{
		// Draw a bunch of concentric spheres at various "brightnesses"
		float distance50percent = theLightHelper.calcApproxDistFromAtten(
			0.50f,  // 75%
			0.001f,
			100000.0f,
			::g_pTheLightManager->vecTheLights[currentLight].atten.x,
			::g_pTheLightManager->vecTheLights[currentLight].atten.y,
			::g_pTheLightManager->vecTheLights[currentLight].atten.z);

		pDebugSphere_3->SetUniformScale(distance50percent);
		pDebugSphere_3->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	}

	{
		// Draw a bunch of concentric spheres at various "brightnesses"
		float distance25percent = theLightHelper.calcApproxDistFromAtten(
			0.25f,  // 75%
			0.001f,
			100000.0f,
			::g_pTheLightManager->vecTheLights[currentLight].atten.x,
			::g_pTheLightManager->vecTheLights[currentLight].atten.y,
			::g_pTheLightManager->vecTheLights[currentLight].atten.z);

		pDebugSphere_4->SetUniformScale(distance25percent);
		pDebugSphere_4->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	}

	{
		// Draw a bunch of concentric spheres at various "brightnesses"
		float distance5percent = theLightHelper.calcApproxDistFromAtten(
			0.05f,  // 75%
			0.001f,
			100000.0f,
			::g_pTheLightManager->vecTheLights[currentLight].atten.x,
			::g_pTheLightManager->vecTheLights[currentLight].atten.y,
			::g_pTheLightManager->vecTheLights[currentLight].atten.z);

		pDebugSphere_5->SetUniformScale(distance5percent);
		pDebugSphere_5->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	}
	return;
}

DWORD WINAPI UpdateBeholderThread(LPVOID pVOIDBeholder)
{
	std::cout << "Starting the beholder thread" << std::endl;
	sBeholderThreadData* pBeholder = (sBeholderThreadData*)(pVOIDBeholder);

	while (!pBeholder->pTheBeholder->exitThread && !pBeholder->bExitThread)
	{
		if (!pBeholder->bSuspendThread)
		{
			pBeholder->pTheBeholder->Update();
			Sleep(0);   // Release this thread if needed
		}
		else
		{
			// Put thread to sleep for X ms
			Sleep(pBeholder->suspendTime_ms);
		}
	}

	std::cout << "Exiting the beholder thread" << std::endl;
	return 0;
}
