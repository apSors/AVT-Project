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

GLint buoyPos_uniformId;			// Buoy light world position
GLint buoyPos2_uniformId;			// Buoy 2 light world position
GLint buoyPos3_uniformId;			// Buoy 3 light world position
GLint buoyPos4_uniformId;			// Buoy 4 light world position
GLint buoyPos5_uniformId;			// Buoy 5 light world position
GLint buoyPos6_uniformId;			// Buoy 6 light world position

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

GLint tex_loc, tex_loc1, tex_loc2;

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

float buoyLightPos[4] =	 { 10.0f, 5.0f, 10.0f, 1.0f };	// Buoy lights world position
float buoyLightPos2[4] = { -15.0f, 5.0f, -15.0f, 1.0f };
float buoyLightPos3[4] = { 5.0f, 6.0f, -5.0f, 1.0f };
float buoyLightPos4[4] = { -5.0f, 6.0f, 5.0f, 1.0f };
float buoyLightPos5[4] = { -15.0f, 6.0f, 15.0f, 1.0f };
float buoyLightPos6[4] = { 15.0f, 6.0f, 15.0f, 1.0f };

float buoyLightConstantAttenuation = 2.0f;
float buoyLightLinearAttenuation = 0.5f;
float buoyLightQuadraticAttenuation = 0.7f;

bool isSunActive = 1;
bool isBuoyLightsActive = 0;
bool isHeadlightsActive = 0;

//number of objects to be drawn
int numObj = 0;

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
	float angle = 0.0f;
	float direction = 0.0f;
};

Boat boat;

float deltaT = 0.05f;
float decayy = 0.1f;

// Structure to store data for each shark fin (cone)
struct SharkFin {
	float speed = 0.0f;
	float pos[3] = { 1000.0f, 1000.0f, 0.0f };
	float angle = 0.0f;
	float direction = 0.0f;
	float slope = 0.0f;
	float initialPos[3] = { 0.0f, 0.0f, 0.0f};
};
// Shark fins
const int sharkfinNumber = 4;
SharkFin fins[sharkfinNumber];

// Function to generate a random float value between min and max
float randomFloat(float min, float max) {
	return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
}

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	int i = 0;

	// Boat movement logic
	boat.pos[0] += ((boat.speed * deltaT) + (1/2 * boat.acceleration * pow(deltaT,2))) * cos(boat.direction * 3.14 / 180);
	boat.pos[1] += ((boat.speed * deltaT) + (1 / 2 * boat.acceleration * pow(deltaT, 2))) * sin(boat.direction * 3.14 / 180);
	//boat.pos[1] += boat.speed * sin(boat.direction * 3.14 / 180) * deltaT;

	// Define the radii for the paths (circular or elliptical)
	float radius_1 = 5.0f;  // Radius for fin 1's path
	float radius_2 = 7.0f;  // Radius for fin 2's path

	for (i = 0; i < sharkfinNumber; i++)
	{
		

		if ((abs(fins[i].pos[0] - fins[i].initialPos[0]) > 5.0f) || ((abs(fins[i].pos[1] - fins[i].initialPos[1])) > 5.0f))
		{
			fins[i].initialPos[0] = (float)(5.0 * cos(rand())) + boat.pos[0];
			fins[i].initialPos[1] = (float)(5.0 * sin(rand())) + boat.pos[1];
			fins[i].slope = ((fins[i].initialPos[1] - boat.pos[1]) / (fins[i].initialPos[0] - boat.pos[0]));
			fins[i].angle = atan(fins[i].slope);
			fins[i].pos[0] = fins[i].initialPos[0];
			fins[i].pos[1] = fins[i].initialPos[1];

		}

		
		// Keep the angle between 0 and 360 degrees
		if (fins[i].angle > 3.14) { fins[i].angle -= 3.14f; }
		else if (fins[i].angle < -3.14f) { fins[i].angle += 3.14f; }

		//fins movement logic
		// we want them to move in a straight line, starting at their spawn point moving towards the boat position
		fins[i].pos[0] += (fins[i].slope * deltaT) * cos(fins[i].angle);  // X position
		fins[i].pos[1] += (fins[i].slope * deltaT) * sin(fins[i].angle);  // Y position

	}

	//boat acceleration reduction
	//boat moving forwards (press 'a' or 'd')
	if ((boat.acceleration > 0) && (boat.speed >= 0))
	{
		boat.acceleration -= decayy;
		boat.speed += boat.acceleration * deltaT;
	}
	else if ((boat.acceleration <= 0) && (boat.speed > 0))
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
		boat.acceleration += decayy;
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

