// Assignment 2.cpp
// Heliaac
// Christopher Trott, Ashley Sexton, Aleesha Torkington
// Created on 28/08/09
// Last Modified on 13/10/09 @ 13:08
//
// Some of this code is taken from animlightpos.cpp on the LearnJCU resources page
// Some code to do with lighting was gained from the URL: http://www.falloutsoftware.com/tutorials/gl/gl8.htm
// Some code to do with text on screen gained from Lighthouse 3D @ URL: http://www.lighthouse3d.com/opengl/glut/index.php?bmpfontortho
// Bitmap.h and Bitmap.cpp found at: http://www.gamedev.net/reference/articles/article1966.asp

#include<GL/freeglut.h>
#include<math.h>
#include<string>
#include<iostream>
#include"Bitmap.h"

using namespace std;

struct vertex
{
    float x;
    float y;
    float z;
};

struct objectBox
{
    float xPos;        // x position
    float yPos;        // y position
    float zPos;        // z position
	float rotX;     // rotation angle in x axis
    float rotY;     // rotation angle of direction (y axis)
	float rotZ;		// rotation angle in z axis
    float xSize;     // x radius (half-length) of bounding box
    float ySize;     // y radius of bounding box
    float zSize;     // z radius of bounding box
};

struct checkPoint
{
	int checkpoint;
	float xSize;
	float ySize;
	float zSize;
	float xPos;
	float yPos;
	float zPos;
	float rotY;
	bool activated;
};

void drawHeli(void);
void drawHeliBody(void);
void drawHeliRotor(void);
void drawGround(void);
void moveHeliForward(float speed, bool checkCol);
void moveHeliBack(float speed, bool checkCol);
void moveHeliDown(float speed, bool checkCol);
void moveHeliUp(float speed, bool checkCol);
void checkBounds(void);
bool checkBoxCollision(objectBox object1, objectBox object2);
void checkHeliCollisions(void);
void checkHeliLanding(void);
void drawBuilding(objectBox building, int textureNum);
void updateFPS(void);
void updateGameTime(void);
void displayText(void);
void resetPerspectiveProjection(void);
void setOrthographicProjection(void);
void renderBitmapString(float x, float y, void *font,char *string);
void displayHelp(void);
float cosDeg(float degRot);
float sinDeg(float degRot);
GLuint loadTextureBMP(char * filename, int wrap, int width, int height);
void displayDashboard(void);
void drawFinishScreen(void);
void enableFog(void);

float cameraDistance = 5.0;
float cameraZoom = 1.5;
objectBox heli = {0, 2, 0, 0, 0, 0, 2.5, 1.5, 1};
float windscreenRot = 0.0;

objectBox eye = {cameraDistance, heli.yPos, cameraDistance, 0, 135, 0, 0, 0, 0};
objectBox building0 = {10, 0, 10, 0, 0, 0, 4, 8, 4};
objectBox landingPadA = {0, 0.002, 0, 0, 0, 0, 3, 0.1, 3};
objectBox landingPadB = {-10, 0.002, -10, 0, 0, 0, 3, 0.1, 3};

bool movingForward = false;
bool movingBack = false;
bool movingUp = false;
bool movingDown = false;
bool turningLeft = false;
bool turningRight = false;

bool stopHeli = false;
bool startHeli = false;

bool gameFinished = false;

int font = (int)GLUT_BITMAP_HELVETICA_18;
int textX = 20;
int textY = 20;

bool light0 = true;
bool light1 = false;

//trying to implement start/stop without changing functions too much
bool helicopterOn = false;

GLfloat light0_position[] = { 1, 20, 1, 0 };

GLuint heliBodyList;
GLuint heliRotorList;
GLuint groundList;

double rotor = 0;
const double MAX_ROTOR_SPEED = 10;
double rotorSpeed = 0.0;
int groundSize = 20;
int groundHeight = 0;

int windowWidth = 500;
int windowHeight = 500;

int windowPosWidth = 100;
int windowPosHeight = 100;

int frames = 0;
int time = 0;
int timeBase = 0;
float fps = 50.0;
char* strFps = new char[4];     // FPS string for display on-screen
char* strGameTime = new char[8];	// Game time string
int bestTime = 100000;
char* strBestTime = new char[8];
int gameTime = 0;
int gameTimeBase = 0;
int penaltyTime = 0;

const int ROTATE_SPEED = 180;
const int HELI_SPEED = 12;
const float LEAN_FACTOR = 15.0;
float heliLeanFront = 0.0;
float heliLeanSide = 0.0;
int rotSpeed = ROTATE_SPEED / fps;
float heliSpeed = HELI_SPEED / fps;

bool pause = false;
bool wire = false;

int last_mouse_x = 0;
int last_mouse_y = 0;
bool leftMouseDown = false;

float pi = 3.1415926535897932384626433832795;

const int MAX_TEXTURES = 10;
GLuint textures[MAX_TEXTURES];

const int MAX_CHECKPOINTS = 3;		// Cannot exceed 100. Only set as high as is required.
checkPoint points[MAX_CHECKPOINTS];
int checkpointNum = 0;

bool heliTextures = true;

