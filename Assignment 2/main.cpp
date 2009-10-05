// Assignment 2.cpp
//Heliaac
//Christopher Trott, Ashley Sexton, Aleesha Torkington
// Created on 28/08/09
// Last Modified on 28/08/09 @ 15:03
//
// Some of this code is taken from animlightpos.cpp on the LearnJCU resources page
// Some code to do with lighting was gained from the URL: http://www.falloutsoftware.com/tutorials/gl/gl8.htm
// Some code to do with text on screen gained from Lighthouse 3D @ URL: http://www.lighthouse3d.com/opengl/glut/index.php?bmpfontortho

#include<GL/freeglut.h>
#include<math.h>
#include<string>
#include<iostream>

using namespace std;


struct vector
{
        float x;
        float y;
        float z;
};

struct object
{
        float x;        // x position
        float y;        // y position
        float z;        // z position
        float rot;      // rotation angle of direction
        float rad;      // radius of bounding sphere
};

struct objectBox
{
        float x;        // x position
        float y;        // y position
        float z;        // z position
        float rot;      // rotation angle of direction
        float radx;     // x radius (half-length) of bounding box
        float rady;     // y radius of bounding box
        float radz;     // z radius of bounding box
};

void drawHeliBody(void);
void drawHeliRotor(void);
void drawGround(void);
void moveHeliForward(float speed, bool checkCol);
void moveHeliBack(float speed, bool checkCol);
void moveHeliDown(float speed, bool checkCol);
void moveHeliUp(float speed, bool checkCol);
void checkBounds(void);
bool checkSphereCollision(object object1, object object2);
bool checkBoxCollision(objectBox object1, objectBox object2);
bool checkSphereBoxCollision(object object1, objectBox object2);
void checkHeliCollisions(void);
void drawBuilding(void);
void updateFPS(void);
void updateGameTime(void);
void displayText(void);
void resetPerspectiveProjection(void);
void setOrthographicProjection(void);
void renderBitmapString(float x, float y, void *font,char *string);
void displayHelp(void);


float cameraDistance = 5.0;
object heli = {0, 2, 0, 0, 2};

objectBox eye = {cameraDistance, heli.y, cameraDistance, 135, 0, 0, 0};
objectBox building0 = {10, 5, 10, 0, 2, 5, 2};

bool movingForward = false;
bool movingBack = false;
bool movingUp = false;
bool movingDown = false;
bool turningLeft = false;
bool turningRight = false;

int font = (int)GLUT_BITMAP_HELVETICA_18;
int textX = 20;
int textY = 20;

bool light0 = true;
bool light1 = false;

//trying to implement start/stop without changing functions too much
bool helicopterOn = false;

float x = 1, y = 20, z = 1;
GLfloat light0_position[] = { x, y, z, 0 };

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
int bestTime = 0;
char* strBestTime = new char[8];
int gameTime = 0;
int gameTimeBase = 0;

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
bool rightMouseDown = false;

float pi = 3.1415926535897932384626433832795;

// Initialise the OpenGL properties
void init(void)
{
        glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
        glShadeModel(GL_FLAT);

        // Make object materials equal to glColor3f() properties
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);

        // Define the heliBody display list
        heliBodyList = glGenLists(1);
        glNewList(heliBodyList, GL_COMPILE);
        drawHeliBody();
        glEndList();
       
        // Define the heliRotor display list
        heliRotorList = glGenLists(1);
        glNewList(heliRotorList, GL_COMPILE);
        drawHeliRotor();
        glEndList();

        // Define the ground display list
        groundList = glGenLists(1);
        glNewList(groundList, GL_COMPILE);
        drawGround();
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
}

void drawBuilding(void)
{
        glPushMatrix();
        glColor3f(0.5, 0.5, 0.5);
        glRotatef(building0.rot, 0.0, 1.0, 0.0);
        glTranslatef(building0.x, building0.y, building0.z);
        glScalef(2 * building0.radx, 2 * building0.rady, 2 * building0.radz);
        glutSolidCube(1.0);
        glPopMatrix();
}