void refresh(int value)
{
	//PUT YOUR CODE HERE
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

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


// ------------------------------------------------------------
//
// Render stufff
//

void renderScene(void) {

	GLint loc;

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	float headlightPos[4] = { boat.pos[0], 1.0f, boat.pos[1], 1.0f};			// Boat headlight world position (spotlight)
	float headlightPos2[4] = { boat.pos[0], 1.0f, 1.0f + boat.pos[1], 1.0f};	// Boat headlight 2 world postion (spotlight)

	float headlightDir[4] = { 1.0f, 0.0f, 0.0f, 0.0f };		// Spotlight pointing diretion 
	float headlightDir2[4] = { 1.0f, 0.0f, 0.0f, 0.0f };	// Spotlight 2 pointing diretion

	float headlightAngle = 0.9;		// Spotlight angle (0-0.9999)

	float headlightExp = 0.02f;		// Headlights quality

	cams[0].pos[0] = camX + boat.pos[0];
	cams[0].pos[1] = camY + boat.pos[2];
	cams[0].pos[2] = camZ + boat.pos[1];

	cams[1].type = 1;
	cams[1].pos[1] = 20;

	cams[2].pos[1] = 20;

	// Follow Cam
	if (activeCam == 0) {
		// set the camera using a function similar to gluLookAt
		lookAt(cams[0].pos[0], cams[0].pos[1], cams[0].pos[2], boat.pos[0], boat.pos[2], boat.pos[1], 0, 1, 0);
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

	float ratio = (m_view[2] - m_view[0]) / (m_view[3] - m_view[1]);

	loadIdentity(PROJECTION);

	if (cams[activeCam].type == 0) {
		perspective(63.13, ratio, 1, 100);
	}
	else {
		ortho(ratio * -25, ratio * 25, -25, 25, 0.1, 100);
	}

	// use our shader

	glUseProgram(shader.getProgramIndex());


		float res[4];		// Sun light world position

		float res6[4];		// Buoy light world position
		float res7[4];		// Buoy light world position
		float res8[4];		// Buoy light world position
		float res9[4];		// Buoy light world position
		float res10[4];		// Buoy light world position
		float res11[4];		// Buoy light world position

		float res2[4];		// Boat headlight world position 
		float res3[4];		// Boat headlight 2 world position
		float res4[4];		// Boat headlight diretion
		float res5[4];		// Boat headlight 2 diretion

		multMatrixPoint(VIEW, sunLightPos, res);		// sunLightPos definido em World Coord so is converted to eye space
		
		multMatrixPoint(VIEW, headlightPos, res2);		// headlightPos definido em World Coord so is converted to eye space
		multMatrixPoint(VIEW, headlightPos2, res3);		// headlightPos2 definido em World Coord so is converted to eye space
		multMatrixPoint(VIEW, headlightDir, res4);		// headlightDir definido em World Coord so is converted to eye space
		multMatrixPoint(VIEW, headlightDir2, res5);		// headlightDir2 definido em World Coord so is converted to eye space

		multMatrixPoint(VIEW, buoyLightPos, res6);		// buoyLightPos definido em World Coord so is converted to eye space
		multMatrixPoint(VIEW, buoyLightPos2, res7);		// buoyLightPos2 definido em World Coord so is converted to eye space
		multMatrixPoint(VIEW, buoyLightPos3, res8);		// buoyLightPos3 definido em World Coord so is converted to eye space
		multMatrixPoint(VIEW, buoyLightPos4, res9);		// buoyLightPos4 definido em World Coord so is converted to eye space
		multMatrixPoint(VIEW, buoyLightPos5, res10);	// buoyLightPos5 definido em World Coord so is converted to eye space
		multMatrixPoint(VIEW, buoyLightPos6, res11);	// buoyLightPos6 definido em World Coord so is converted to eye space

		glUniform4fv(sunPos_uniformId, 1, res);

		glUniform4fv(buoyPos_uniformId, 1, res6);
		glUniform4fv(buoyPos2_uniformId, 1, res7);
		glUniform4fv(buoyPos3_uniformId, 1, res8);
		glUniform4fv(buoyPos4_uniformId, 1, res9);
		glUniform4fv(buoyPos5_uniformId, 1, res10);
		glUniform4fv(buoyPos6_uniformId, 1, res11);

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

		int objId = 0;

	for (int i = 0; i < numObj; ++i) {

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
			//rotate(MODEL, -90.0f, 1.0f, 0.0f, 0.0f);
		}
		// boat
		if (i == 1)
		{
			translate(MODEL, boat.pos[0] - 0.0f, boat.pos[2], boat.pos[1] - 0.0f);
			rotate(MODEL, -boat.direction, 0.0f, 1.0f, 0.0f);
		}
		if (i == 0) { 
			rotate(MODEL, -90.0f, 1.0f, 0.0f, 0.0f); 
		} 
		//base of 1st house 
		else if (i == 2) {
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, 4.5f, 0.0f, 4.5);
		}
		//roof of 1st house
		else if (i == 3) {

			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, 5.0f, 1.0f, 5.0f);
			rotate(MODEL, 45.0, 0.0f, 1.0f, 0.0f);
		}
		//base of 2nd house 
		else if (i == 4) {
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, 4.5f, 0.0f, -1.5f);
		}
		//roof of 2nd house 
		else if (i == 5) {
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, 5.0f, 1.0f, -1.0f);
			rotate(MODEL, 45.0, 0.0f, 1.0f, 0.0f);
		}
		//base of 3rd house 
		else if (i == 6) {
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, -4.5f, 0.0f, -1.5f);
		}
		//roof of 3rd house 
		else if (i == 7) {
			scale(MODEL, 2.0f, 2.0f, 2.0f);
			translate(MODEL, -4.0f, 1.0f, -1.0f);
			rotate(MODEL, 45.0, 0.0f, 1.0f, 0.0f);
		}
		//handle of the paddle
		else if (i == 8) {
			translate(MODEL, 0.5f, 1.2f, 0.0f);
			rotate(MODEL, 90.0f, 0.0f, 0.0f, 1.0f);
		}
		//head1 of the paddle
		else if (i == 9) {
			translate(MODEL, -0.4f, 1.2f, 0.0f);
			rotate(MODEL, -90.0f, 0.0f, 0.0f, 1.0f);
		}
		//head2 of the paddle
		else if (i == 10) {
			translate(MODEL, 1.4f, 1.2f, 0.0f);
			rotate(MODEL, 90.0f, 0.0f, 0.0f, 1.0f);
		}
		//shark fin 1
		else if (i == 11) {
			translate(MODEL, fins[0].pos[0] - 2.0f, fins[0].pos[2], fins[0].pos[1] - 1.0f); // Adjust for fin's position
			rotate(MODEL, -fins[0].angle, 0.0f, 1.0f, 0.0f); // Rotate fin based on its angle

			//translate(MODEL, 5.0f, 0.0f, 5.0f);
			//rotate(MODEL, 90.0f, 0.0f, 0.0f, 1.0f);
		}
		//shark fin 2
		else if (i == 12) {
			translate(MODEL, fins[1].pos[0] - 4.0f, fins[1].pos[2], fins[1].pos[1] - 0.0f); // Adjust for fin's position
			rotate(MODEL, -fins[1].angle, 0.0f, 1.0f, 0.0f); // Rotate fin based on its angle
		

			//translate(MODEL, 4.0f, 0.0f, -7.0f);
			//rotate(MODEL, 90.0f, 0.0f, 0.0f, 1.0f);
		}

		// Initial settings for fins' speed and direction:
		for (int j = 0; j < sharkfinNumber; j++)
		{
			fins[j].speed = 30.0f;  // Degrees per second
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
		pushMatrix(MODEL);

		translate(MODEL, fins[i].pos[0], fins[i].pos[2], fins[i].pos[1]); // Adjust for fin's position
		//rotate(MODEL, -fins[i].angle, 0.0f, 1.0f, 0.0f); // Rotate fin based on its angle

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
	RenderText(shaderText, "This is a sample text", 25.0f, 25.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	RenderText(shaderText, "AVT Light and Text Rendering Demo", 440.0f, 570.0f, 0.5f, 0.3, 0.7f, 0.9f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

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

	case 'p':
		printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
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
	case 'n':
		isSunActive = !isSunActive;
		break;
	case 'c':
		isBuoyLightsActive = !isBuoyLightsActive;
		break;
	case 'h':
		isHeadlightsActive = !isHeadlightsActive;
		break;
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
	//glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

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

	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");

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

	//values for the """water"""
	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.0f, 0.0f, 13.0f, 0.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	//create the water quad
	amesh = createQuad(100.0f, 100.0f);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;

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
	numObj++;

	//lets try making a house
	//base of the house
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;
	//roof of the house
	amesh = createCone(0.5, 1.0, 4);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;

	//antoher one
	//base of the house
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;
	//roof of the house
	amesh = createCone(0.5, 1.0, 4);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;

	//and antoher one
	//base of the house
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;
	//roof of the house
	amesh = createCone(0.5, 1.0, 4);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;

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
	numObj++;
	//head1
	amesh = createCone(0.25, 0.25, 2);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;
	//head2
	amesh = createCone(0.25, 0.25, 2);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);
	numObj++;

	//shark fins
	for (int i = 0; i < sharkfinNumber; i++)
	{
		amesh = createCone(1, 0.5f, 3);
		memcpy(amesh.mat.ambient, amb, 10 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 10 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 10 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 10 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);
	}

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