// Initialise the OpenGL properties
void init(void)
{
    glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
    glShadeModel(GL_FLAT);

	enableFog();

    // Make object materials equal to glColor*() properties
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

	heli.yPos = groundHeight + heli.ySize;

    // Define the heliBody display 
    heliBodyList = glGenLists(1);
    glNewList(heliBodyList, GL_COMPILE);
    drawHeliBody();
    glEndList();
   
    // Define the heliRotor display list
    heliRotorList = glGenLists(1);
    glNewList(heliRotorList, GL_COMPILE);
    drawHeliRotor();
    glEndList();

    glClearColor(0.0, 0.0, 0.0, 0.0);
   
    // Set light position
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    // Create light components
    GLfloat ambientLight[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat diffuseLight[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat specularLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    // Assign light components
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    timeBase = glutGet(GLUT_ELAPSED_TIME);

	textures[0] = loadTextureBMP( "Textures/ground.bmp", true, 256, 256 );
	textures[1] = loadTextureBMP( "Textures/building.bmp", true, 256, 256 );
	textures[2] = loadTextureBMP( "Textures/heliTex.bmp", true, 256, 256 );
	textures[3] = loadTextureBMP( "Textures/heliPadA.bmp", true, 256, 256 );
	textures[4] = loadTextureBMP( "Textures/heliPadB.bmp", true, 256, 256 );
	textures[5] = loadTextureBMP( "Textures/sky.bmp", true, 256, 256);

	// Define the ground display list
    groundList = glGenLists(1);
    glNewList(groundList, GL_COMPILE);
    drawGround();
    glEndList();

	points[0].checkpoint = 0;
	points[0].xSize = 5.0;
	points[0].ySize = 5.0;
	points[0].zSize = 0.5;
	points[0].xPos = 5.0;
	points[0].yPos = 2.5;
	points[0].zPos = 5.0;
	points[0].rotY = 0;

	points[1].checkpoint = 1;
	points[1].xSize = 5.0;
	points[1].ySize = 5.0;
	points[1].zSize = 0.5;
	points[1].xPos = -5.0;
	points[1].yPos = points[1].ySize / 2;	// Make yPos so that the bottom edge is on the ground
	points[1].zPos = 5.0;
	points[1].rotY = 60;

	points[2].checkpoint = 2;
	points[2].xSize = 5.0;
	points[2].ySize = 5.0;
	points[2].zSize = 0.5;
	points[2].xPos = -5.0;
	points[2].yPos = points[2].ySize/2;		// Make yPos so that the bottom edge is on the ground
	points[2].zPos = -5.0;
	points[2].rotY = 45;
}

void enableFog(void)
{
	GLfloat fogColor[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat density = 0.04;

	glEnable(GL_DEPTH_TEST); //enable the depth testing
	glEnable(GL_FOG); //enable the fog
	glFogi(GL_FOG_MODE, GL_EXP2); //set the fog mode to GL_EXP2

	glFogfv(GL_FOG_COLOR, fogColor); //set the fog color to our color chosen above
	glFogf(GL_FOG_DENSITY, density); //set the density to the value above
	glHint(GL_FOG_HINT, GL_DONT_CARE); // set the fog to look the nicest, may slow down on older cards
}

GLuint loadTextureBMP( char * filename, int wrap, int width, int height )
{
	Bitmap *image;
    GLuint texture;

	image = new Bitmap();

	glEnable(GL_TEXTURE_2D);

	if (image == NULL) {
		cout << "Could not create Bitmap class.\n";
		return -1;
	}

	if (image->loadBMP(filename))
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		/*// when texture area is small, bilinear filter the closest mipmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
		// when texture area is large, bilinear filter the first mipmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );*/

		// if wrap is true, the texture wraps over at the edges (repeat)
		//       ... false, the texture ends at the edges (clamp)
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP );

		glTexImage2D(GL_TEXTURE_2D, 0, 3, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);

		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	}
	else 
	{
		cout << "Could not load bitmap " << filename << ".\n";
		return -1;
	}

	/*if ( gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, image->data ) )
    {
		cout << "Error creating texture mipmaps for texture: " << filename << ".\n";
    }*/


	if (image)
	{
		delete image;
	}

    return texture;
}


//draw sky
void drawSky()
{
	float roll = 0.0;
	float skyz = 0.0;
	float skyzb = 0.0;
	float skyx = 0.0;
	float skyxb = 0.0;
	float skyHeight = 20.0;
	float skyDist = 20.0;

	if (heli.zPos + skyDist < groundSize)
	{
		skyz = heli.zPos + skyDist;
	}
	else skyz = groundSize;

	if (heli.zPos - skyDist > -groundSize)
	{
		skyzb = heli.zPos - skyDist;
	}
	else skyzb = -groundSize;

	if (heli.xPos + skyDist < groundSize)
	{
		skyx = heli.xPos + skyDist;
	}
	else skyx = groundSize;

	if (heli.xPos - skyDist > -groundSize)
	{
		skyxb = heli.xPos - skyDist;
	}
	else skyxb = -groundSize;


	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, textures[5]);			// Select The Sky Texture
	glColor3f(0.33, 0.64, 1);
	glBegin(GL_QUADS);							// Begin Drawing Quads
	//top sky
		glTexCoord2f(1.0f,roll/1.5f+1.0f); glVertex3f(-groundSize, skyHeight, groundSize);	// Top Right
		glTexCoord2f(0.0f,roll/1.5f+1.0f); glVertex3f(groundSize, skyHeight, groundSize);	// Top Left
		glTexCoord2f(0.0f,roll/1.5f+0.0f); glVertex3f(groundSize, skyHeight, -groundSize);	// Bottom Left
		glTexCoord2f(1.0f,roll/1.5f+0.0f); glVertex3f(-groundSize, skyHeight, -groundSize);	// Bottom Right
	//right sky
		glTexCoord2f(1.0f,roll/1.5f+1.0f); glVertex3f(groundSize, skyHeight, skyz);	// Top Right
		glTexCoord2f(0.0f,roll/1.5f+1.0f); glVertex3f(-groundSize, skyHeight, skyz);	// Top Left
		glTexCoord2f(0.0f,roll/1.5f+0.0f); glVertex3f(-groundSize, 0, skyz);	// Bottom Left
		glTexCoord2f(1.0f,roll/1.5f+0.0f); glVertex3f(groundSize, 0, skyz);	// Bottom Right
	//front sky
		glTexCoord2f(1.0f,roll/1.5f+1.0f); glVertex3f(skyx, skyHeight, groundSize);	// Top Right
		glTexCoord2f(0.0f,roll/1.5f+1.0f); glVertex3f(skyx, skyHeight, -groundSize);	// Top Left
		glTexCoord2f(0.0f,roll/1.5f+0.0f); glVertex3f(skyx, 0, -groundSize);	// Bottom Left
		glTexCoord2f(1.0f,roll/1.5f+0.0f); glVertex3f(skyx, 0, groundSize);	// Bottom Right
	// left sky
		glTexCoord2f(1.5f,roll+1.0f); glVertex3f(groundSize, skyHeight, skyzb);		// Top Right
		glTexCoord2f(0.5f,roll+1.0f); glVertex3f(-groundSize, skyHeight, skyzb);		// Top Left
		glTexCoord2f(0.5f,roll+0.0f); glVertex3f(-groundSize, 0.0, skyzb);		// Bottom Left
		glTexCoord2f(1.5f,roll+0.0f); glVertex3f(groundSize, 0.0, skyzb);		// Bottom Right
	// back sky
		// making sky roll toward viewer
		glTexCoord2f(1.0f,roll/1.5f+1.0f); glVertex3f(skyxb,skyHeight,-groundSize);	// Top Right
		glTexCoord2f(0.0f,roll/1.5f+1.0f); glVertex3f(skyxb,skyHeight,groundSize);	// Top Left
		glTexCoord2f(0.0f,roll/1.5f+0.0f); glVertex3f(skyxb,0.0,groundSize);	// Bottom Left
		glTexCoord2f(1.0f,roll/1.5f+0.0f); glVertex3f(skyxb,0.0,-groundSize);	// Bottom Right
	// Bottom sky?
		//glTexCoord2f(1.5f,roll+1.0f); glVertex3f( 28.0f,+7.0f,0.0f);		// Top Right
		//glTexCoord2f(0.5f,roll+1.0f); glVertex3f(-28.0f,+7.0f,0.0f);		// Top Left
		//glTexCoord2f(0.5f,roll+0.0f); glVertex3f(-28.0f,+7.0f,-50.0f);		// Bottom Left
		//glTexCoord2f(1.5f,roll+0.0f); glVertex3f( 28.0f,+7.0f,-50.0f);		// Bottom Right
	glEnd();	// Done Drawing Quads
	glDisable(GL_TEXTURE_2D);
}