// Draw the body
void drawHeliBody()
{
	glPushMatrix();
        //// Make color red
        //glTranslatef(0.6, 0.0, 0.0);
        //glColor3f(1.0, 0.0, 0.0);
        //glutSolidCube(2.0);
        //// Draw the tail boom
        //glTranslatef(-1.2, 0.0, 0.0);
        //glScalef(1.8, 0.4, 0.4);
        //glutSolidCube(2.0);
        //glPopMatrix();
	
	//Side 1:
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_POLYGON);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.5, 0.0, 0.0);
	glVertex3f(0.5, 0.5, 0.0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.5, 0.5, 0.0);
	glVertex3f(0.5, 1.0, 0.0);
	glEnd();


	glBegin(GL_POLYGON);
	glVertex3f(0.5, 0.0, 0.0);
	glVertex3f(2.0, 0.0, 0.0);
	glVertex3f(2.0, 1.0, 0.0);
	glVertex3f(0.5, 1.0, 0.0);
	glEnd();

	//Needs to be transparent
	glBegin(GL_POLYGON);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.5, 1.0, 0.0);
	glVertex3f(2.0, 1.0, 0.0);
	glVertex3f(2.0, 2.0, 0.0);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(2.0, 0.0, 0.0);
	glVertex3f(3.0, 0.0, 0.0);
	glVertex3f(3.0, 2.0, 0.0);
	glVertex3f(2.0, 2.0, 0.0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(2.0, 2.0, 0.0);
	glVertex3f(3.0, 2.0, 0.0);
	glVertex3f(3.0, 3.0, 0.0);
	glEnd();

	//Top:

	glBegin(GL_POLYGON);
	glVertex3f(3.0, 3.0, 0.0);
	glVertex3f(3.0, 3.0, -1.0);
	glVertex3f(2.0, 2.0, -1.0);
	glVertex3f(2.0, 2.0, 0.0);
	glEnd();

	//Needs to be transparent:
	glBegin(GL_POLYGON);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(2.0, 2.0, 0.0);
	glVertex3f(2.0, 2.0, -1.0);
	glVertex3f(0.5, 1.0, -1.0);
	glVertex3f(0.5, 1.0, 0.0);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.5, 1.0, 0.0);
	glVertex3f(0.5, 1.0, -1.0);
	glVertex3f(0.0, 0.0, -1.0);
	glVertex3f(0.0, 0.0, 0.0);

	//Another side:
	glBegin(GL_POLYGON);
	glVertex3f(0.0, 0.0, -1.0);
	glVertex3f(0.5, 0.0, -1.0);
	glVertex3f(0.5, 0.5, -1.0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.0, 0.0, -1.0);
	glVertex3f(0.5, 0.5, -1.0);
	glVertex3f(0.5, 1.0, -1.0);
	glEnd();


	glBegin(GL_POLYGON);
	glVertex3f(0.5, 0.0, -1.0);
	glVertex3f(2.0, 0.0, -1.0);
	glVertex3f(2.0, 1.0, -1.0);
	glVertex3f(0.5, 1.0, -1.0);
	glEnd();

	//Needs to be transparent
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_POLYGON);
	glVertex3f(0.5, 1.0, -1.0);
	glVertex3f(2.0, 1.0, -1.0);
	glVertex3f(2.0, 2.0, -1.0);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(2.0, 0.0, -1.0);
	glVertex3f(3.0, 0.0, -1.0);
	glVertex3f(3.0, 2.0, -1.0);
	glVertex3f(2.0, 2.0, -1.0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(2.0, 2.0, -1.0);
	glVertex3f(3.0, 2.0, -1.0);
	glVertex3f(3.0, 3.0, -1.0);
	glEnd();
	
	//Back
	glBegin(GL_POLYGON);
	glVertex3f(3.0, 3.0, 0.0);
	glVertex3f(3.0, 3.0, -1.0);
	glVertex3f(3.0, 0.0, -1.0);
	glVertex3f(3.0, 0.0, 0.0);
	glEnd();

	glPopMatrix();

	//Tail
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	glTranslatef(3.5, 1.0, -0.5);
	glScalef(3.0, 0.4, 0.4);
	glutSolidCube(1.0);
	glPopMatrix();
}

// Draw the rotor blades
void drawHeliRotor()
{
        glPushMatrix();
        glTranslatef(0.0, 1.2, 0.0);

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
        glTranslatef(0.0, 0.1, 0.0);
        glScalef(4.0, 0.2, 0.2);
        glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
        glRotatef(90, 0.0, 1.0, 0.0);
        glScalef(4.0, 0.2, 0.2);
        glutSolidCube(1.0);
        glPopMatrix();

        glPopMatrix();
}

