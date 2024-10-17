//
// AVT: Phong Shading and Text rendered with FreeType library
// The text rendering was based on https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
// 
// Author: João Madeiras Pereira
//

#include <math.h>
#include <iostream>
#include <sstream>
#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>


// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"

#include "avtFreeType.h"
#include "Texture_Loader.h"

using namespace std;

#define CAPTION "AVT Demo: Phong Shading and Text rendered with FreeType"
int WindowHandle = 0;
int WinX = 1024, WinY = 768;

unsigned int FrameCount = 0;

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

//Vector with meshes
vector<struct MyMesh> myMeshes;

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;

GLint sunPos_uniformId;				// Sun light world position
GLint buoyNumber_uniformId;

GLint buoyPos_uniformId[buoyNumber];

GLint buoyConstantAttenuation_unirformId; 
GLint buoyLinearAttenuation_unirformId;
GLint buoyQuadraticAttenuation_unirformId;

GLint headlightPos_uniformId;		// Boat headlight world position
GLint headlightPos2_uniformId;		// Boat headlight 2 world position
GLint headlightDir_uniformId;		// Boat headlight pointing diretion
GLint headlightDir2_uniformId;		// Boat headlight 2 world position
GLint headlightAngle_uniformId;		// Boat headlight angle
GLint headlightExp_uniformId;		// Boat headlight exponent

GLint isSunActive_uniformId;
GLint isBuoyLightsActive_uniformId;
GLint isHeadlightsActive_uniformId;

GLint depthFog_uniformId;	// Fog controller

GLuint TextureArray[3];

GLint tex_loc, tex_loc1, tex_loc2;
GLint texMode_uniformId;

// Camera Position
float camX, camY, camZ;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];

float sunLightPos[4] = {-5.0f, 5.0f, 15.0f, 1.0f};		// Sun light world position

float *buoyLightPos[buoyNumber];
// float buoyLightPos[4] =	 { 10.0f, 5.0f, 10.0f, 1.0f };	// Buoy lights world position
// float buoyLightPos2[4] = { -15.0f, 5.0f, -15.0f, 1.0f };
// float buoyLightPos3[4] = { 5.0f, 6.0f, -5.0f, 1.0f };
// float buoyLightPos4[4] = { -5.0f, 6.0f, 5.0f, 1.0f };
// float buoyLightPos5[4] = { -15.0f, 6.0f, 15.0f, 1.0f };
// float buoyLightPos6[4] = { 15.0f, 6.0f, 15.0f, 1.0f };

float buoyLightConstantAttenuation = 2.0f;
float buoyLightLinearAttenuation = 0.5f;
float buoyLightQuadraticAttenuation = 0.7f;

bool isSunActive = 1;
bool isBuoyLightsActive = 0;
bool isHeadlightsActive = 0;

bool depthFog = 0;

//number of objects to be drawn
int objectNumber = 0;

//pause variable
bool isPaused = false;

class Obstacle {
public:
	float center[3] = { 0.0f, 0.0f, 0.0f };
	float radius = 0.0f;
};

//
const int houseNumber = 1000 * 2 * 2; //*2 because 1 house is 1 roof + 1 base and another because its 2 sides of the race
const int buoyNumber = 1000;

// number of obstacles that have bounding boxes
const int obstacleNumber = houseNumber + buoyNumber +2*2; //+2*2 because we add 2 houses in the back manually
Obstacle obstacles[obstacleNumber];

int activeCam = 0;

//variable used to switch speeds using 'o'
int speedSwitch = 0;

const float radius = 5.0f;       // Distance from the center of the scene
const float maxDistance = 15.0f; // Max distance from the center before "death"
const float speedMultiplier = 0.02f;
const float shakeAmplitude = 0.05f;
const float shakeFrequency = 5.0f;

class Camera {
public:
	float pos[3] = { 0, 0, 0 };
	float target[3] = { 0, 0, 0 };
	int type = 0;
};

Camera cams[3];

class Boat {
public:
	float speed = 0.0f;
	float acceleration = 0.0f;
	float pos[3] = { 0.0f, 0.0f, 0.0f };
	float angle = 90.0f;
	float direction = 90.0f;
	const float bb_radius = sqrt(0.75);
	float bb_center[3] = { 0.5f, 0.5f, 0.5f };
	int lives = 4;
};

Boat boat;

float deltaT = 0.05f;
float decayy = 0.1f;

// Structure to store data for each shark fin (cone)
struct SharkFin {
	//fin speed and position
	float speed = 0.0f;
	float pos[3] = { 1000.0f, 1000.0f, 0.0f };
	//fin angle and fin direction, angle changes instantly when a or d are pressed and direction gradually changes to that value
	float angle = 0.0f;
	float direction = 0.0f;
	//the slope of the line and initial position used to make the pathing of the fins
	float slope = 0.0f;
	float initialPos[3] = { 0.0f, 0.0f, 0.0f };
	float targetPos[3] = { 0.0f, 0.0f, 0.0f };
	// radius and center for the sphere
	const float bb_radius = 0.5f;
	float bb_center[3] = { 0.0f, 0.0f, 0.0f };
};
// Shark fins
const int sharkfinNumber = 1;
SharkFin fins[sharkfinNumber];