void drawBuilding(objectBox building, int textureNum)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[textureNum]);
//	   -------
//   /       /|
//	v2-----v3 |
//	|       | |
//	|       | |
//	|       | |
//	|       | |
//	|       |/
//	v0-----v1

	vertex v0 = {-building.xSize, 0.0, building.zSize};
	vertex v1 = {building.xSize, 0.0, building.zSize};
	vertex v2 = {-building.xSize, building.ySize, building.zSize};
	vertex v3 = {building.xSize, building.ySize, building.zSize};

	glTranslatef(building0.xPos, building0.yPos, building0.zPos);
	glRotatef(building0.rotY, 0.0, 1.0, 0.0);

	glBegin(GL_QUADS);
	// Draw wall
	glTexCoord2f(0.0, 0.0);							glVertex3f(v0.x, v0.y, v0.z);
	glTexCoord2f(building.xSize, 0.0);				glVertex3f(v1.x, v1.y, v1.z);
	glTexCoord2f(building.xSize, building.ySize);	glVertex3f(v3.x, v3.y, v3.z);
	glTexCoord2f(0.0, building.ySize);				glVertex3f(v2.x, v2.y, v2.z);
	// Draw wall
	glTexCoord2f(0.0,0.0);							glVertex3f(v0.x, v0.y, -v0.z);
	glTexCoord2f(building.xSize,0.0);				glVertex3f(v1.x, v1.y, -v1.z);
	glTexCoord2f(building.xSize,building.ySize);	glVertex3f(v3.x, v3.y, -v3.z);
	glTexCoord2f(0.0,building.ySize);				glVertex3f(v2.x, v2.y, -v2.z);
	// Draw wall
	glTexCoord2f(0.0, building.ySize);				glVertex3f(v2.x, v2.y, -v2.z);
	glTexCoord2f(0.0, 0.0);							glVertex3f(v0.x, v0.y, -v0.z);
	glTexCoord2f(building.xSize, 0.0);				glVertex3f(v0.x, v0.y, v0.z);
	glTexCoord2f(building.xSize, building.ySize);	glVertex3f(v2.x, v2.y, v2.z);
	// Draw wall
	glTexCoord2f(0.0, 0.0);							glVertex3f(v1.x, v1.y, v1.z);
	glTexCoord2f(0.0, building.ySize);				glVertex3f(v3.x, v3.y, v3.z);
	glTexCoord2f(building.xSize, building.ySize);	glVertex3f(v3.x, v3.y, -v3.z);
	glTexCoord2f(building.xSize, 0.0);				glVertex3f(v1.x, v1.y, -v1.z);
	// Draw floor
	glVertex3f(v0.x, v0.y, v0.z);
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v1.x, v1.y, -v1.z);
	glVertex3f(v0.x, v0.y, -v0.z);
	// Draw roof
	glVertex3f(v2.x, v2.y, v2.z);
	glVertex3f(v3.x, v3.y, v3.z);
	glVertex3f(v3.x, v3.y, -v3.z);
	glVertex3f(v2.x, v2.y, -v2.z);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawHeli()
{
    glPushMatrix();
    // Go to the heli position before drawing the heli
    glTranslatef(heli.xPos, heli.yPos, heli.zPos);
    glRotatef(heli.rotY, 0.0, 1.0, 0.0);
    glRotatef(heliLeanFront, 0.0, 0.0, 1.0);
    glRotatef(heliLeanSide, 1.0, 0.0, 0.0);
	
	// Draw bounding box
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(-heli.xSize, -heli.ySize, -heli.zSize);
	glVertex3f(heli.xSize, -heli.ySize, -heli.zSize);
	glVertex3f(heli.xSize, heli.ySize, -heli.zSize);
	glVertex3f(-heli.xSize, heli.ySize, -heli.zSize);
	glVertex3f(-heli.xSize, -heli.ySize, -heli.zSize);
	glVertex3f(-heli.xSize, -heli.ySize, heli.zSize);
	glVertex3f(heli.xSize, -heli.ySize, heli.zSize);
	glVertex3f(heli.xSize, heli.ySize, heli.zSize);
	glVertex3f(-heli.xSize, heli.ySize, heli.zSize);
	glVertex3f(-heli.xSize, -heli.ySize, heli.zSize);
	glEnd();

	// Animate rotor
	glPushMatrix();
    glTranslatef(1.0, 0.6, 0.0);
    glRotatef(rotor, 0.0, 1.0, 0.0);
    // Draw rotor
    glCallList(heliRotorList);
	glPopMatrix();

	glPushMatrix();
	glRotatef(90, 1.0, 0.0, 0.0);
	glTranslatef(-2.7, 0.2, -0.5);
    glRotatef(rotor, 0.0, 1.0, 0.0);
	glScalef(0.4, 0.4, 0.4);

    // Draw tail rotor
    glCallList(heliRotorList);
	glPopMatrix();

    // Draw body
    //glCallList(heliBodyList);
	drawHeliBody();

    glPopMatrix();
}

