// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <iostream>
#include <stack>   
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

const int window_width = 1024, window_height = 768;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], GLushort[], int);
void loadObject(char*, glm::vec4, Vertex* &, GLushort* &, int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 16;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
// Grid
const int axisIndex = 0;
const int gridIndex = 1;
// Normal colored objects
const int baseIndexStandardColor = 2;
const int topIndexStandardColor = 3;
const int arm1IndexStandardColor = 4;
const int jointIndexStandardColor = 5;
const int arm2IndexStandardColor = 6;
const int penIndexStandardColor = 7;
const int buttonIndexStandardColor = 8;
// Lighter colored objects
const int baseIndexHighlightedColor = 9;
const int topIndexHighlightedColor = 10;
const int arm1IndexHighlightedColor = 11;
const int arm2IndexHighlightedColor = 12;
const int penIndexHighlightedColor = 13;
// Lights
//const int lightCube1Index = 14;
//const int lightCube2Index = 15;
GLuint VertexArrayId[NumObjects];
GLuint VertexBufferId[NumObjects];
GLuint IndexBufferId[NumObjects];

// TL
size_t VertexBufferSize[NumObjects];
size_t IndexBufferSize[NumObjects];
size_t NumIdcs[NumObjects];
size_t NumVerts[NumObjects];

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;

// Declare global objects
// TL
const size_t CoordVertsCount = 6;
Vertex CoordVerts[CoordVertsCount];
// Colors 
float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
float blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
// General vertices
float origin[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float xMax[4] = { 5.0f, 0.0f, 0.0f, 1.0f };
float yMax[4] = { 0.0f, 5.0f, 0.0f, 1.0f };
float zMax[4] = { 0.0f, 0.0f, 5.0f, 1.0f };
// Grid vertices
const size_t GridVertsIndexCount = 44;
Vertex GridVerts[GridVertsIndexCount];
GLushort GridVertsIndices[GridVertsIndexCount];
// Obj vertices stored in a 2D array
#define MAXVERTS 320
#define MAXPARTS 7
Vertex RobotArmVerts[MAXPARTS][MAXVERTS];
GLushort RobotArmIndices[MAXVERTS];
// Booleans
bool CameraSelected = false;
bool BaseSelected = false;
bool TopSelected = false;
bool Arm1Selected = false;
bool Arm2Selected = false;
bool PenSelected = false;
bool ShiftPressed = false;
bool MousePressed = false;
bool IsObjectActive[8];
// Transformations
glm::vec3 J0_BaseTranslate;
float J1_TopRotate = 0.0f; // J1
float J2_Arm1Rotate = 0.0f; // J2
float J3_Arm2Rotate = 0.0f; // J3
float J4_PenRotateLongitude = 0.0f; // J4
float J5_PenRotateLatitude = 0.0f; // J5
float J6_PenRotateAxis = 0.0f; // J6

int initWindow(void) {
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// FOR MAC

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Ajoy,Nicole(30795334)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void) {
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// TL
	// Define objects
	createObjects();

	// ATTN: create VAOs for each of the newly created objects here:
	VertexBufferSize[0] = sizeof(CoordVerts);
	NumVerts[0] = CoordVertsCount;
	createVAOs(CoordVerts, NULL, 0);

	VertexBufferSize[1] = sizeof(GridVerts);
	NumVerts[1] = GridVertsIndexCount;
	createVAOs(GridVerts, NULL, 1);
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {
	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);	// TL

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
}

// Ensure your .obj files are in the correct format and properly loaded by looking at the following function
void loadObject(char* file, glm::vec4 color, Vertex* &out_Vertices, GLushort* &out_Indices, int ObjectId) {
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();
	//std::cout << "objectId: " << ObjectId << " | vertCount: " << vertCount << " | idxCount: " << idxCount << std::endl;

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
		//RobotArmVerts[ObjectId - 2][i].SetPosition(&indexed_vertices[i].x);
		//RobotArmVerts[ObjectId - 2][i].SetNormal(&indexed_normals[i].x);
		//RobotArmVerts[ObjectId - 2][i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
		//RobotArmIndices[i] = i;
	}

	// set global variables!!
	NumIdcs[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}

void createObjects(void) {
	//-- COORDINATE AXES --//
	CoordVerts[0] = { { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[1] = { { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[2] = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[3] = { { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[4] = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[5] = { { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	
	//-- GRID --//
	
	// ATTN: Create your grid vertices here!
	int gridVertCounter = 0;
	for (int x = -5; x <= 5; x++) {
		float v1[4] = { x, 0, -5, 1.0f };
		GridVerts[gridVertCounter].SetPosition(v1);
		GridVerts[gridVertCounter].SetColor(white);
		float v2[4] = { x, 0, 5, 1.0f };
		GridVerts[gridVertCounter + 1].SetPosition(v2);
		GridVerts[gridVertCounter + 1].SetColor(white);
		gridVertCounter += 2;
	}
	for (int z = -5; z <= 5; z++) {
		float v1[4] = { -5, 0, z, 1.0f };
		GridVerts[gridVertCounter].SetPosition(v1);
		GridVerts[gridVertCounter].SetColor(white);
		float v2[4] = { 5, 0, z, 1.0f };
		GridVerts[gridVertCounter + 1].SetPosition(v2);
		GridVerts[gridVertCounter + 1].SetColor(white);
		gridVertCounter += 2;
	}
	
	//-- .OBJs --//

	// ATTN: Load your models here through .obj files -- example of how to do so is as shown
	Vertex* Verts;
	GLushort* Idcs;
	
	// Load the original colors first
	loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, baseIndexStandardColor);
	createVAOs(Verts, Idcs, baseIndexStandardColor);
	loadObject("models/top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), Verts, Idcs, topIndexStandardColor);
	createVAOs(Verts, Idcs, topIndexStandardColor);
	loadObject("models/arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), Verts, Idcs, arm1IndexStandardColor);
	createVAOs(Verts, Idcs, arm1IndexStandardColor);
	loadObject("models/joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, jointIndexStandardColor);
	createVAOs(Verts, Idcs, jointIndexStandardColor);
	loadObject("models/arm2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts, Idcs, arm2IndexStandardColor);
	createVAOs(Verts, Idcs, arm2IndexStandardColor);
	loadObject("models/pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts, Idcs, penIndexStandardColor);
	createVAOs(Verts, Idcs, penIndexStandardColor);
	loadObject("models/button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, buttonIndexStandardColor);
	createVAOs(Verts, Idcs, buttonIndexStandardColor);

	// Load with the lighter colors (this is messy, sorry, but it seems to work)
	loadObject("models/base.obj", glm::vec4(1.0, 0.75, 0.75, 1.0), Verts, Idcs, baseIndexHighlightedColor);
	createVAOs(Verts, Idcs, baseIndexHighlightedColor);
	loadObject("models/top.obj", glm::vec4(0.75, 1.0, 0.75, 1.0), Verts, Idcs, topIndexHighlightedColor);
	createVAOs(Verts, Idcs, topIndexHighlightedColor);
	loadObject("models/arm1.obj", glm::vec4(0.75, 0.75, 1.0, 1.0), Verts, Idcs, arm1IndexHighlightedColor);
	createVAOs(Verts, Idcs, arm1IndexHighlightedColor);
	loadObject("models/arm2.obj", glm::vec4(0.75, 1.0, 1.0, 1.0), Verts, Idcs, arm2IndexHighlightedColor);
	createVAOs(Verts, Idcs, arm2IndexHighlightedColor);
	loadObject("models/pen.obj", glm::vec4(1.0, 1.0, 0.75, 1.0), Verts, Idcs, penIndexHighlightedColor);
	createVAOs(Verts, Idcs, penIndexHighlightedColor);

	// Load light cubes with emmision material
	//loadObject("models/lightcube1.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts, Idcs, lightCube1Index);
	//createVAOs(Verts, Idcs, lightCube1Index);
	//loadObject("models/lightcube2.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts, Idcs, lightCube2Index);
	//createVAOs(Verts, Idcs, lightCube2Index);
}

void pickObject(void) {
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);
	
	if (gPickedIndex == 255) { // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void renderScene(void) {
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		//if (CameraSelected) {
		if (IsObjectActive[0]) {
			computeMatricesFromInputs();
			//gProjectionMatrix = getProjectionMatrix();
			gViewMatrix = getViewMatrix();
		}
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glm::vec3 lightPos1 = getCameraPosition();
		glUniform3f(LightID, lightPos1.x + 5, lightPos1.y, lightPos1.z);

		glm::vec3 lightPos2 = getCameraPosition();
		glUniform3f(LightID, lightPos2.x - 5, lightPos2.y, lightPos2.z);

		glBindVertexArray(VertexArrayId[0]);	// Draw CoordAxes
		glDrawArrays(GL_LINES, 0, NumVerts[0]);

		glBindVertexArray(VertexArrayId[1]);	// Draw Grid
		glDrawArrays(GL_LINES, 0, NumVerts[1]);
		
		// Draw base
		//if (BaseSelected) {
		if (IsObjectActive[baseIndexStandardColor]) {
			J0_BaseTranslate = UpdateBaseTranslate();
		}
		ModelMatrix = glm::translate(ModelMatrix, J0_BaseTranslate);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		if (!IsObjectActive[baseIndexStandardColor]) {
			glBindVertexArray(VertexArrayId[baseIndexStandardColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[baseIndexStandardColor], GL_UNSIGNED_SHORT, (void*)0);
		}
		else {
			glBindVertexArray(VertexArrayId[baseIndexHighlightedColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[baseIndexHighlightedColor], GL_UNSIGNED_SHORT, (void*)0);
		}
		
		// Draw top
		//if (TopSelected) {
		if (IsObjectActive[topIndexStandardColor]) {
			J1_TopRotate = UpdateTopRotate();
		}
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, degrees(J1_TopRotate), glm::vec3(0.0, 1.0, 0.0));
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		if (!IsObjectActive[topIndexStandardColor]) {
			glBindVertexArray(VertexArrayId[topIndexStandardColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[topIndexStandardColor], GL_UNSIGNED_SHORT, (void*)0);
		}
		else {
			glBindVertexArray(VertexArrayId[topIndexHighlightedColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[topIndexHighlightedColor], GL_UNSIGNED_SHORT, (void*)0);
		}

		// Draw arm1
		//if (Arm1Selected) {
		if (IsObjectActive[arm1IndexStandardColor]) {
			J2_Arm1Rotate = UpdateArm1Rotate();
		}
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.4f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, degrees(J2_Arm1Rotate), glm::vec3(1.0f, 0.0f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		if (!IsObjectActive[arm1IndexStandardColor]) {
			glBindVertexArray(VertexArrayId[arm1IndexStandardColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[arm1IndexStandardColor], GL_UNSIGNED_SHORT, (void*)0);
		}
		else {
			glBindVertexArray(VertexArrayId[arm1IndexHighlightedColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[arm1IndexHighlightedColor], GL_UNSIGNED_SHORT, (void*)0);
		}

		// Draw joint
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 1.25f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glBindVertexArray(VertexArrayId[jointIndexStandardColor]);
		glDrawElements(GL_TRIANGLES, NumIdcs[jointIndexStandardColor], GL_UNSIGNED_SHORT, (void*)0);

		// Draw arm2
		//if (Arm2Selected) {
		if (IsObjectActive[arm2IndexStandardColor]) {
			J3_Arm2Rotate = UpdateArm2Rotate();
		}
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, degrees(J3_Arm2Rotate), glm::vec3(1.0f, 0.0f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		if (!IsObjectActive[arm2IndexStandardColor]) {
			glBindVertexArray(VertexArrayId[arm2IndexStandardColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[arm2IndexStandardColor], GL_UNSIGNED_SHORT, (void*)0);
		}
		else {
			glBindVertexArray(VertexArrayId[arm2IndexHighlightedColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[arm2IndexHighlightedColor], GL_UNSIGNED_SHORT, (void*)0);
		}

		// Draw pen
		//if (PenSelected) {
		if (IsObjectActive[penIndexStandardColor]) {
			if (!ShiftPressed) {
				J4_PenRotateLongitude = UpdatePenLongitudeRotate();
				J5_PenRotateLatitude = UpdatePenLatitudeRotate();
			}
			else {
				J6_PenRotateAxis = UpdatePenAxisRotate();
			}
		}
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, degrees(J4_PenRotateLongitude), glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, degrees(J5_PenRotateLatitude), glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, degrees(J6_PenRotateAxis), glm::vec3(0.0f, 1.0f, 0.0f));
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		if (!IsObjectActive[penIndexStandardColor]) {
			glBindVertexArray(VertexArrayId[penIndexStandardColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[penIndexStandardColor], GL_UNSIGNED_SHORT, (void*)0);
		}
		else {
			glBindVertexArray(VertexArrayId[penIndexHighlightedColor]);
			glDrawElements(GL_TRIANGLES, NumIdcs[penIndexHighlightedColor], GL_UNSIGNED_SHORT, (void*)0);
		}

		// Draw button
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.25f, 0.1f));
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glBindVertexArray(VertexArrayId[buttonIndexStandardColor]);
		glDrawElements(GL_TRIANGLES, NumIdcs[buttonIndexStandardColor], GL_UNSIGNED_SHORT, (void*)0);
			
		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void cleanup(void) {
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

void setActive(int activeIndex) {
	if (IsObjectActive[activeIndex]) {
		IsObjectActive[activeIndex] = false;
	}
	else {
		for (int i = 0; i < 9; i++) {
			IsObjectActive[i] = (i == activeIndex ? true : false);
		}
	}
}

// Alternative way of triggering functions on keyboard events
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_A:
				break;
			case GLFW_KEY_D:
				break;
			case GLFW_KEY_W:
				break;
			case GLFW_KEY_S:
				break;
			case GLFW_KEY_SPACE:
				break;
			// Task 2: Camera rotations
			case GLFW_KEY_C: {
				//CameraSelected = !CameraSelected;
				//std::cout << "CameraSelected bool: " << CameraSelected << std::endl;
				setActive(0);
				break;
			}
			// Task 4: transformations
			case GLFW_KEY_B: {
				//BaseSelected = !BaseSelected;
				//std::cout << "BaseSelected bool: " << BaseSelected << std::endl;
				setActive(baseIndexStandardColor);
				break;
			}
			case GLFW_KEY_T: {
				//TopSelected = !TopSelected;
				//std::cout << "TopSelected bool: " << TopSelected << std::endl;
				setActive(topIndexStandardColor);
				break;
			}
			case GLFW_KEY_1: {
				//Arm1Selected = !Arm1Selected;
				//std::cout << "Arm1Selected bool: " << Arm1Selected << std::endl;
				setActive(arm1IndexStandardColor);
				break;
			}
			case GLFW_KEY_2: {
				//Arm2Selected = !Arm2Selected;
				//std::cout << "Arm2Selected bool: " << Arm2Selected << std::endl;
				setActive(arm2IndexStandardColor);
				break;
			}
			case GLFW_KEY_P: {
				//PenSelected = !PenSelected;
				//std::cout << "PenSelected bool: " << PenSelected << std::endl;
				setActive(penIndexStandardColor);
				break;
			}
			case GLFW_KEY_LEFT_SHIFT: {
				ShiftPressed = !ShiftPressed;
				//std::cout << "ShiftPressed bool: " << ShiftPressed << std::endl;
			}
			default:
				break;
		}
	}
}

// Alternative way of triggering functions on mouse click events
static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		MousePressed = true;
		pickObject();
	}
	else {
		MousePressed = false;
	}
}

int main(void) {
	// TL
	// ATTN: Refer to https://learnopengl.com/Getting-started/Transformations, https://learnopengl.com/Getting-started/Coordinate-Systems,
	// and https://learnopengl.com/Getting-started/Camera to familiarize yourself with implementing the camera movement

	// ATTN (Project 3 only): Refer to https://learnopengl.com/Getting-started/Textures to familiarize yourself with mapping a texture
	// to a given mesh

	// Initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// Initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}


		// DRAWING POINTS
		renderScene();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}