// Make a yellow ground square with 2 x groundSize width and length
void drawGround(void)
{
        // Make color yellow
        glColor3f(1.0, 1.0, 0.0);
        // Draw the ground
        glBegin(GL_QUADS);
        glVertex3f(-groundSize, groundHeight, -groundSize);
        glVertex3f(groundSize, groundHeight, -groundSize);
        glVertex3f(groundSize, groundHeight, groundSize);
        glVertex3f(-groundSize, groundHeight, groundSize);
        glEnd();
}

void checkBounds(void)
{
        // If outside of x bounds, move to within
        if (heli.x < -groundSize)
        {      
                heli.x = -groundSize;
                eye.x = -groundSize + cameraDistance;
        }
        else if (heli.x > groundSize)
        {
                heli.x = groundSize;
                eye.x = groundSize + cameraDistance;
        }

        // If outside of z bounds, move to within
        if (heli.z < -groundSize)
        {
                heli.z = -groundSize;
                eye.z = -groundSize + cameraDistance;
        }
        else if (heli.z > groundSize)
        {
                heli.z = groundSize;
                eye.z = groundSize + cameraDistance;
        }
}

bool checkSphereCollision(object object1, object object2)
{
        bool collision = false;
        object diff = {0, 0, 0, 0};

        // compute the absolute (positive) distance from object1 to object2
        diff.x = abs(object1.x - object2.x);
        diff.y = abs(object1.y - object2.y);
        diff.z = abs(object1.z - object2.z);
        diff.rad = object1.rad + object2.rad;

        // If the distance between each of the three dimensions is within the radii combined, there is a collision
        if(diff.x < diff.rad && diff.y < diff.rad && diff.z < diff.rad)
        {
                collision = true;
        }

        return collision;
}

bool checkBoxCollision(objectBox object1, objectBox object2)
{
        bool collision = false;
        objectBox diff = {0, 0, 0, 0};

        // compute the absolute (positive) distance from object1 to object2
        diff.x = abs(object1.x - object2.x);
        diff.y = abs(object1.y - object2.y);
        diff.z = abs(object1.z - object2.z);
        diff.radx = object1.radx + object2.radx;
        diff.rady = object1.rady + object2.rady;
        diff.radz = object1.radz + object2.radz;

        // If the distance between each of the three dimensions is within the radii combined, there is a collision
        if(diff.x < diff.radx && diff.y < diff.rady && diff.z < diff.radz)
        {
                collision = true;
        }

        return collision;
}

bool checkSphereBoxCollision(object object1, objectBox object2)
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
}