// Draw the body
void drawHeliBody()
{
	if (heliTextures)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[2]);
	}

	glPushMatrix();
	//
	//     v12               v0---------------v1 \
	//     | \               |                 |  \
    //     |  v8-------------v9                |   v4
    //     |   |             |                 v5  |
    //    v13-v11------------v10               |   v6
    //                       |                 |  /
	//                       v2---------------v3 /
	//

	// Body vertices
	vertex v0 = { -1.3, 1.0, 1.0 };
	vertex v1 = { 0.9, 1.0, 1.0 };
	vertex v2 = { 0.3, -1.0, 1.0 };
	vertex v3 = { 2.5, -1.0, 1.0 };
	vertex v4 = { 2.1, 0.8, 0.5 };
	vertex v5 = { 1.7, 0.0, 1.0 };
	vertex v6 = { 2.7, 0.0, 0.5 };
	vertex v7 = { 2.5, 1.0, 1.0 };
	// Tail vertices
	vertex v8 = { -2.5, 0.4, 0.4 };
	vertex v9 = { -0.8, 0.4, 0.4 };
	vertex v10 = { -2.5, -0.4, 0.4 };
	vertex v11 = { 0.85, -0.4, 0.4 };
	// Tip of tail
	vertex v12 = {-2.8, 1.0, 0.4};
	vertex v13 = {-2.8, -0.4, 0.4};

	// Draw main body
	glColor4f(1.0, 0.4, 0.4, 1.0);	// Light Red
	// Right Side Panel
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v0.x, v0.y, v0.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v1.x, v1.y, v1.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v2.x, v2.y, v2.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v3.x, v3.y, v3.z);
	glEnd();

	// Top Panel
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v0.x, v0.y, v0.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v1.x, v1.y, v1.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v0.x, v0.y, -v0.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v1.x, v1.y, -v1.z);
	glEnd();

	// Bottom Panel
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v2.x, v2.y, v2.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v3.x, v3.y, v3.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v2.x, v2.y, -v2.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v3.x, v3.y, -v3.z);
	glEnd();

	// Left Panel
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v0.x, v0.y, -v0.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v1.x, v1.y, -v1.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v2.x, v2.y, -v2.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v3.x, v3.y, -v3.z);
	glEnd();

	// Rear Panel
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v0.x, v0.y, v0.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v2.x, v2.y, v2.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v0.x, v0.y, -v0.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v2.x, v2.y, -v2.z);
	glEnd();

	// Tip of tail (top)
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v8.x, v8.y, v8.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v12.x, v12.y, v12.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v8.x, v8.y, -v8.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v12.x, v12.y, -v12.z);
	glEnd();

	// Tip of tail (back)
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v12.x, v12.y, v12.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v13.x, v13.y, v13.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v12.x, v12.y, -v12.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v13.x, v13.y, -v13.z);
	glEnd();

	// Tip of tail (side1)
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v12.x, v12.y, v12.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v8.x, v8.y, v8.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v13.x, v13.y, v13.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v11.x, v11.y, v11.z);
	glEnd();

	// Tip of tail (side2)
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v12.x, v12.y, -v12.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v8.x, v8.y, -v8.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v13.x, v13.y, -v13.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v11.x, v11.y, -v11.z);
	glEnd();

	// Tip of tail (bottom)
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);		glVertex3f(v11.x, v11.y, v11.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v13.x, v13.y, v13.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v11.x, v11.y, -v11.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v13.x, v13.y, -v13.z);
	glEnd();
	
	// Front Heli Nose
	glBegin(GL_TRIANGLE_STRIP);
	glColor4f(1.0, 0.4, 0.4, 1.0);	// Light Red
	glTexCoord2f(0.0, 0.0);		glVertex3f(v5.x, v5.y, v5.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v3.x, v3.y, v3.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v6.x, v6.y, v6.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v3.x, v3.y, 0.0);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v6.x, v6.y, -v6.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v3.x, v3.y, -v3.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v5.x, v5.y, -v5.z);
	glEnd();

	// Main Heli Tail
	glBegin(GL_TRIANGLE_STRIP);
	// Left
	glTexCoord2f(0.0, 0.0);		glVertex3f(v8.x, v8.y, v8.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v9.x, v9.y, v9.z);
	glTexCoord2f(1.0, 1.0);		glVertex3f(v10.x, v10.y, v10.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v11.x, v11.y, v11.z);
	// Bottom
	glTexCoord2f(0.0, 0.0);		glVertex3f(v10.x, v10.y, -v10.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v11.x, v11.y, -v11.z);
	// Right
	glTexCoord2f(1.0, 1.0);		glVertex3f(v8.x, v8.y, -v8.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v9.x, v9.y, -v9.z);
	// Top
	glTexCoord2f(0.0, 0.0);		glVertex3f(v8.x, v8.y, v8.z);
	glTexCoord2f(0.0, 1.0);		glVertex3f(v9.x, v9.y, v9.z);
	// Back
	glTexCoord2f(1.0, 1.0);		glVertex3f(v10.x, v10.y, -v10.z);
	glTexCoord2f(1.0, 0.0);		glVertex3f(v10.x, v10.y, v10.z);
	glEnd();

	
	glPushMatrix();
	glTranslatef(v2.x, v2.y, v2.z);
	// Change texture for chair
	glBindTexture(GL_TEXTURE_2D, textures[4]);

	//Chair
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);
	glTranslatef(0.5, 0.9, -1.0);
	glScalef(0.2, 1.5, 1.5);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);
	glTranslatef(1.0, 0.25, -1.0);
	glScalef(0.75, 0.2, 1.5);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the Skids
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glTranslatef(0.5, -0.5, 0.0);
	glutSolidCylinder(0.125, 0.25, 15.0, 15.0);
	glTranslatef(1.0, 0.0, 0.0);
	glutSolidCylinder(0.125, 0.25, 15.0, 15.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.0, -0.3, -0.5);
	glScalef(2.5, 0.2, 0.2);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glTranslatef(0.5, -1.5, 0.0);
	glutSolidCylinder(0.125, 0.25, 15.0, 15.0);
	glTranslatef(1.0, 0.0, 0.0);
	glutSolidCylinder(0.125, 0.25, 15.0, 15.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.0, -0.3, -1.5);
	glScalef(2.5, 0.2, 0.2);
	glutSolidCube(1.0);
	glPopMatrix();
	// End Skids
	glPopMatrix();


	// Rotate cockpit windscreen
	glTranslatef(v1.x, v1.y, v1.z);		// Translate v1
	glRotatef(windscreenRot, 0.0, 0.0, 1.0);		// Rotate
	glTranslatef(-v1.x, -v1.y, -v1.z);	// Translate -v1

	// Draw cockpit window
	glColor4f(0.0, 0.0, 0.6, 0.5);	// Blue
	// Right Screen
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v4.x, v4.y, v4.z);
	glVertex3f(v5.x, v5.y, v5.z);
	glVertex3f(v6.x, v6.y, v6.z);
	glEnd();

	// Front top screen
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v4.x, v4.y, v4.z);
	glVertex3f(v1.x, v1.y, 0.0);
	glVertex3f(v4.x, v4.y, -v4.z);
	glVertex3f(v1.x, v1.y, -v1.z);
	glEnd();

	// Front mid screen
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(v4.x, v4.y, v4.z);
	glVertex3f(v4.x, v4.y, -v4.z);
	glVertex3f(v6.x, v6.y, v6.z);
	glVertex3f(v6.x, v6.y, -v6.z);
	glEnd();

	// Left Screen
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(v1.x, v1.y, -v1.z);
	glVertex3f(v4.x, v4.y, -v4.z);
	glVertex3f(v5.x, v5.y, -v5.z);
	glVertex3f(v6.x, v6.y, -v6.z);
	glEnd();

	glPopMatrix();

	if (heliTextures)
	{
		glDisable(GL_TEXTURE_2D);
	}
}

// Draw the rotor blades
void drawHeliRotor()
{
        glPushMatrix();
        glTranslatef(0.0, 0.8, 0.0);

        glPushMatrix();     
        // Make color blue
        glColor3f(0.0f,0.0f,1.0f);
        // Draw rotor axle
        glRotatef(90, 1.0, 0.0, 0.0);
        glutSolidCylinder(0.4, 0.4, 15, 15);
        glPopMatrix();

        glPushMatrix();
        // Make color grey
        glColor3f(0.8, 0.8, 0.8);
        // Draw blades
        glTranslatef(0.0, 0.05, 0.0);
        glScalef(3.0, 0.1, 0.2);
        glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
        glRotatef(90, 0.0, 1.0, 0.0);
        glScalef(3.0, 0.1, 0.2);
        glutSolidCube(1.0);
        glPopMatrix();

        glPopMatrix();
}

// Make a yellow ground square with 2 x groundSize width and length
void drawGround(void)
{
	glEnable(GL_TEXTURE_2D);
	
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	// Make color yellow
	glColor4f(1.0, 1.0, 1.0, 1.0);

	// Draw the ground
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);						glVertex3f(-groundSize, groundHeight, groundSize);
	glTexCoord2f(groundSize/2, 0.0);			glVertex3f(groundSize, groundHeight, groundSize);
	glTexCoord2f(groundSize/2, groundSize/2);	glVertex3f(groundSize, groundHeight, -groundSize);
	glTexCoord2f(0.0, groundSize/2);			glVertex3f(-groundSize, groundHeight, -groundSize);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawCheckpoint(int checkpoint)
{
	// draw checkpoint number
	const int POINT_NUM_STR = 2;
	char* pointStr = new char[POINT_NUM_STR];
	sprintf(pointStr, "%.2i", checkpoint + 1);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glRasterPos3f(points[checkpoint].xPos, points[checkpoint].yPos, points[checkpoint].zPos);
	for(int i = 0; i < POINT_NUM_STR; i++)
	{
		glutBitmapCharacter((void *)font, pointStr[i]);
	}

	glPushMatrix();
	
	if (points[checkpoint].activated)
	{
		// If the checkpoint has been activated, draw green
		glColor4f(0.0, 1.0, 0.0, 0.5);
	}
	else
	{
		// Otherwise draw red
		glColor4f(1.0, 0.0, 0.0, 0.5);
	}

	// Draw the checkpoint
	glTranslatef(points[checkpoint].xPos, points[checkpoint].yPos, points[checkpoint].zPos);
	glRotatef(points[checkpoint].rotY, 0.0, 1.0, 0.0);
	glScalef(points[checkpoint].xSize, points[checkpoint].ySize, points[checkpoint].zSize);
	glutSolidCube(1.0);
	glPopMatrix();
}