const float duration = 30.0f;  // 30 seconds duration
float elapsedTime = 0.0f;     
int difficulty = 0;

// Function to generate a random float value between min and max
float randomFloat(float min, float max) {
	return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
}

bool isColliding(float radius1, float *center1, float radius2, float *center2) {
	// get the distance between the centers sqrt((x1-x2)^2 + (y1-y2)^2 + (z1-z2)^2)
	float center_diff = sqrt(pow(center1[0] - center2[0], 2) + pow(center1[1] - center2[1], 2) + pow(center1[2] - center2[2], 2));
	//get the total length of both radii combined
	float radiusLength = radius1 + radius2;

	if (center_diff < radiusLength) { return true; }
	else { return false; };
}

void handleCollisionFin(int index)
{
	//reduce number of lives on collision with sharks
	boat.lives--;
	//if life gets to or below 0, reset lives and position to starting location
	if (boat.lives <= 0) {
		boat.lives = 4;
		boat.pos[0] = 0.0f;
		boat.pos[1] = 0.0f;
		boat.speed = 0.0f;
		boat.acceleration = 0.0f;
		boat.angle = 0.0f;
		boat.direction = 0.0f;
		return;
	}
	//vector that dictates what direction the objects will go when they collide
	float centerVector[3] = { boat.bb_center[0] - fins[index].bb_center[0], boat.bb_center[1] - fins[index].bb_center[1], boat.bb_center[2] - fins[index].bb_center[2] };
	normalize(centerVector);

	//vector that stores boat speed.x, speed.y and speed.z (always 0)
	float boatdirectionalSpeed[3] = { boat.speed * cos(boat.direction), boat.speed * sin(boat.direction), 0.0f };
	//vector that stores fin speed.x, speed.y and speed.z (always 0)
	float findirectionalSpeed[3] = { fins[index].speed * cos(fins[index].angle), fins[index].speed * sin(fins[index].angle), 0.0f };
	//relative speed of the collision expressed in a 3d vector
	float relativeSpeed[3] = { boatdirectionalSpeed[0] - findirectionalSpeed[0], boatdirectionalSpeed[1] - findirectionalSpeed[1], boatdirectionalSpeed[2] - findirectionalSpeed[2] };

	//final speed of the collision considering the direction of the collision
	float collisionSpeed = dotProduct(relativeSpeed, centerVector);
	//now that the speed is calculated we must apply it to both objects
	while (isColliding(boat.bb_radius, boat.bb_center, fins[index].bb_radius, fins[index].bb_center))
	{
		boat.bb_center[0] += 0.01f * centerVector[0];
		boat.pos[0] += 0.01f * centerVector[0];
		boat.bb_center[1] += 0.01f * centerVector[0];
		boat.pos[1] += 0.01f * centerVector[1];
	}

	boat.acceleration = -5.0f;
	boat.speed = -collisionSpeed / 5;
	//boat.direction = (atan2(centerVector[0], centerVector[2])*180)/3.1415;
	//boat.angle = (atan2(centerVector[0], centerVector[2]) * 180) / 3.1415;
}