void checkHeliCollisions(void)
{
        bool collision = false;

        if ( checkSphereBoxCollision(heli, building0) )
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
float cosDeg(float heliRot)
{
        float radRot = heliRot * pi/180;
        return (cos(radRot));
}

// Converts degrees(in) to radians and returns the sine (out)
float sinDeg(float heliRot)
{
        float radRot = heliRot * pi/180;
        return (sin(radRot));
}

// Catches keyboard key presses
void keyboard(unsigned char key, int mouseX, int mouseY)
{
		if(pause)
		{
			pause = false;
		}

        switch (key)
        {
                // If ESC key is pressed, exit
                case 27:
                        glutLeaveMainLoop();
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
					if (rotorSpeed < MAX_ROTOR_SPEED)
						rotorSpeed = rotorSpeed + 0.1;
					else helicopterOn = true;
						break;
				case 'x':
						//Stop Blades
						if (rotorSpeed > 0)
							rotorSpeed = rotorSpeed - 0.1;
						if (rotorSpeed < MAX_ROTOR_SPEED)
							helicopterOn = false;
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
                        break;
                case GLUT_KEY_PAGE_UP:
                        // Zoom in
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
        if (rightMouseDown)
        {
                float rotate = ( (float)x - (float)last_mouse_x) / (float)windowWidth * 360.0;
                eye.rot += rotate;
                last_mouse_x = x;
        }
}

void mouse(int button, int state, int x, int y)
{
        last_mouse_x = x;

        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
                rightMouseDown = true;
        }
        else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
        {
                eye.rot = 135;
        }
        else
        {
                rightMouseDown = false;
        }
}

void moveHeliForward(float speed, bool checkCol)
{
        // Move heli
        heli.x += speed * cosDeg(heli.rot);
        heli.z -= speed * sinDeg(heli.rot);

        // Move camera
        eye.x += speed * cosDeg(heli.rot);
        eye.z -= speed * sinDeg(heli.rot);

        checkHeliCollisions();
}

void moveHeliBack(float speed, bool checkCol)
{
        // Move heli
        heli.x -= speed * cosDeg(heli.rot);
        heli.z += speed * sinDeg(heli.rot);

        // Move camera
        eye.x -= speed * cosDeg(heli.rot);
        eye.z += speed * sinDeg(heli.rot);

        checkHeliCollisions();
}

void moveHeliUp(float speed, bool checkCol)
{
        heli.y += heliSpeed;
        eye.y += heliSpeed;

        checkHeliCollisions();
}

void moveHeliDown(float speed, bool checkCol)
{
        heli.y -= heliSpeed;
        eye.y -= heliSpeed;

        checkHeliCollisions();
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
		// Display game time
		renderBitmapString(textX, textY + 25, (void *)font, strGameTime);

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

	glColor3f(1.0, 0.7, 0.2);

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
	renderBitmapString(HELP_SPACE, 425, (void *)font, "\tfor Cp2060 Assignment 2");

	// Display a backing screen so that the text is readable
	const float MARGIN = 5.0;
	glColor4f(0.5, 0.5, 0.5, 0.5);	// Grey
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
	if(!pause)
	{
		if (helicopterOn == true)
		{
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
				heli.rot += rotSpeed;
				// Adjust the rotor spin to counter heli spin
				rotor += rotorSpeed - rotSpeed;
				heliLeanSide = -LEAN_FACTOR;
			}
			else if (turningRight)
			{
				// turn right
				heli.rot -= rotSpeed;
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
				if(heli.y > groundHeight + heli.rad/2.0)
				{
					moveHeliDown(heliSpeed, true);
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
        glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
        glMatrixMode(GL_MODELVIEW);
}


// Display the scene and it's components
void display(void)
{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        gluLookAt(eye.x, eye.y, eye.z, heli.x, heli.y, heli.z, 0.0, 1.0, 0.0);

        // Rotate camera so that it is always behind the heli
        glPushMatrix();
        glTranslatef(heli.x, heli.y, heli.z);
        glRotatef(-heli.rot + eye.rot, 0.0, 1.0, 0.0);
        glTranslatef(-heli.x, -heli.y, -heli.z);

        // Draw the helicopter
        glPushMatrix();
        // Go to the heli position before drawing the heli
        glTranslatef(heli.x, heli.y, heli.z);
        glRotatef(heli.rot, 0.0, 1.0, 0.0);
        glRotatef(heliLeanFront, 0.0, 0.0, 1.0);
        glRotatef(heliLeanSide, 1.0, 0.0, 0.0);
        // Draw body
        glCallList(heliBodyList);
        // Animate rotor
        glTranslatef(0.6, 0.0, 0.0);
        glRotatef(rotor, 0.0, 1.0, 0.0);
        // Draw rotor
        glCallList(heliRotorList);
        glPopMatrix();

        // Draw ground
        glPushMatrix();
        glTranslatef(0, groundHeight, 0);
        glCallList(groundList);
        glPopMatrix();

        // Draw building
        glPushMatrix();
        drawBuilding();
        glPopMatrix();

		updateFPS();
		updateGameTime();
		displayText();

        glPopMatrix();

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
	case 3: pause = !pause; // Help/Credits/Scores
		break;
	case 4: exit(0);  // exit program
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

		// create main menu (no ID needed)
		glutCreateMenu(mymenu);
		glutAddMenuEntry("Full Shading", 1);
		glutAddMenuEntry("Wire Frame", 2);
		glutAddMenuEntry("Help/Credits/Scores", 3);
		glutAddMenuEntry("Exit Program", 4);
		// set the menu to the right button
		glutAttachMenu(GLUT_RIGHT_BUTTON);  

        glutMainLoop();
        return 0;
}