void drawLandingPad(objectBox pad, int textureNum)
{
	// Use planes to determine how far the texture is stretched/tiled
	GLfloat texParamsX[4] = {0.4, 0.0, 0.2, 0.0};
	GLfloat texParamsY[4] = {0.0, 0.4, 0.2, 0.0};

	glPushMatrix();
	// Draw a landing pad
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[textureNum]);
	glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, texParamsX);
	glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, texParamsY);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glTranslatef(pad.xPos, pad.yPos, pad.zPos);
	glRotatef(-90, 1.0, 0.0, 0.0);
	glutSolidCylinder(pad.xSize, pad.ySize, 10, 10);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glPopMatrix();
}

void checkBounds(void)
{
    // If outside of x bounds, move to within
    if (heli.xPos < -groundSize)
    {      
        heli.xPos = -groundSize;
        eye.xPos = -groundSize + cameraDistance;
	}
    else if (heli.xPos > groundSize)
    {
        heli.xPos = groundSize;
        eye.xPos = groundSize + cameraDistance;
    }

    // If outside of z bounds, move to within
    if (heli.zPos < -groundSize)
    {
        heli.zPos = -groundSize;
        eye.zPos = -groundSize + cameraDistance;
    }
    else if (heli.zPos > groundSize)
    {
        heli.zPos = groundSize;
        eye.zPos = groundSize + cameraDistance;
    }
}

bool checkBoxCollision(objectBox object1, objectBox object2)
{
    bool collision = false;
    objectBox diff = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	objectBox object1a = object1;
	objectBox object2a = object2;

	// Rotate size in x and z to correspond with rotation of the object
	object1a.xSize = cosDeg(object2a.rotY) * object2a.xSize + sinDeg(object2a.rotY) * object2a.zSize;
	object1a.zSize = -sinDeg(object2a.rotY) * object2a.xSize + cosDeg(object2a.rotY) * object2a.zSize;

	object2a.xSize = cosDeg(object2a.rotY) * object2a.xSize + sinDeg(object2a.rotY) * object2a.zSize;
	object2a.zSize = -sinDeg(object2a.rotY) * object2a.xSize + cosDeg(object2a.rotY) * object2a.zSize;

    // compute the absolute (positive) distance from object1 to object2
    diff.xPos = abs(object1.xPos - object2.xPos);
    diff.yPos = abs(object1.yPos - object2.yPos);
    diff.zPos = abs(object1.zPos - object2.zPos);
    diff.xSize = object1a.xSize + object2a.xSize;
    diff.ySize = object1a.ySize + object2a.ySize;
    diff.zSize = object1a.zSize + object2a.zSize;

    // If the distance between each of the three dimensions is within the radii combined, there is a collision
    if(diff.xPos < diff.xSize && diff.yPos < diff.ySize && diff.zPos < diff.zSize)
    {
            collision = true;
    }

    return collision;
}

bool checkPointCollision(objectBox object1, checkPoint object2)
{
	bool collision = false;

	objectBox diff = {0, 0, 0, 0};
	objectBox object1a = object1;
	checkPoint object2a = object2;

	// Rotate size in x and z to correspond with rotation of the object
	object1a.xSize = abs( cosDeg(object2a.rotY) * object2a.xSize + -sinDeg(object2a.rotY) * object2a.zSize );
	object1a.zSize = abs( sinDeg(object2a.rotY) * object2a.xSize + cosDeg(object2a.rotY) * object2a.zSize );

	object2a.xSize = cosDeg(object2a.rotY) * object2a.xSize + -sinDeg(object2a.rotY) * object2a.zSize;
	object2a.zSize = sinDeg(object2a.rotY) * object2a.xSize + cosDeg(object2a.rotY) * object2a.zSize;

	// Compute the absolute (positive) distance from object1 to object2
	diff.xPos = abs(object1.xPos - object2.xPos);
	diff.yPos = abs(object1.yPos - object2.yPos);
	diff.zPos = abs(object1.zPos - object2.zPos);
	diff.xSize = object2a.xSize;
	diff.ySize = object2a.ySize;
	diff.zSize = object2a.zSize;

    // If the distance between each of the three dimensions is within the radii combined, there is a collision
    if(diff.xPos < diff.xSize && diff.yPos < diff.ySize && diff.zPos < diff.zSize)
    {
            collision = true;
    }

	return collision;
}

/*bool checkSphereBoxCollision(object object1, objectBox object2)
{
	bool collision = false;

	objectBox diff = {0, 0, 0, 0};

    //compute the absolute (positive) distance from object1 to object2
    diff.x = abs(object1.x - object2.x);
    diff.y = abs(object1.y - object2.y);
    diff.z = abs(object1.z - object2.z);
    diff.radx = object1.rad + object2.radx;
    diff.rady = object1.rad + object2.rady;
    diff.radz = object1.rad + object2.radz;

    // If the distance between each of the three dimensions is within the radii combined, there is a collision
    if(diff.x < diff.radx && diff.y < diff.rady && diff.z < diff.radz)
    {
            collision = true;
    }

	return collision;
}*/

void checkHeliThruCollisions(void)
{
	// for each checkpoint, if it is not activated, check for collision with helicopter
	for (int pointNum = 0; pointNum < MAX_CHECKPOINTS; pointNum++)
	{
		if ( !points[pointNum].activated )
		{
			if ( checkPointCollision(heli, points[pointNum]) )
			{
				// If the checkpoint entered is not the checkpoint that is next, penalize 10s per gate missed
				if ( checkpointNum != pointNum )
				{
					penaltyTime += (pointNum - checkpointNum) * 10000;
					// Activate all the points leading up to incorrectly activated point
					for (checkpointNum; checkpointNum < pointNum; checkpointNum++)
					{
						points[checkpointNum].activated = true;
					}
				}
				else
				{
					points[pointNum].activated = true;
					checkpointNum = pointNum + 1;
				}
			}
		}
	}
}

void checkHeliLanding(void)
{
	// If landed on landing pad B
	if ( checkBoxCollision(heli, landingPadB) )
	{
		// If engine is off and all checkpoints have been activated
		if ( !helicopterOn && !stopHeli && checkpointNum == MAX_CHECKPOINTS )
		{
				gameFinished = true;
		}
	}
}

void checkHeliCollisions(void)
{
    bool collision = false;

    if ( checkBoxCollision(heli, building0) )
    {
            collision = true;
    }

    if (collision)
    {
        if (movingForward)
        {
                moveHeliBack(heliSpeed, false);
        }
        else if (movingBack)
        {
                moveHeliForward(heliSpeed, false);
        }

        if (movingUp)
        {
                moveHeliDown(heliSpeed, false);
        }
        else if (movingDown)
        {
                moveHeliUp(heliSpeed, false);
        }
    }
}

// Converts degrees (in) to radians and returns the cosine (out)
float cosDeg(float degRot)
{
    float radRot = degRot * pi/180;
    return (cos(radRot));
}

// Converts degrees(in) to radians and returns the sine (out)
float sinDeg(float degRot)
{
    float radRot = degRot * pi/180;
	return (sin(radRot));
}

void restartGame()
{
	gameFinished = false;

	// Reset checkpoints
	checkpointNum = 0;
	for (int num = 0; num < MAX_CHECKPOINTS; num++)
	{
		points[num].activated = false;
	}

	// Reset time
	gameTime = 0;
	penaltyTime = 0;

	// Reset heli position
	heli.xPos = 0;
	heli.yPos = groundHeight + heli.ySize;
	heli.zPos = 0;
	heli.rotY = 0;

	// Reset eye position
	eye.xPos = cameraDistance;
	eye.yPos = heli.yPos;
	eye.zPos = cameraDistance;
	eye.rotY = 135;
}