void handleCollisionStatic(int index)
{
	//vector that dictates what direction the objects will go when they collide
	float centerVector[3] = { boat.bb_center[0] - obstacles[index].center[0], boat.bb_center[1] - obstacles[index].center[1], boat.bb_center[2] - obstacles[index].center[2] };
	normalize(centerVector);
	
	//relative speed of the collision expressed in a 3d vector
	float relativeSpeed[3] = { boat.speed * cos(boat.direction), boat.speed * sin(boat.direction), 0.0f };

	//final speed of the collision considering the direction of the collision
	float collisionSpeed = dotProduct(relativeSpeed, centerVector);
	//now that the speed is calculated we must apply it to both objects
	while (isColliding(boat.bb_radius, boat.bb_center, obstacles[index].radius, obstacles[index].center))
	{
		boat.bb_center[0] += 0.01f * centerVector[0];
		boat.pos[0] += 0.01f * centerVector[0];
		boat.bb_center[1] += 0.01f * centerVector[0];
		boat.pos[1] += 0.01f * centerVector[1];
	}

	boat.acceleration = -5.0f;
	boat.speed = -collisionSpeed / 5;
	//boat.direction = (atan2(centerVector[0], centerVector[2])*180)/3.1415;
	//boat.angle = (atan2(centerVector[0], centerVector[2]) * 180) / 3.1415;
}

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;

	// Boat movement logic
	boat.pos[0] += ((boat.speed * deltaT) + (1 / 2 * boat.acceleration * pow(deltaT, 2))) * cos(boat.direction * 3.14 / 180);
	boat.pos[1] += ((boat.speed * deltaT) + (1 / 2 * boat.acceleration * pow(deltaT, 2))) * sin(boat.direction * 3.14 / 180);
	
	//pass deltaT to seconds
	//printf("%f\n", elapsedTime);
	float dt = (1 / deltaT) / 600.0f;
	elapsedTime += dt;

	// After 30 seconds, set progress to 1
	// After 30 seconds, set progress to 1
	if (elapsedTime >= duration) {
		difficulty = 1;
	}
	
	for (int i = 0; i < sharkfinNumber; i++)
	{
		if ((abs(fins[i].pos[0] - fins[i].initialPos[0]) > 10.0f) || ((abs(fins[i].pos[1] - fins[i].initialPos[1])) > 10.0f))
		{
			fins[i].initialPos[0] = (float)(10.0 * cos(rand())) + boat.pos[0];
			fins[i].initialPos[1] = (float)(10.0 * sin(rand())) + boat.pos[1];
			fins[i].targetPos[0] = boat.pos[0];
			fins[i].targetPos[1] = boat.pos[1];
			fins[i].slope = ((fins[i].initialPos[1] - boat.pos[1]) / (fins[i].initialPos[0] - boat.pos[0]));
			fins[i].slope = fins[i].slope / abs(fins[i].slope);
			fins[i].angle = atan(fins[i].slope);
			fins[i].pos[0] = fins[i].initialPos[0];
			fins[i].pos[1] = fins[i].initialPos[1];
			fins[i].direction = -(180/3.1415) * atan2(boat.pos[1] - fins[i].pos[1], boat.pos[0] - fins[i].pos[0]);
		}
		

		// Keep the angle between 0 and 360 degrees
		if (fins[i].angle > 3.14) { fins[i].angle -= 3.14f; }
		else if (fins[i].angle < -3.14f) { fins[i].angle += 3.14f; }

		//fins movement logic
		// we want them to move in a straight line, starting at their spawn point moving towards the boat position
		if (fins[i].initialPos[0] < fins[i].targetPos[0])
		{
			fins[i].pos[0] += -(fins[i].slope * deltaT) * cos(fins[i].angle);  // X position
			fins[i].pos[1] += -(fins[i].slope * deltaT) * sin(fins[i].angle);  // Y position
		}
		else
		{
			fins[i].pos[0] += (fins[i].slope * deltaT) * cos(fins[i].angle);  // X position
			fins[i].pos[1] += (fins[i].slope * deltaT) * sin(fins[i].angle);  // Y position
		}
		

		fins[i].bb_center[0] = fins[i].pos[0];
		fins[i].bb_center[1] = fins[i].pos[1];
	}

	if (boat.acceleration > 5.0f)
	{
		boat.acceleration = 5.0f;
	}

	//boat acceleration reduction
	//boat moving forwards (press 'a' or 'd')
	if ((boat.acceleration > 0) && (boat.speed >= 0))
	{
		boat.acceleration -= 3*decayy;
		boat.speed += boat.acceleration * deltaT;
	}
	else if ((boat.acceleration < 0) && (boat.speed > 0))
	{
		boat.acceleration -= 2*decayy;
		boat.speed += boat.acceleration * deltaT;
		if (boat.speed <= 0)
		{
			boat.acceleration = 0;
			boat.speed = 0;
		}
	}
	

	//boat moving backwards (press 's')
	if ((boat.acceleration < 0) && (boat.speed <= 0))
	{
		boat.acceleration += 3*decayy;
		boat.speed += boat.acceleration * deltaT;
	}
	else if ((boat.acceleration >= 0) && (boat.speed < 0))
	{
		boat.acceleration += 2*decayy;
		boat.speed += boat.acceleration * deltaT;
		if (boat.speed >= 0)
		{
			boat.acceleration = 0;
			boat.speed = 0;
		}
	}
	if (boat.speed > 10.0f)
	{
		boat.speed = 10.0f;
	}
	

	boat.bb_center[0] = boat.pos[0] + (sqrt(2) / 2) * cos(((boat.direction * 3.1415) / 180) + 3.1415 / 4);
	boat.bb_center[1] = boat.pos[1] + (sqrt(2) / 2) * sin(((boat.direction * 3.1415) / 180) + 3.1415 / 4);

	//check for collision between boat and fins
	for (int i = 0; i < sharkfinNumber; i++)
	{
		if (isColliding(boat.bb_radius, boat.bb_center, fins[i].bb_radius, fins[i].bb_center))
		{
			handleCollisionFin(i);
		}
	}
	//check for collision in static objects
	for (int i = 0; i < obstacleNumber; i++)
	{
		if (isColliding(boat.bb_radius, boat.bb_center, obstacles[i].radius, obstacles[i].center))
		{
			handleCollisionStatic(i);
		}
	}


	//handle boat angle incremental increase, to make the rotation animation
	if ((boat.angle - boat.direction) > 0)
	{
		boat.direction += decayy * (4 - (3.0f * speedSwitch));
		if ((boat.direction > boat.angle) && (boat.speed == 0) || (boat.speed == 0))
		{
			boat.angle = boat.direction;
		}
	}
	else if ((boat.angle - boat.direction) < 0)
	{
		boat.direction -= decayy * (4 - (3.0f * speedSwitch));
		if ((boat.direction < boat.angle) && (boat.speed == 0) || (boat.speed == 0))
		{
			boat.angle = boat.direction;
		}
	}
	else if (boat.angle == 0)
	{
		if (boat.direction > boat.angle)
		{
			boat.direction -= decayy * (4 - (3.0f * speedSwitch));
		}
		else if (boat.direction < boat.angle)
		{
			boat.direction += decayy * (4 - (3.0f * speedSwitch));
		}
	}
	if ((boat.acceleration < 0) && (boat.speed <= 0)) //when the boat starts accelerating backwards
	{
		boat.acceleration += 7*decayy;
		boat.speed += boat.acceleration * deltaT;
	}
	else if ((boat.acceleration >= 0) && (boat.speed < 0)) // when the boat starts decelerating
	{
		boat.acceleration += 6*decayy;
		boat.speed += boat.acceleration * deltaT;
		if (boat.speed >= 0)
		{
			boat.acceleration = 0;
			boat.speed = 0;
		}
	}

	//handle boat angle incremental increase, to make the rotation animation
	if (abs(boat.direction - boat.angle) < 0.1) //this helps deal with differences that arent divisble by decayy, example is boat.angle = 65.99043 boat.direction = 65.99057
	{
		boat.angle = boat.direction;
	}
	if ((boat.angle - boat.direction) > 0) //dealing with angle differences from sides 'a' and 'd'
	{
		boat.direction += decayy * (6 - (1.0f * speedSwitch));
		if (boat.speed == 0) //when the boat stops going forward we want to make sure it stops rotating
		{
			boat.angle = boat.direction;
		}
	}
	else if ((boat.angle - boat.direction) < 0)
	{
		boat.direction -= decayy * (6 - (1.0f * speedSwitch));
		if ((boat.direction < boat.angle) && (boat.speed == 0) || (boat.speed == 0))
		{
			boat.angle = boat.direction;
		}
	}
	else if (boat.angle == 0)
	{
		if (boat.direction > boat.angle)
		{
			boat.direction -= decayy * (6 - (1.0f * speedSwitch));
		}
		else if (boat.direction < boat.angle)
		{
			boat.direction += decayy * (6 - (1.0f * speedSwitch));
		}
	}


	glutTimerFunc(1 / deltaT, timer, 0);
}

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);
}

void renderScene(void) {

	GLint loc;

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	float headlightPos[4] = { boat.pos[0], 1.0f, boat.pos[1], 1.0f};			// Boat headlight world position (spotlight)
	float headlightPos2[4] = { -1.0f + boat.pos[0], 1.0f, boat.pos[1], 1.0f};	// Boat headlight 2 world postion (spotlight)

	float headlightDir[4] = { 0.0f, 0.0f, 1.0f, 0.0f };		// Spotlight pointing diretion 
	float headlightDir2[4] = { 0.0f, 0.0f, 1.0f, 0.0f };	// Spotlight 2 pointing diretion

	float headlightAngle = 0.9;		// Spotlight angle (0-0.9999)

	float headlightExp = 0.02f;		// Headlights quality

	cams[0].pos[0] = camX + boat.bb_center[0];
	cams[0].pos[1] = camY + boat.bb_center[2];
	cams[0].pos[2] = camZ + boat.bb_center[1];

	cams[1].type = 1;
	cams[1].pos[1] = 20;

	cams[2].pos[1] = 20;

	// Follow Cam
	if (activeCam == 0) {
		// set the camera using a function similar to gluLookAt
		lookAt(cams[0].pos[0], cams[0].pos[1], cams[0].pos[2], boat.bb_center[0], boat.bb_center[2], boat.bb_center[1], 0, 1, 0);
	}
	// Static Ortho Cam
	else if (activeCam == 1) {
		lookAt(cams[1].pos[0], cams[1].pos[1], cams[1].pos[2], 0, 0, 0, 0, 0, 1);
	}
	// Static Perspective 
	else if (activeCam == 2) {
		lookAt(cams[2].pos[0], cams[2].pos[1], cams[2].pos[2], 0, 0, 0, 0, 0, 1);
	}

	GLint m_view[4];

	glGetIntegerv(GL_VIEWPORT, m_view);

	float ratio = (float)(m_view[2] - m_view[0]) / (m_view[3] - m_view[1]);

	loadIdentity(PROJECTION);

	if (cams[activeCam].type == 0) {
		perspective(63.13, ratio, 1, 100);
	}
	else {
		ortho(ratio * -25, ratio * 25, -25, 25, 0.1, 100);
	}

	// use our shader

	glUseProgram(shader.getProgramIndex());

	float *res[buoyNumber];

	for (int i = 0; i < buoyNumber; i++)
	{
		multMatrixPoint(VIEW, buoyLightPos[i], res[i]);
		glUniform4fv(buoyPos_uniformId[i], 1, res[i]);
	}
	float res2[4];		// Spotlight world position 
	float res3[4];		// Spotlight 2 world position
	float res4[4];		// Spotlight poiting diretion
	float res5[4];		// Spotlight 2 poiting diretion

	multMatrixPoint(VIEW, sunLightPos, res);		// sunLightPos definido em World Coord so is converted to eye space
		
	multMatrixPoint(VIEW, headlightPos, res2);		// headlightPos definido em World Coord so is converted to eye space
	multMatrixPoint(VIEW, headlightPos2, res3);		// headlightPos2 definido em World Coord so is converted to eye space
	multMatrixPoint(VIEW, headlightDir, res4);		// headlightDir definido em World Coord so is converted to eye space
	multMatrixPoint(VIEW, headlightDir2, res5);		// headlightDir2 definido em World Coord so is converted to eye space

	glUniform4fv(sunPos_uniformId, 1, res);

	glUniform1f(buoyConstantAttenuation_unirformId, buoyLightConstantAttenuation);
	glUniform1f(buoyLinearAttenuation_unirformId, buoyLightLinearAttenuation);
	glUniform1f(buoyQuadraticAttenuation_unirformId, buoyLightQuadraticAttenuation);

	glUniform4fv(headlightPos_uniformId, 1, res2);
	glUniform4fv(headlightPos2_uniformId, 1, res3);
	glUniform4fv(headlightDir_uniformId, 1, res4);
	glUniform4fv(headlightDir2_uniformId, 1, res5);

	glUniform1f(headlightAngle_uniformId, headlightAngle);
	glUniform1f(headlightExp_uniformId, headlightExp);

	glUniform1f(isSunActive_uniformId, isSunActive);
	glUniform1f(isBuoyLightsActive_uniformId, isBuoyLightsActive);
	glUniform1f(isHeadlightsActive_uniformId, isHeadlightsActive);

	glUniform1f(depthFog_uniformId, depthFog);
	glUniform1f(buoyNumber_uniformId, buoyNumber);


	int objId = 0;

	//Associar os Texture Units aos Objects Texture
	//stone.tga loaded in TU0; checker.tga loaded in TU1;  lightwood.tga loaded in TU2

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);


	int stone = 0;		// IDs das texturas
	int checker = 1;
	int wood = 2;

	//Indicar aos tres samplers do GLSL quais os Texture Units a serem usados
	glUniform1i(tex_loc, stone);
	glUniform1i(tex_loc1, checker);
	glUniform1i(tex_loc2, wood);

	glUniform1i(texMode_uniformId, wood);

	//render the first few opaque objects (water,boat, paddle, 2 houses...)
	for (int i = 0; i < objectNumber; ++i) {

		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[objId].mat.shininess);
		pushMatrix(MODEL);

		// """water"""
		if (i == 0) {
			glUniform1i(texMode_uniformId, wood);
			rotate(MODEL, -90.0f, 1.0f, 0.0f, 0.0f);
		}
		// boat
		else if (i == 1)
		{
			glUniform1i(texMode_uniformId, wood);
			translate(MODEL, boat.pos[0] - 0.0f, boat.pos[2], boat.pos[1] - 0.0f);
			rotate(MODEL, -boat.direction, 0.0f, 1.0f, 0.0f);
		}
		//handle of the paddle
		else if (i == 2) {
			glUniform1i(texMode_uniformId, wood);
			translate(MODEL, 0.5f, 1.2f, 0.0f);
			rotate(MODEL, 90.0f, 0.0f, 0.0f, 1.0f);
		}
		//head1 of the paddle
		else if (i == 3) {
			glUniform1i(texMode_uniformId, wood);
			translate(MODEL, -0.4f, 1.2f, 0.0f);
			rotate(MODEL, -90.0f, 0.0f, 0.0f, 1.0f);
		}
		//head2 of the paddle
		else if (i == 4) {
			glUniform1i(texMode_uniformId, wood);
			translate(MODEL, 1.4f, 1.2f, 0.0f);
			rotate(MODEL, 90.0f, 0.0f, 0.0f, 1.0f);
		}
		//house 1 base
		else if (i == 5) {
			glUniform1i(texMode_uniformId, stone);
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, (obstacles[houseNumber + 1].center[0] / 2) - 0.5f, (obstacles[houseNumber + 1].center[2] / 2) - 0.5f, (obstacles[houseNumber + 1].center[1] / 2) - 0.5f);
		}
		//house 1 roof
		else if (i == 6) {
			glUniform1i(texMode_uniformId, wood);
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, (obstacles[houseNumber + 1].center[0] / 2), (obstacles[houseNumber + 1].center[2]), (obstacles[houseNumber + 1].center[1] / 2));
			rotate(MODEL, 45.0, 0.0f, 1.0f, 0.0f);
		}
		//house 2 base
		else if (i == 7) {
			glUniform1i(texMode_uniformId, stone);
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, (obstacles[houseNumber + 2].center[0] / 2) - 0.5f, (obstacles[houseNumber + 2].center[2] / 2) - 0.5f, (obstacles[houseNumber + 2].center[1] / 2) - 0.5f);
		}
		//house 2 roof
		else if (i == 8) {
			glUniform1i(texMode_uniformId, wood);
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, (obstacles[houseNumber + 2].center[0] / 2), (obstacles[houseNumber + 2].center[2]), (obstacles[houseNumber + 2].center[1] / 2));
			rotate(MODEL, 45.0, 0.0f, 1.0f, 0.0f);
		}

		
		
		
		
		
		
		/*
		//base of 7th house 
		else if (i == 14) {
			glUniform1i(texMode_uniformId, stone);
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, (obstacles[6].center[0] / 2) - 0.5f, (obstacles[6].center[2] / 2) - 0.5f, (obstacles[6].center[1] / 2) - 0.5f);
		}
		//roof of 7th house 
		else if (i == 15) {
			glUniform1i(texMode_uniformId, wood);
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, -4.0f, 1.0f, 7.0f);
			rotate(MODEL, 45.0, 0.0f, 1.0f, 0.0f);
		}
		//1st buoy 
		else if (i == 16) {
			translate(MODEL, (obstacles[7].center[0] / 1) - 0.0f, (obstacles[7].center[2] / 1) - 1.0f, (obstacles[7].center[1] / 1) - 0.0f);
		}
		*/
	
		// Initial settings for fins' speed and direction:
		for (int j = 0; j < sharkfinNumber; j++)
		{
			if (difficulty == 0) { fins[j].speed = 30.0f; }
			else if (difficulty == 1) { fins[j].speed = 60.0f; }
		}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(myMeshes[objId].vao);

		glDrawElements(myMeshes[objId].type, myMeshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		objId++;
	}

	//render the houses on eachside based on houseNumber
	for (int j = 0; j < houseNumber; j++) //*2 because a house is a base and a roof
	{
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[objId].mat.shininess);
		glUniform1i(texMode_uniformId, checker);

		pushMatrix(MODEL);
		//if base
		if ((j % 2) == 0)
		{
			glUniform1i(texMode_uniformId, stone);
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, (obstacles[j].center[0] / 2) - 0.5f, (obstacles[j].center[2] / 2) - 0.5f, (obstacles[j].center[1] / 2) - 0.5f);
		}
		//else roof
		else
		{
			glUniform1i(texMode_uniformId, wood);
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, (obstacles[j].center[0] / 2), (obstacles[j].center[2]), (obstacles[j].center[1] / 2) - 1.0f);
			rotate(MODEL, 45.0, 0.0f, 1.0f, 0.0f);
		}
		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(myMeshes[objId].vao);
		glDrawElements(myMeshes[objId].type, myMeshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		objId++;
	}
	objId = objId + houseNumber;

	//render the shark fins based on sharkfinNumber
	for (int i = 0; i < sharkfinNumber; i++)
	{
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[objId].mat.shininess);
		glUniform1i(texMode_uniformId, checker);

		pushMatrix(MODEL);

		translate(MODEL, fins[i].pos[0], fins[i].pos[2], fins[i].pos[1]); // Adjust for fin's position6
		rotate(MODEL, fins[i].direction, 0.0f, 1.0f, 0.0f); // Rotate fin based on its angle

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(myMeshes[objId].vao);
		glDrawElements(myMeshes[objId].type, myMeshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		objId++;
	}

	//render the translucent buoys
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < buoyNumber; i++)
	{
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[objId].mat.shininess);
		glUniform1i(texMode_uniformId, checker);

		pushMatrix(MODEL);

		glUniform1i(texMode_uniformId, 4);
		translate(MODEL, obstacles[obstacleNumber - buoyNumber + i].center[0], obstacles[obstacleNumber - buoyNumber + i].center[2], obstacles[obstacleNumber - buoyNumber + i].center[1]);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(myMeshes[objId].vao);
		glDrawElements(myMeshes[objId].type, myMeshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		objId++;
	}
	glDisable(GL_BLEND);

	char lives_UI_MSG[11];
	snprintf(lives_UI_MSG, 11, "lives: %d/4", boat.lives);

	char timer_UI_MSG[20];
	int hours, minutes, seconds;
	
	seconds = int(elapsedTime);
	minutes = seconds / 60;
	hours = seconds / 3600;
	seconds -= minutes * 60;
	minutes -= hours * 60;
	snprintf(timer_UI_MSG, 20,"H:%d M:%d S:%d", hours, minutes, seconds);
	const char pause_UI_MSG[8] = "Paused!";

	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	//the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	if (isPaused)
	{
		RenderText(shaderText, pause_UI_MSG, 500.0f, 400.0f, 3.0f, 0.3, 0.7f, 0.9f);
	}
	else 
	{
		RenderText(shaderText, lives_UI_MSG, 10.0f, 720.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	}
	RenderText(shaderText, timer_UI_MSG, 680.0f, 720.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch (key) {

	case 27:
		glutLeaveMainLoop();
		break;
	case 'm': glEnable(GL_MULTISAMPLE); break;
	case 'k': glDisable(GL_MULTISAMPLE); break;

		// Follow Cam
	case '1':
		activeCam = 0;
		break;
		// Static Ortho Cam
	case '2':
		activeCam = 1;
		break;
		// Static Perpective Cam
	case '3':
		activeCam = 2;
		break;
	case 'a': // Move Left
		boat.angle -= 30.0f;  // Negative direction for left
		if ((boat.angle / 360) < -1.0f)
		{
			boat.angle += 360;
			boat.direction += 360;
		}
		boat.acceleration = 5.0f + (1.0f * speedSwitch);  // Set the speed
		break;
	case 'd':  // Move Right
		boat.angle += 30.0f;  // Positive direction for right
		if ((boat.angle / 360) > 1.0f)
		{
			boat.angle -= 360;
			boat.direction -= 360;
		}
		boat.acceleration = 5.0f + (1.0f * speedSwitch);  // Set the speed
		break;
	case 's':
		boat.acceleration = -5.0f - (1.0f * speedSwitch);
		boat.angle = boat.direction;
		break;
	case 'o':
		//increase speedSwitch for speed change logic
		speedSwitch++;
		//if speedswitch is 2 or more, return to 0
		if (speedSwitch >= 2)
		{
			speedSwitch = 0;
		}
		break;
	case 'n':
		isSunActive = !isSunActive;
		break;
	case 'c':
		isBuoyLightsActive = !isBuoyLightsActive;
		break;
	case 'h':
		isHeadlightsActive = !isHeadlightsActive;
		break;
	case 'f':
		depthFog = !depthFog;
		break;
	case 'p':
		isPaused = !isPaused;
		if (isPaused == true)
		{
			deltaT = 0;
		}
		else
		{
			deltaT = 0.05f;
		}
	}
}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camY = rAux * sin(betaAux * 3.14f / 180.0f);


	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//
GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");

	sunPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "sun_pos");					// Sun light world position

	buoyPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_pos");					// Buoy light world position
	buoyPos2_uniformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_pos2");				// Buoy2 light world position
	buoyPos3_uniformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_pos3");				// Buoy3 light world position
	buoyPos4_uniformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_pos4");				// Buoy4 light world position
	buoyPos5_uniformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_pos5");				// Buoy5 light world position
	buoyPos6_uniformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_pos6");				// Buoy6 light world position

	buoyConstantAttenuation_unirformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_const_att");	// Buoys constant attenuation
	buoyLinearAttenuation_unirformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_linear_att");	// Buoys linear attenuation
	buoyQuadraticAttenuation_unirformId = glGetUniformLocation(shader.getProgramIndex(), "buoy_quad_att");	// Buoys quadratic attenuation

	headlightPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "headlight_pos");		// Boat headlight world position
	headlightPos2_uniformId = glGetUniformLocation(shader.getProgramIndex(), "headlight_pos2");		// Boat headlight 2 world position
	headlightDir_uniformId = glGetUniformLocation(shader.getProgramIndex(), "headlight_dir");		// Boat headlight diretion
	headlightDir2_uniformId = glGetUniformLocation(shader.getProgramIndex(), "headlight_dir2");		// Boat headlight 2 diretion
	headlightAngle_uniformId = glGetUniformLocation(shader.getProgramIndex(), "headlight_angle");	// Boat headlight angle
	headlightExp_uniformId = glGetUniformLocation(shader.getProgramIndex(), "headlight_exp");		// Boat headlight exponent

	isSunActive_uniformId = glGetUniformLocation(shader.getProgramIndex(), "isSunActive");					// Sun light bool
	isBuoyLightsActive_uniformId = glGetUniformLocation(shader.getProgramIndex(), "isBuoyLightsActive");	// Buoy lights bool
	isHeadlightsActive_uniformId = glGetUniformLocation(shader.getProgramIndex(), "isHeadlightsActive");	// Boat headlights bool

	depthFog_uniformId = glGetUniformLocation(shader.getProgramIndex(), "depthFog");	// Spotlight 2 exponent

	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");

	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode"); // different modes of texturing

	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shaderText.isProgramValid()) {
		printf("GLSL Text Program Not Valid!\n");
		exit(1);
	}

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//
void init()
{
	MyMesh amesh;

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	// set the camera position based on its spherical coordinates
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	//Texture Object definition
	glGenTextures(3, TextureArray);
	Texture2D_Loader(TextureArray, "stone.tga", 0);
	Texture2D_Loader(TextureArray, "checker.png", 1);
	Texture2D_Loader(TextureArray, "lightwood.tga", 2);

	//values for the """water"""
	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.0f, 0.0f, 13.0f, 0.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	//create the water quad
	amesh = createQuad(10000.0f, 10000.0f);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;

	//value for the boat
	diff[0] = 0.8f;
	diff[1] = 0.6f;
	diff[2] = 0.4f;
	diff[3] = 1.0f;
	//create the boat cube
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;

	//lets do the paddle for the boat
	//handle
	amesh = createCylinder(1.5f, 0.05f, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	//head1
	amesh = createCone(0.25, 0.25, 2);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	//head2
	amesh = createCone(0.25, 0.25, 2);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;

	//houses
	//base of the house 1
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	//roof of the house 1
	amesh = createCone(0.5, 1.0, 4);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	obstacles[houseNumber + 1].center[0] = 3.0f + 1.0f;
	obstacles[houseNumber + 1].center[1] = -5.0f + 1.0f;
	obstacles[houseNumber + 1].center[2] = 0.0f + 1.0f;
	obstacles[houseNumber + 1].radius = 1.0f; //sqrt(0.75 * 4);

	//base of the house 1
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	//roof of the house 1
	amesh = createCone(0.5, 1.0, 4);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	obstacles[houseNumber + 2].center[0] = -3.0f + 1.0f;
	obstacles[houseNumber + 2].center[1] = -5.0f + 1.0f;
	obstacles[houseNumber + 2].center[2] = 0.0f + 1.0f;
	obstacles[houseNumber + 2].radius = 1.0f; //sqrt(0.75 * 4);



	//make the houses on the left side
	for (int i = 0; i < houseNumber/2; i++)
	{
		//base
		amesh = createCube();
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);
		//roof
		amesh = createCone(0.5, 1.0, 4);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);
		obstacles[i].center[0] = 10.0f + 1.0f;
		obstacles[i].center[1] = -5.0f + 2*i + 1.0f;
		obstacles[i].center[2] = 0.0f + 1.0f;
		obstacles[i].radius = 1.0f; //sqrt(0.75 * 4);
	}
	//make the houses on the right side
	for (int i = 0; i < houseNumber/2; i++)
	{
		//base
		amesh = createCube();
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);
		//roof
		amesh = createCone(0.5, 1.0, 4);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);
		obstacles[i + houseNumber / 2].center[0] = -10.0f + 1.0f;
		obstacles[i + houseNumber / 2].center[1] = -5.0f + 2 * i + 1.0f;
		obstacles[i + houseNumber / 2].center[2] = 0.0f + 1.0f;
		obstacles[i + houseNumber / 2].radius = 1.0f; //sqrt(0.75 * 4);
	}

	//shark fins
	for (int i = 0; i < sharkfinNumber; i++)
	{
		amesh = createCone(1, 0.5f, 3);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);
	}

	//buoys come last because they will be translucid
	diff[3] = 0.5f;
	for (int i = 0; i < buoyNumber; i++)
	{
		amesh = createCone(5.0, 1.4, 100);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);

		switch (i % 6) {
		case 0:
			obstacles[(obstacleNumber - buoyNumber) + i].center[0] = -5.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].center[1] = 5.0f + 5 * (i-0);
			obstacles[(obstacleNumber - buoyNumber) + i].center[2] = 0.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].radius = 0.8f;
			break;
		case 1:
			obstacles[(obstacleNumber - buoyNumber) + i].center[0] = -0.5f;
			obstacles[(obstacleNumber - buoyNumber) + i].center[1] = 10.0f + 5 * (i-1);
			obstacles[(obstacleNumber - buoyNumber) + i].center[2] = 0.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].radius = 0.8f;
			break;
		case 2:
			obstacles[(obstacleNumber - buoyNumber) + i].center[0] = 4.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].center[1] = 15.0f + 5 * (i-2);
			obstacles[(obstacleNumber - buoyNumber) + i].center[2] = 0.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].radius = 0.8f;
			break;
		case 3:
			obstacles[(obstacleNumber - buoyNumber) + i].center[0] = 1.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].center[1] = 20.0f + 5 * (i-3);
			obstacles[(obstacleNumber - buoyNumber) + i].center[2] = 0.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].radius = 0.8f;
			break;
		case 4:
			obstacles[(obstacleNumber - buoyNumber) + i].center[0] = 7.75f;
			obstacles[(obstacleNumber - buoyNumber) + i].center[1] = 20.0f + 5 * (i-4);
			obstacles[(obstacleNumber - buoyNumber) + i].center[2] = 0.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].radius = 0.8f;
			break;
		case 5:
			obstacles[(obstacleNumber - buoyNumber) + i].center[0] = -3.5f;
			obstacles[(obstacleNumber - buoyNumber) + i].center[1] = 15.0f + 5 * (i - 5);
			obstacles[(obstacleNumber - buoyNumber) + i].center[2] = 0.0f;
			obstacles[(obstacleNumber - buoyNumber) + i].radius = 0.8f;
			break;
		}
		buoyLightPos[i][0] = obstacles[(obstacleNumber - buoyNumber) + i].center[0];
		buoyLightPos[i][1] = obstacles[(obstacleNumber - buoyNumber) + i].center[1];
		bstacles[(obstacleNumber - buoyNumber) + i].center[2] = 5.0f;
	}

	/*
	//lets try making a house
	//base of the house 1
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	//roof of the house 1
	amesh = createCone(0.5, 1.0, 4);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	obstacles[0].center[0] = 9.0f + 1.0f;
	obstacles[0].center[1] = 9.0f + 1.0f;
	obstacles[0].center[2] = 0.0f + 1.0f;
	obstacles[0].radius = 1.0f; //sqrt(0.75 * 4);
	//buoy 6
	amesh = createCone(3.0, 0.3, 100);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	objectNumber++;
	obstacles[12].center[0] = 15.0f;
	obstacles[12].center[1] = 20.0f;
	obstacles[12].center[2] = 0.0f + 1.0f;
	obstacles[12].radius = 0.2f;
	*/

	/*// create geometry and VAO of the pawn
	amesh = createPawn();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);


	// create geometry and VAO of the sphere
	amesh = createSphere(1.0f, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);

	float amb1[]= {0.3f, 0.0f, 0.0f, 1.0f};
	float diff1[] = {0.8f, 0.1f, 0.1f, 1.0f};
	float spec1[] = {0.9f, 0.9f, 0.9f, 1.0f};
	shininess=500.0;

	// create geometry and VAO of the cylinder
	amesh = createCylinder(1.5f, 0.5f, 20);
	memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);

	// create geometry and VAO of the
	amesh = createCone(1.5f, 0.5f, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);*/

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char** argv) {

	cams[0].pos[0] = camX;
	cams[0].pos[1] = camY;
	cams[0].pos[2] = camZ;

	cams[1].type = 1;
	cams[1].pos[1] = 50;

	cams[2].pos[1] = 50;

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	glutIdleFunc(renderScene);  // Use it for maximum performance
	//glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);


	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}