// Catches keyboard key presses
void keyboard(unsigned char key, int mouseX, int mouseY)
{
	if(pause)
	{
		pause = false;
	}
	
	// If game is finished and enter key is pressed
	if (gameFinished && key == 13)
	{
		restartGame();
	}

    switch (key)
    {
        // If ESC key is pressed, exit
        case 27:
            glutLeaveMainLoop();
            break;
		case '1':
			if( windscreenRot > 0 )
				windscreenRot = 0.0;
			else
				windscreenRot = 35.0;
			glutPostRedisplay();
			break;
        case 'a':
            // Start moving the heli up
            movingUp = true;
            break;
        case 'z':
            // Start moving the heli down
            movingDown = true;
            break;
		case 's':
			// Start Blades
			startHeli = true;
			break;
		case 'x':
			//Stop Blades
			stopHeli = true;
			break;
    }
}

void keyboardUp(unsigned char key, int mouseX, int mouseY)
{
    switch (key)
    {
        case 'a':
                // Stop moving the heli up
                movingUp = false;
                break;
        case 'z':
                // Stop moving the heli down
                movingDown = false;
                break;
    }
}

void shadingOn()
{
	if (wire == true)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	wire = false;
}

void wireFrameOn()
{
	if (wire == false)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	wire = true;

}

void heliTexturesOn()
{
		heliTextures = true;
}

void heliTexturesOff()
{
		heliTextures = false;
}

// Catches special key presses
void special(int key, int mouseX, int mouseY)
{
	// If paused, unpause
	if (pause && key != GLUT_KEY_F1)
	{
		pause = false;
	}

    switch (key)
    {
        case GLUT_KEY_LEFT:
                // rotate heli left
                turningLeft = true;
                break;
        case GLUT_KEY_RIGHT:
                // rotate heli right
                turningRight = true;
                break;
        case GLUT_KEY_UP:
                // move "forward"
                movingForward = true;
                break;
        case GLUT_KEY_DOWN:
                // move "backward"
                movingBack = true;
                break;
		case GLUT_KEY_F1:
				pause = !pause;
				break;
		case GLUT_KEY_F2:
				if (wire == false)
				{
					wireFrameOn();
				}
				else
				{
					shadingOn();
				}
				break;
		case GLUT_KEY_F3:
				if (heliTextures == false)
				{
					heliTexturesOn();
				}
				else
				{
					heliTexturesOff();
				}
				break;
		case GLUT_KEY_F8:
                // turn the light/s on or off
                light0 = !light0;
				if (light0)
				{                        
					glEnable(GL_LIGHT0);
                }
                else
                {
                    glDisable(GL_LIGHT0);
                }
                break;
		case GLUT_KEY_PAGE_DOWN:
                // Zoom out
				if (cameraZoom > 1.0)
				{
					cameraZoom -= 0.1;
				}
                break;
        case GLUT_KEY_PAGE_UP:
                // Zoom in
				if (cameraZoom < 3.0)
				{
					cameraZoom += 0.1;
					//reshape(windowWidth, windowHeight);
				}
                break;
    }
}


void specialUp(int key, int mouseX, int mouseY)
{
    switch(key)
    {
        case GLUT_KEY_UP:
                // move "forward"
                movingForward = false;
                break;
        case GLUT_KEY_DOWN:
                // move "backward"
                movingBack = false;
                break;
        case GLUT_KEY_LEFT:
                // rotate heli left
                turningLeft = false;
                break;
        case GLUT_KEY_RIGHT:
                // rotate heli right
                turningRight = false;
                break;
    }
}

void mouseMotion(int x, int y)
{
    if (leftMouseDown)
    {
        float rotateY = ( (float)x - (float)last_mouse_x) / (float)windowWidth * 360.0;
		float rotateZ = ( (float)y - (float)last_mouse_y) / (float)windowHeight * 360.0;
        eye.rotY += rotateY;
        eye.rotZ += rotateZ / 2.0;
        last_mouse_x = x;
		last_mouse_y = y;
    }

	if(eye.rotZ > 15 && heli.yPos < heli.ySize)
	{
		eye.rotZ = 15;
	}
}

void mouse(int button, int state, int x, int y)
{
    last_mouse_x = x;
	last_mouse_y = y;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        leftMouseDown = true;
    }
    else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
    {
        eye.rotY = 135;
		eye.rotX = 0;
		eye.rotZ = 0;
    }
    else
    {
        leftMouseDown = false;
    }
}

void moveHeliForward(float speed, bool checkCol)
{
    // Move heli
    heli.xPos += speed * cosDeg(heli.rotY);
    heli.zPos -= speed * sinDeg(heli.rotY);

    // Move camera
    eye.xPos += speed * cosDeg(heli.rotY);
    eye.zPos -= speed * sinDeg(heli.rotY);

    checkHeliCollisions();
	checkHeliThruCollisions();
}

void moveHeliBack(float speed, bool checkCol)
{
    // Move heli
    heli.xPos -= speed * cosDeg(heli.rotY);
    heli.zPos += speed * sinDeg(heli.rotY);

    // Move camera
    eye.xPos -= speed * cosDeg(heli.rotY);
    eye.zPos += speed * sinDeg(heli.rotY);

    checkHeliCollisions();
	checkHeliThruCollisions();
}

void moveHeliUp(float speed, bool checkCol)
{
    heli.yPos += heliSpeed;
    eye.yPos += heliSpeed;

    checkHeliCollisions();
	checkHeliThruCollisions();
}

void moveHeliDown(float speed, bool checkCol)
{
    heli.yPos -= heliSpeed;
    eye.yPos -= heliSpeed;

    checkHeliCollisions();
	checkHeliThruCollisions();
}

void updateGameTime()
{
	if (!pause)
	{
		gameTime += time - gameTimeBase;	// Increment the time spent playing
		int gameTimeMillisec = (gameTime % 1000) / 10;		// Get milliseconds from game time
		int gameTimeSeconds = (gameTime % 60000) / 1000;	// Get seconds from game time
		int gameTimeMinutes = (gameTime % 3600000) / 60000;	// Get minutes from game time
		sprintf(strGameTime, "Time: %.2i:%.2i:%.2i", gameTimeMinutes, gameTimeSeconds, gameTimeMillisec);	
	}

	gameTimeBase = time;
}

void updateFPS()
{
    // Update the FPS every second
    frames++;
    time = glutGet(GLUT_ELAPSED_TIME);

    if (time - timeBase > 1000) // If a second has passed
    {
        fps = frames * 1000.0 / (time - timeBase);              // calculate FPS
        sprintf(strFps, "FPS: %4.2f", fps);		// get the string value of integer FPS
        timeBase = time;        // Set the base time to current time
        frames = 0;     // Reset the frame count
    }
}

void displayText()
{
    // Write text to screen
    glPushMatrix();
    glColor3f(0.0, 1.0, 1.0);
    setOrthographicProjection();
    glLoadIdentity();
	// Display FPS
    renderBitmapString(textX, textY, (void *)font, strFps);

	displayDashboard();

	// If paused, display help/controls/scores
	if (pause)
	{
		displayHelp();
	}

    resetPerspectiveProjection();
    glPopMatrix();
}

void displayHelp()
{
	const int HELP_SPACE = 20;
	const int HELP_YPOS = 100;
	const float MARGIN = 5.0;

	glDisable(GL_DEPTH_TEST);

	// Display a backing screen so that the text is readable
	glColor4f(0.5, 0.5, 0.5, 0.8);	// Grey
	glBegin(GL_QUADS);
	glVertex2f(MARGIN, MARGIN);
	glVertex2f(windowWidth - MARGIN, MARGIN);
	glVertex2f(windowWidth - MARGIN, windowHeight - MARGIN);
	glVertex2f(MARGIN, windowHeight - MARGIN);
	glEnd();

	glColor4f(1.0, 0.7, 0.2, 1.0);	// Orange

	renderBitmapString(HELP_SPACE, HELP_YPOS, (void *)font, "F1 - Pause and bring up this screen");
	renderBitmapString(HELP_SPACE, HELP_YPOS + HELP_SPACE, (void *)font, "F2 - Switch between wireframe and solid shapes");
	renderBitmapString(HELP_SPACE, HELP_YPOS + 2*HELP_SPACE, (void *)font, "F3 - Switch between textures and no textures");
	renderBitmapString(HELP_SPACE, HELP_YPOS + 3*HELP_SPACE, (void *)font, "F8 - Switch Light 0 on/off");
	renderBitmapString(HELP_SPACE, HELP_YPOS + 4*HELP_SPACE, (void *)font, "a/z - Move helicopter up/down");
	renderBitmapString(HELP_SPACE, HELP_YPOS + 5*HELP_SPACE, (void *)font, "Directional Arrows - Move forward/backward");
	renderBitmapString(2*HELP_SPACE, HELP_YPOS + 6*HELP_SPACE, (void *)font, " and Turn left/right");
	renderBitmapString(HELP_SPACE, HELP_YPOS + 7*HELP_SPACE, (void *)font, "s/x - Start/stop engine");
	renderBitmapString(HELP_SPACE, HELP_YPOS + 8*HELP_SPACE, (void *)font, "Right Mouse - Brings up game menu");
	renderBitmapString(HELP_SPACE, HELP_YPOS + 9*HELP_SPACE, (void *)font, "Drag Left Mouse - Rotate camera around helicopter");
	renderBitmapString(HELP_SPACE, HELP_YPOS + 10*HELP_SPACE, (void *)font, "Middle Mouse - Reset camera to chase position");

	// Display the best time
	int bestTimeMinutes = (bestTime % 1000) / 10;
	int bestTimeSeconds = (bestTime % 60000) / 1000;
	int bestTimeMillisec = (bestTime % 3600000) / 60000;

	sprintf(strBestTime, "Best Time: %.2i:%.2i:%.2i", bestTimeMinutes, bestTimeSeconds, bestTimeMillisec);
	renderBitmapString(HELP_SPACE, 350, (void *)font, strBestTime);

	// Display who wrote the project and it's purpose
	renderBitmapString(HELP_SPACE, 400, (void *)font, "Written by Aleesha, Ashley and Chris");
	renderBitmapString(HELP_SPACE, 425, (void *)font, " for Cp2060 Assignment 2");

	glEnable(GL_DEPTH_TEST);
}

void displayDashboard()
{
	const float DASH_HEIGHT_PC = 0.2;
	const float DASH_WIDTH_PC = 0.0;

	float dashHeight = windowHeight - (DASH_HEIGHT_PC * windowHeight);
	float dashWidth = windowWidth - (DASH_WIDTH_PC * windowWidth);

	glDisable(GL_DEPTH_TEST);

	// Draw the dash board
	glColor4f(0.5, 0.5, 0.5, 1.0);	// Grey
	glBegin(GL_QUADS);
	glVertex2f(0.0, dashHeight);
	glVertex2f(dashWidth, dashHeight);
	glVertex2f(dashWidth, windowHeight);
	glVertex2f(0.0, windowHeight);
	glEnd();

	glColor4f(0.0, 1.0, 0.5, 1.0);	// Green
	// Display game time
	renderBitmapString(30, dashHeight + 30, (void *)font, strGameTime);

	// Display speed
	char* strHeliSpeed = new char[12];
	sprintf(strHeliSpeed, "Speed: %.2f", heliSpeed);
	renderBitmapString(dashWidth / 2.3, dashHeight + 30, (void *)font, strHeliSpeed);

	// Display altitude
	char* strAltitude = new char[14];
	sprintf(strAltitude, "Altitude: %.2f", heli.yPos - heli.ySize);
	renderBitmapString(3 * (dashWidth / 4), dashHeight + 30, (void *)font, strAltitude);

	// Display checkpoint number
	char* strCheckpoint = new char[14];
	sprintf(strCheckpoint, "Checkpoint: %.2i/%.2i", checkpointNum, MAX_CHECKPOINTS);
	renderBitmapString(3 * (dashWidth / 5), dashHeight + 60, (void *)font, strCheckpoint);

	// Display altitude
	char* strPenalty = new char[22];
	int penaltyTimeMinutes = (penaltyTime % 1000) / 10;
	int penaltyTimeSeconds = (penaltyTime % 60000) / 1000;
	int penaltyTimeMillisec = (penaltyTime % 3600000) / 60000;

	sprintf(strPenalty, "Penalty Time: %.2i:%.2i:%.2i", penaltyTimeMinutes, penaltyTimeSeconds, penaltyTimeMillisec);
	renderBitmapString(30, dashHeight + 60, (void *)font, strPenalty);

	glEnable(GL_DEPTH_TEST);
}

char* getTimeString(int time)
{
	char* strTime = new char[10];
	int timeMSec = (time % 1000) / 10;
	int timeSec = (time % 60000) / 1000;
	int timeMin = (time % 3600000) / 60000;
	sprintf( strTime, "%.2i:%.2i:%.2i", timeMin, timeSec, timeMSec);
	return strTime;
}

void drawFinishScreen()
{
	glColor4f(0.5, 1.0, 0.5, 1.0);	// Light Green
	char* strFinishTime = new char[30];
	int totalTime = gameTime + penaltyTime;
	char* strTotalTime = getTimeString(totalTime);
	char* strBestTime = getTimeString(bestTime);

	sprintf( strFinishTime, "Your time is: %s", strTotalTime);
	renderBitmapString(20, 20, (void *)font, "Race Completed");
	renderBitmapString(40, 40, (void *)font, strFinishTime);

	// Do calculations vs best time and display appropriate message
	if ( totalTime < bestTime )
	{
		char* strOldBest = new char[30];
		renderBitmapString(20, 60, (void *)font, "Congratulations you have a new best time!");
		sprintf(strOldBest, "The old best time was: %s", strBestTime);
		renderBitmapString(20, 80, (void *)font, strOldBest);
		// Output new time to saved file

	}
	else
	{
		renderBitmapString(20, 60, (void *)font, "You have not beaten the best time.");
		renderBitmapString(20, 80, (void *)font, "Would you like to try again?");
	}

	// If you would like to play again, press Enter
	renderBitmapString(20, 100, (void *)font, "If you would like to play again, press Enter.");
	// Otherwise, press ESC to quit
	renderBitmapString(20, 120, (void *)font, "Otherwise press ESC to quit.");

	int MARGIN = 0;
	// Display a backing screen so that the text is readable
	glColor4f(0.5, 0.5, 0.5, 1.0);	// Grey
	glBegin(GL_QUADS);
	glVertex2f(MARGIN, MARGIN);
	glVertex2f(windowWidth - MARGIN, MARGIN);
	glVertex2f(windowWidth - MARGIN, windowHeight - MARGIN);
	glVertex2f(MARGIN, windowHeight - MARGIN);
	glEnd();
}

// These next three functions are taken from Lighthouse 3D tutorials:
// http://www.lighthouse3d.com/opengl/glut/index.php?bmpfontortho
void setOrthographicProjection()
{
    // switch to projection mode
    glMatrixMode(GL_PROJECTION);
    // save previous matrix which contains the
    //settings for the perspective projection
    glPushMatrix();
    // reset matrix
    glLoadIdentity();
    // set a 2D orthographic projection
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    // invert the y axis, down is positive
    glScalef(1, -1, 1);
    // move the origin from the bottom left corner
    // to the upper left corner
    glTranslatef(0, -windowHeight, 0);
    glMatrixMode(GL_MODELVIEW);
}

// Set projection back to it's starting point
void resetPerspectiveProjection()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Draw the string on the screen
void renderBitmapString(float x, float y, void *font,char *string)
{
    char *c;
    // Set the draw co-ordinates
    glRasterPos2f(x, y);

    for (c=string; *c != '\0'; c++)
    {
        // Display each character in the array
        glutBitmapCharacter(font, *c);
    }
}
// End lighthouse 3D code

// When there's nothing else to do, update animation
void idle(void)
{
	if (!helicopterOn)
	{
		checkHeliLanding();
	}

	if (stopHeli)
	{
		if (rotorSpeed > 0)
		{
			rotorSpeed = rotorSpeed - 0.2; 
		}

		if (heli.yPos > groundHeight + heli.ySize)
		{
			heli.yPos -= 0.02;
			eye.yPos -= 0.02;

			if (heli.yPos < groundHeight + heli.ySize)
			{
				heli.yPos = groundHeight + heli.ySize;
			}
		}
		else if (rotorSpeed <= 0)
		{
			stopHeli = false;
		}

		if (rotorSpeed < MAX_ROTOR_SPEED) 
		{
			helicopterOn = false;
		}
	}
	else if (startHeli)
	{
		if (rotorSpeed < MAX_ROTOR_SPEED)
		{
			rotorSpeed = rotorSpeed + 0.2; 
			heli.yPos += 0.02; 
			eye.yPos += 0.02; 
		} 
		else
		{
			helicopterOn = true;
			startHeli = false;
		}
	}

	if(!pause && !gameFinished)
	{
		if (helicopterOn == true)
		{
			// Update speed
			heliSpeed = HELI_SPEED / fps;
			if (movingForward)
			{
				// move forward
				moveHeliForward(heliSpeed, true);
				heliLeanFront = -LEAN_FACTOR;
			
			}
			else if (movingBack)
			{
				// move back
				moveHeliBack(heliSpeed, true);
				heliLeanFront = LEAN_FACTOR;
			}
			else
			{
				// Reset the front lean and straighten up
				heliLeanFront = 0;
			}

			if (turningLeft)
			{
				// turn left
				heli.rotY += rotSpeed;
				// Adjust the rotor spin to counter heli spin
				rotor += rotorSpeed - rotSpeed;
				heliLeanSide = -LEAN_FACTOR;
			}
			else if (turningRight)
			{
				// turn right
				heli.rotY -= rotSpeed;
				// Adjust the rotor spin to counter heli spin
				rotor += rotorSpeed + rotSpeed;
				heliLeanSide = LEAN_FACTOR;
			}
			else
			{
				// Turn the rotor normally
				rotor += rotorSpeed;
				// Reset the lean and straighten up
				heliLeanSide = 0;
			}
		}
		else
		{
			//turn the rotor normally
			rotor += rotorSpeed;
			heliLeanSide = 0;
		}

		if (helicopterOn == true)
		{
			if (movingUp)
			{
				moveHeliUp(heliSpeed, true);
			}
			else if (movingDown)
			{
				if(heli.yPos > groundHeight + heli.ySize)
				{
					moveHeliDown(heliSpeed, true);
				}

				if(heli.yPos < groundHeight + heli.ySize)
				{
					heli.yPos = groundHeight + heli.ySize;
				}
			}
		}

		// Make sure heli is in the level's bounds
        checkBounds();
	}

	glutPostRedisplay();
}

// When the window is reshaped, this function updates the camera and display
void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, cameraZoom, 60.0);
    glMatrixMode(GL_MODELVIEW);

	windowWidth = w;
	windowHeight = h;
}


// Display the scene and it's components
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

	if ( !gameFinished )
	{
		gluLookAt(eye.xPos, eye.yPos, eye.zPos, heli.xPos, heli.yPos, heli.zPos, 0.0, 1.0, 0.0);

		// Rotate camera so that it is always behind the heli
		glPushMatrix();
		glTranslatef(heli.xPos, heli.yPos, heli.zPos);
		glRotatef(eye.rotZ, 0.0, 0.0, 1.0);
		glRotatef(-heli.rotY + eye.rotY, 0.0, 1.0, 0.0);
		glTranslatef(-heli.xPos, -heli.yPos, -heli.zPos);

		//Draw Sky
		glPushMatrix;
		drawSky();
		glPopMatrix;


		// Draw ground
		glPushMatrix();
		glTranslatef(0, groundHeight, 0);
		glCallList(groundList);
		glPopMatrix();

		// Draw building
		glPushMatrix();
		drawBuilding(building0, 1);
		glPopMatrix();

		// Draw the helicopter
		drawHeli();

		// Draw landing pads A and B
		drawLandingPad(landingPadA, 3);
		drawLandingPad(landingPadB, 4);

		// Draw the checkpoints
		for (int checkPoint = 0; checkPoint < MAX_CHECKPOINTS; checkPoint++)
		{
			drawCheckpoint( points[checkPoint].checkpoint);
		}
		
		updateFPS();
		updateGameTime();
		displayText();
		displayDashboard();

		glPopMatrix();
	}
	else
	{
		setOrthographicProjection();
		drawFinishScreen();
		resetPerspectiveProjection();
	}
    glutSwapBuffers();
}

//adding menu

void mymenu(int choice)
{

	switch (choice) {
	case 1: shadingOn(); // Full Shading
		break;
	case 2: wireFrameOn();  // Wireframe
		break;
	case 3: heliTexturesOn(); // enable texture mapping
		break;
	case 4: heliTexturesOff(); // disable texture mapping
		break;
	case 5: pause = !pause; // Help/Credits/Scores
		break;
	case 6: exit(0);  // exit program
		break;
	default: break;
	}
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(windowPosWidth, windowPosHeight);
    glutCreateWindow("Cp2060 Assignment 2 - Heliaac");
    init();
    glutDisplayFunc(display);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);

	// create sub menu 1
	int subMenu1 = glutCreateMenu(mymenu);
	glutAddMenuEntry("Full Shading", 1);
	glutAddMenuEntry("Wire Frame", 2);

	// create sub menu 2 
	int subMenu2 = glutCreateMenu(mymenu);
	glutAddMenuEntry("Enable", 3);
	glutAddMenuEntry("Disable", 4);

	//create sub menu 3
	int subMenu3 = glutCreateMenu(mymenu);
	glutAddMenuEntry("Yes", 6);
	glutAddMenuEntry("No", 0);

	// create main menu (no ID needed)
	glutCreateMenu(mymenu);
	glutAddSubMenu("Shading", subMenu1);
	glutAddSubMenu("Helicopter Textures", subMenu2);
	glutAddMenuEntry("Help/Credits/Scores", 5);
	glutAddSubMenu("Exit Program", subMenu3);
	// set the menu to the right button
	glutAttachMenu(GLUT_RIGHT_BUTTON);  

    glutMainLoop();
    return 0;
}