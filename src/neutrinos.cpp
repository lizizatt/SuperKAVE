//********************************************************
// Syzygy is licensed under the BSD license v2
// see the file SZG_CREDITS for details
//********************************************************

//  -Liz Izatt
// liz@berkeley.edu

// precompiled header include MUST appear as the first non-comment line
#include "arPrecompiled.h"

//#define ICECUBE					//Ross 6/11/2013 - comment this out to run Duke's Super-KAVE simulation, keep in to run IceCube simulation

#ifdef ICECUBE
#include "icecube.h"
#endif

// MUST come before other szg includes. See arCallingConventions.h for details.
#define SZG_DO_NOT_EXPORT
#include <stdio.h>
#include <stdlib.h>
#include "arMasterSlaveFramework.h"
#include "arInteractableThing.h"
#include "arInteractionUtilities.h"
#include "arGlut.h"
#include "arGraphicsHeader.h" //include gl.h

#ifdef WIN32            //used to be WINNEUTRINO. DJZ 10/10/2013
#include <time.h>
#endif



// The class containing all the relevant information about each dot
class dot {
public:
	float cx, cy, cz, charge, time, radius;
	arVector3 angle;
	dot(float x, float y, float z, arVector3 theta, float q, float t, float r) {
		cx = x * 3.28;
		cy = y * 3.28;
		cz = z * 3.28;
		angle = theta;
		charge = q;
		time = t;
		radius = r;
	};
	dot(){};
};

typedef struct ringPointHolder{  //just a wrapped vector of arVector3s
	vector<arVector3> ringPoints;
}ringPointHolder;

class dotVector {  //a dotVector is an individual event.
public:
	double startTime;
	double endTime;
	double minTime, maxTime;
	double length;
	vector<double> particleType;  //each particleType as a double, which is encoded as GEANT particle code (http://hepunx.rl.ac.uk/BFROOT/www/Computing/Environment/NewUser/htmlbug/node51.html)
	vector<string> particleName;  //just for ease of access, if we know what the type is (eg, muon electron pion) we'll write it in here, else we'll just enter "???"
	vector<double> coneAngle;
	vector<arVector3> coneDirection;
	double vertexPosition[3];
	vector<bool> haveRingPoints;
	vector<bool> doDisplay;
	vector<vector<ringPointHolder> > ringPoints;
	vector<float> momentum;  //momentum (in MeV ? )
	vector<float> energy; //in the case of time-compression (supernova file), each consecutive 3 entries in this will be vertex positions ... else, energy in MeV
	vector<dot> dots;  //holds the inner cylinder
	vector<dot> outerDots; //holds the outer cylinder
	dotVector(vector<dot> in, vector<dot> outer, float st) {
		dots = in;
		outerDots = outer;
		startTime = st;
	};
	dotVector(){};
};

class textItem{
public:
	arVector3 myColor;
	string myString;
	textItem(string ins, arVector3 ins2){
		myString = ins;
		myColor = ins2;
	}
	textItem(string ins){
		myString = ins;
		myColor = arVector3(1,1,1);
	}
};

class dataDisplay{ 
public:
	bool isVisible;
	static int beingGrabbed;  //if -1, nothing is being grabbed.
	static int numberDisplays;
	int myIndex;  //given on initialization
	arVector3 offset;  //used for movement.
	arMatrix4 myOriginalRotationMatrix;
	arVector3 myPos;  //location of very center
	arMatrix4 myRotationMatrix;  //location outward of screen
	vector<textItem> contents;  //contents, to be drawn from top to bottom, line by line
	void drawDataDisplay(arMasterSlaveFramework& fw);
	dataDisplay(); //used to be dataDisplay::dataDisplay(); -DJZ 10/10/2013
};

dataDisplay::dataDisplay(){
	myIndex = numberDisplays;
	numberDisplays++;
}

int dataDisplay::beingGrabbed = -1;
int dataDisplay::numberDisplays = 0;



class menuItem{  //knows how to draw itself, and what to do if its clicked.
public:
	menuItem(){  //used to be menuItem::menuItem() -DJZ 10/10/2013
	}
	char * content[10];
	int index;
	int numLines;
	int startOffSetX;
	int startOffSetY;
	float scale;
	void drawSelf( bool highlighted);
	void (*onClick)();
};



class menu{
public:
	int id;  //for identification purposes
	menu * parent;
	vector<menu> children;
	vector<menuItem> myItems;  //all of my items, each knowing how to draw itself given an index (center is 0, numbers to sides are offsets)
	menu(){ //used menu::menu() -DJZ 10/10/2013
		static int numMenus = 0;
		id = numMenus;
		numMenus++;
	}
	void drawSelf(arMasterSlaveFramework& fw);  
};



//Global variables
bool debug = false;  //enables some useful print statements
//menu variables

vector<string> logFile;

double lastButtonPress;
double buttonPressThreshold = .25;
bool b0;
bool b1;
bool b2;
bool b3;
bool b4;
bool b5;
bool b5Reg;
menu currentMenu;
menu nextMenu;
menu mainMenu;
menu optionsMenu;
menu visMenu;
menu displaysMenu;
menu controlsMenu;
menu conesMenu;
bool doHUD = false;  //outdated
bool doMenu = false;  //whether or not the menu is currently displayed
bool doMainMenu = true; 
bool doOptionsMenu = false;
bool doLoadMenu = false;
bool doCherenkovConeMenu = false;
bool triggerDepressed = false;
int cherenkovConeMenuIndex = 0;  //there will be (num charenkov cones) / 3 submenus if the number of cherenkov cones is greated than 4
int menuIndex = 0;  //current index in the menu, defaults to 0 .. can be -2,-1,0,1,2 for 5 windows
bool doColorKey = false;  //window next to the primary tablet with information for charge or time display ... not currently implemented
int modifiedCherenkovConeIndex = -1;  //if we modify a cone index, instead of sharing the entire doDisplay vector, we'll change this to a value 0 - doDisplay.size()-1.  If it's -1, no change

bool doCherenkovCone = true;   //toggle for cherenkov cones lines connecting particle to projection on wall.
bool doScaleByCharge = true;   //scales the radii of circles by their respective charges.  Hard coded scale factor at the moment.
double cherenkovElectronThreshold = .768; // in MeV
double electronMass = 9.11e-31; //kilos  //ID -11 (11 = positron)
double cherenkovMuonThreshold = 158.7; //MeV
double muonMass = 1.88352473e-28; //kilos  //ID 13 (antiparticle = 13)
double cherenkovPionThreshold = 209.7;  //MeV
double pionMass = 2.483e-28;  //kilos  //ID is 211 / -211
double speedOfLight = 299792458.; //in m/s
double currentTime = 0.0;
double timeScaleFactor = .5;  //scale factor for playback.  At 1, length of the supernova is 7 seconds.  There's a bug or me being stupid somewhere that's requiring this to not be 1.0.
double timeHolder1 = 0.0;
bool doCylinderDivider = true;  //do outer detector true/false
int autoPlay = 0;   //autoplay back = -1, autoplay forward = 1;
float lastJoyStickMove = 0;  //last time the joystick was moved, used for double-press activation, not currently working
bool joyStickMoveDir = true;  //last joystick move direction, true = right, false = left, not implemented
float ax, az;
GLUquadricObj * quadObj;        //cylinder quadratic object.
/*
static float RADIUS = 17;     //Constants for sizing IN METERS
static float HEIGHT = 40;
static float OUTERRADIUS = 17.61;  
static float OUTERHEIGHT = 41.22;
static float PI = 3.141592;
static float innerDotRad = 0.3;
static float outerDotRad = 0.2;
*/
static float RADIUS = 17 * 3.28;     //Constants for sizing IN FEED
static float HEIGHT = 40 * 3.28;
static float OUTERRADIUS = 17.61 * 3.28;  
static float OUTERHEIGHT = 41.22*  3.28;
static float PI = 3.141592;
static float innerDotRad = 0.3 * 3.28;
static float outerDotRad = 0.2 * 3.28;
static double threshold = 1.;
int index;  //current location in the event list
int indexTransfer;  
vector<dotVector> dotVectors;     //Vector to hold all generated dot vectors in loaded order
dotVector currentDots;               //Class to hold unknown number of dots (just wraps the dotVector)
arVector3 currentPosition;
float viewer_distance=100.0;
float viewer_angle=0.0;
bool l_button=false;
bool r_button=false;
arVector3 l_position;
dot tempDot;                    //dot held between loading new events. This is a one-dot buffer
bool stored = false;            //Telling us whether a dot is stored in tempDot
bool colorByCharge = false;      //whether to color by charge or time
bool doTimePlayback = true;
double eventBegan = 0;
double elapsedTime = 0;
ifstream dataFile;              //data input
char* filename;
bool doTimeCompressed = false;
arVector3 deltaPosition, originalPosition;
arVector3 deltaDirection, originalDirection;
bool isTouchingVertex = false;
bool isGrabbingVertex = false;
int itemTouching = 0;  //0 corresponds to vertex, else subtract 1 and is index of cone
arVector3 vertexOffset;
bool doDataDisplay = true;
vector<dataDisplay> myDataDisplays;
bool doWristBoundTime = false;
double wristRotation = 0;  //wrist rotation, in degrees
double menuDist = 3;
arVector3 menuPosition; //offset from rotation matrix
arMatrix4 menuRotationMatrix;
bool grabbingDisplay = false;
//string bufferLine;  
vector<char*> needsReleasing;

bool displayRanThisRound = false;


//debug helper function
void debugText(string s){
	if(debug){
		cout << s;
		cout << "\n";
	}
}


//helper functions written for arVector3s

arVector3 subtract(arVector3 a, arVector3 b){
		return arVector3( a[0]-b[0], a[1]-b[1], a[2]-b[2]);
}

arVector3 add(arVector3 a, arVector3 b){
	return arVector3(a[0]+b[0], a[1]+b[1], a[2]+b[2]);
}

arMatrix4 add(arMatrix4 a, arMatrix4 b){
	arMatrix4 out;
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			out[i*4+j] = a[i*4+j] + b[i*4+j];
		}
	}
}

arMatrix4 subtract(arMatrix4 a, arMatrix4 b){
	arMatrix4 out;
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			out[i*4+j] = a[i*4+j] - b[i*4+j];
		}
	}
	return out;
}

#ifdef WINNEUTRINO
float magnitude(arVector3& a){
	return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}
#else
float magnitude(arVector3 a){
	return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}
#endif

double dotProduct(arVector3 a, arVector3 b){
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

arVector3 crossProduct(arVector3 a, arVector3 b){
	return arVector3 ( (a[1]*b[2]-a[2]*b[1]) , ( a[2]*b[0]-a[0]*b[2]),  (a[0]*b[1]-a[1]*b[0]));
}

arVector3 normalize(arVector3 a){
	float mag = sqrt(pow(a[0],2)+pow(a[1],2)+pow(a[2],2));
	return arVector3(a[0]/mag,a[1]/mag,a[2]/mag);
}

//absolute value function to deal with a stupid compilation issue (b/c there's an abs(double) function in more than one of our imported libraries?)
#ifndef WINNEUTRINO
double abs(double in){
	if(in > 0.0){
		return in;
	}
	return in * -1.0;
}
#endif

void dataDisplay::drawDataDisplay(arMasterSlaveFramework& fw){  //updates position (if necessary) and draws
	//figure out if we're touching the displa
	//figure out which item we're grabbing ... go for closest one

	bool isTouching = false;
	bool isInRange = true;

	arMatrix4 rotationMatrixHolder = ar_getNavMatrix() * fw.getMatrix(1);
	//this is exact hand position / orientation
	double dirX = rotationMatrixHolder[8];
	double dirY = rotationMatrixHolder[9];
	double dirZ = rotationMatrixHolder[10];
	rotationMatrixHolder[12] -= dirX;
	rotationMatrixHolder[13] -= dirY;
	rotationMatrixHolder[14] -= dirZ;
	arMatrix4 nextDisplayRotMat = rotationMatrixHolder;
	nextDisplayRotMat[12] -= 3*dirX;
	nextDisplayRotMat[13] -= 3*dirY;
	nextDisplayRotMat[14] -= 3*dirZ;

	arVector3 handPos = arVector3(rotationMatrixHolder[12], rotationMatrixHolder[13], rotationMatrixHolder[14]);
	arVector3 displayCenter = arVector3(myRotationMatrix[12], myRotationMatrix[13], myRotationMatrix[14]);
	arVector3 displayX = arVector3(myRotationMatrix[0], myRotationMatrix[1], myRotationMatrix[2]);
	arVector3 displayY = arVector3(myRotationMatrix[4], myRotationMatrix[5], myRotationMatrix[6]);
	arVector3 displayZ = arVector3(myRotationMatrix[8], myRotationMatrix[9], myRotationMatrix[10]);

	isInRange = dotProduct( dotProduct((handPos - (displayCenter + displayX * 1.5)), displayX) * displayX, dotProduct((handPos - (displayCenter - displayX * 1.5)), displayX) * displayX) < 0; 
	isInRange = isInRange && dotProduct( dotProduct((handPos - (displayCenter + displayY * 2.5)), displayY) * displayY, dotProduct((handPos - (displayCenter - displayY * 2.5)), displayY) * displayY) < 0; 
	isInRange = isInRange && dotProduct( dotProduct((handPos - (displayCenter + displayZ * 3.2)), displayZ) * displayZ, dotProduct((handPos - (displayCenter - displayZ * .5)), displayZ) * displayZ) < 0; 
	//isInRange = magnitude(subtract(handPos, arVector3(myRotationMatrix[12], myRotationMatrix[13], myRotationMatrix[14]))) < 5;


	if( !isVisible || (isInRange && (beingGrabbed == -1 || beingGrabbed == myIndex))){  //are we in range and are we not already grabbing one?  grab it
		isTouching = true;
		/*
		if(fw.getOnButton(5)){ //we just started grabbing, set offset (currentPos - handPos)
			offset = subtract(myPos, handPos);
			myOriginalRotationMatrix = myRotationMatrix;
		}
		*/
		if(fw.getButton(5)){ //is grabbing
			if(isVisible)
				beingGrabbed = myIndex;
			//update location based on hand movement
			//myPos = add(handPos, offset);
			if(isVisible)
				grabbingDisplay = true;
			myRotationMatrix = nextDisplayRotMat;
		} else { //not grabbing anymore
			beingGrabbed = -1;
		}	
		if(!isVisible){  //if invisible, parent onto spawn location and then stop before we render it
			//	myPos = add(handPos, 5*myIndex*arVector3(handRightX, handRightY, handRightZ));
			double sideX = rotationMatrixHolder[0];
			double sideY = rotationMatrixHolder[1];
			double sideZ = rotationMatrixHolder[2];
			nextDisplayRotMat[12] += 4*sideX;
			nextDisplayRotMat[13] += 4*sideY;
			nextDisplayRotMat[14] += 4*sideZ;
			myRotationMatrix = nextDisplayRotMat;
			return;
		}
	} else{
		isTouching = false;
	}

	glPushMatrix();


		glTranslatef(myPos[0], myPos[1], myPos[2]);

		//glRotatef(90,-1,0,0);
		glMultMatrixf(myRotationMatrix.v);

		glScalef(3,5,.5);
		glColor3f(.75,.75,.75);
		glutSolidCube(1.0);
		glColor3f(0,0,0);
		glutWireCube(1.01);
		glScalef(.9,.9,1.2);
		glutSolidCube(1.0);

		if(isTouching){
			glPushMatrix();
				glColor4f(0,1,0,.5);
				if(fw.getButton(5)){
					glScalef(1.3,1.3,.5);
				}else{
					glScalef(1.2,1.2,.5);
				}
				glutSolidCube(1);
				glColor3f(0,0,0);
				glutWireCube(1.001);
			glPopMatrix();
		}	

		glTranslatef(-.4,.4,.52);
		//now at upper left

		glColor3f(1,1,1);
		//write text, line by line
		glLineWidth(3.0);
		for(int i = 0; i < contents.size(); i++){
			string myString = contents[i].myString;
			glColor3f(contents[i].myColor[0], contents[i].myColor[1], contents[i].myColor[2]);
			glPushMatrix();
			glScalef(.0004,.0004,.0004);  
			for(int j = 0; j < myString.length(); j++){
				char myChar = myString.at(j);
				glutStrokeCharacter(GLUT_STROKE_ROMAN, myChar);
			}
			glPopMatrix();
			glTranslatef(0,-.05,0);
		}
	glPopMatrix();
}

int red_values [] =		{132	,74,	22,		4,		4,		6,		40,		115,	193,	249,	253,	249,	219,	219,	219,	219};  //r
int green_values [] =	{4		,12,	4,		124,	175,	184,	183,	114,	193,	249,	213,	191,	143,	129,	102,	33};    //g
int blue_values [] =	{186	,178,	186,	186,	186,	86,		7,		0,		6,		21,		80,		1,		12,		12,		12,		12};   //b

//returns color of dots based on whether charge or time are the metrics for coloring.
arVector3 dotColor(dot d) {
	float red, green, blue;
	float numDivs = 16;
	int section = 0;
	if(colorByCharge) {
		float value = d.charge;
		if(value > 26.7) section = 15;
		else if (value > 23.3) section = 14;
		else if (value > 20.2) section = 13;
		else if (value > 17.3) section = 12;
		else if (value > 14.7) section = 11;
		else if (value > 12.2) section = 10;
		else if (value > 10) section = 9;
		else if (value > 8.0) section = 8;
		else if (value > 6.2) section = 7;
		else if (value > 4.7) section = 6;
		else if (value > 3.3) section = 5;
		else if (value > 2.2) section = 4;
		else if (value > 1.3) section = 3;
		else if (value > 0.7) section = 2;
		else if (value > 0.2) section = 1;

		red = red_values[section] / 255.0;
		blue = blue_values[section] / 255.0;
		green = green_values[section] / 255.0;
	} else {
		float value = d.time;
		if(value < 893) section = 15;
		else if (value < 909) section = 14;
		else if (value < 925) section = 13;
		else if (value < 941) section = 12;
		else if (value < 957) section = 11;
		else if (value < 973) section = 10;
		else if (value < 989) section = 9;
		else if (value < 1005) section = 8;
		else if (value < 1021) section = 7;
		else if (value < 1037) section = 6;
		else if (value < 1053) section = 5;
		else if (value < 1069) section = 4;
		else if (value < 1085) section = 3;
		else if (value < 1101) section = 2;
		else if (value < 1117) section = 1;

		//now we will access an array of pre-picked values corresponding with the superscan mode, based on the SECTION
		red = red_values[section] / 255.0;
		blue = blue_values[section] / 255.0;
		green = green_values[section] / 255.0;
	}
	return arVector3(red,green,blue);
}

//logic for drawing a circle on the edge of the cylinder.
void drawCircle(dot d, bool doOuterCircle) {  //outerCircle is for outer detector, adds the square around the dot
	//if(d.time > elapsedTime*100 && doTimePlayback){
	//	return;
	//}
	
	if(doWristBoundTime){  //use wrist rotation value to figure out whether or not to draw this dot
		//-70 = 0, 70 = maxTime
		double wristRotationLocal = (wristRotation + 90) / 180.0;  //now scaled from 0 to 1
		double cTime = (d.time-currentDots.minTime) / currentDots.maxTime;  //now scaled from 0-1
		if(cTime > wristRotationLocal)
			return;
	}
	
	bool top = (d.angle[0]==0 && d.angle[1]==0) ? true : false;
	float cx = d.cx;
	float cy = d.cy;
	float cz = d.cz;
	float j, newx, newy, newz;
	arVector3 color = dotColor(d);
	glColor3f(color[0], color[1], color[2]);
	//    glBegin(GL_LINES);

	float radius = d.radius;
	float radiusScaleFactor = 1.;
	if(doScaleByCharge && abs(d.charge) < 26.7){
		float scaleMin = .5;
		if(doTimeCompressed){
			scaleMin = .25;
		}
		radiusScaleFactor = scaleMin/26.7 * d.charge + scaleMin;
		radius = radius * radiusScaleFactor;
	}
	int subdivisions = 25;
	if(doTimeCompressed && autoPlay != 0){
		subdivisions = 4;
	}
	if(top)
	{
		glPushMatrix();
		glTranslatef(cx,cy,cz);
		if(doOuterCircle){
			glBegin(GL_QUADS);
			//outside border
			glVertex3f(-.6f,.6f,0);
			glVertex3f(-.6f,-.6f,0);
			glVertex3f(-.6+.2*radiusScaleFactor,-.6+.2*radiusScaleFactor,0);
			glVertex3f(-.6+.2*radiusScaleFactor,.6-.2*radiusScaleFactor,0);

			glVertex3f(-.6f,-.6f,0);
			glVertex3f(.6f,-.6f,0);
			glVertex3f(.6-.2*radiusScaleFactor,-.6+.2*radiusScaleFactor,0);
			glVertex3f(-.6+.2*radiusScaleFactor,-.6+.2*radiusScaleFactor,0);

			glVertex3f(.6f,-.6f,0);
			glVertex3f(.6f,.6f,0);
			glVertex3f(.6-.2*radiusScaleFactor,.6-.2*radiusScaleFactor,0);
			glVertex3f(.6-.2*radiusScaleFactor,-.6+.2*radiusScaleFactor,0);

			glVertex3f(.6f,.6f,0);
			glVertex3f(-.6f,.6f,0);
			glVertex3f(-.6+.2*radiusScaleFactor,.6-.2*radiusScaleFactor,0);
			glVertex3f(.6-.2*radiusScaleFactor,.6-.2*radiusScaleFactor,0);
			glEnd();
		}
		gluDisk(quadObj,0,radius,subdivisions,1);
		glPopMatrix();
	}
	else {
		float theta = atan2(cy,cx);
		glPushMatrix();
		glTranslatef(cx,cy,cz);
		glRotatef(90,0,1,0);
		glRotatef(-theta*180/PI,1,0,0);
		gluDisk(quadObj,0,radius,subdivisions,1);
		if(doOuterCircle){
			glBegin(GL_QUADS);
			//outside border
			glVertex3f(-.6f,.6f,0);
			glVertex3f(-.6f,-.6f,0);
			glVertex3f(-.6+.2*radiusScaleFactor,-.6+.2*radiusScaleFactor,0);
			glVertex3f(-.6+.2*radiusScaleFactor,.6-.2*radiusScaleFactor,0);
			//glVertex3f(-.4f,-.4f,0);
			//glVertex3f(-.4f,.4f,0);

			glVertex3f(-.6f,-.6f,0);
			glVertex3f(.6f,-.6f,0);
			glVertex3f(.6-.2*radiusScaleFactor,-.6+.2*radiusScaleFactor,0);
			glVertex3f(-.6+.2*radiusScaleFactor,-.6+.2*radiusScaleFactor,0);
			//glVertex3f(.4f,-.4f,0);
			//glVertex3f(-.4f,-.4f,0);

			glVertex3f(.6f,-.6f,0);
			glVertex3f(.6f,.6f,0);
			glVertex3f(.6-.2*radiusScaleFactor,.6-.2*radiusScaleFactor,0);
			glVertex3f(.6-.2*radiusScaleFactor,-.6+.2*radiusScaleFactor,0);
			//glVertex3f(.4f,.4f,0);
			//glVertex3f(.4f,-.4f,0);

			glVertex3f(.6f,.6f,0);
			glVertex3f(-.6f,.6f,0);
			glVertex3f(-.6+.2*radiusScaleFactor,.6-.2*radiusScaleFactor,0);
			glVertex3f(.6-.2*radiusScaleFactor,.6-.2*radiusScaleFactor,0);
			//glVertex3f(-.4f,.4f,0);
			//glVertex3f(.4f,.4f,0);
			glEnd();
		}
		glPopMatrix();

	}
	//    glEnd();
}

//loop through vector of dots and draws them all.
void drawDots(void) {
	debugText("Began Draw Dots");
	for(std::vector<dot>::size_type i = 0; i != currentDots.dots.size(); i++) {
		drawCircle(currentDots.dots[i],false);  //does not draw the outer detector component (the square around it)
	}
	if(doCylinderDivider){
		for(std::vector<dot>::size_type i = 0; i != currentDots.outerDots.size();i++){
			drawCircle(currentDots.outerDots[i],true); //draws the outer detector component
		}
	}
	debugText("Ended draw dots");
}

arVector3 intersectCylinder(arVector3 rayOrigin, arVector3 ray, float radius, float height){  //assumes cylinder aligned along z-axis and centered on 0,0,0, returns a point of intersection
	/*
	RAYCASTING ALGORITHM
	(vx+dx*t)^2 + (vy+dy*t)^2 = r^2
	vx^2 + 2*vx*dx*t + dx^2*t^2 + vy^2 + 2*vy*dy*t + dy^2*t^2 = r^2
	*****	t^2 * (dx^2 + dy^2) + t * (2*vx*dx+2*vy*dy) + (vx^2 + vy^2 - r^2) = 0 ****
	solve for t
	first positive t will be the intersection we want (since we're starting inside the cylinder in all cases)
	plug in t to (vx+dx*t) for x coord, (vy+dx*t) for y coord, t*dz will give delta z to generate z coord
	if z > h or z < 0, then we hit a disk, backtrack
	solve (vz + dz*t) = h or (vz+dz*t) = 0
	thus t = ([h/0]-vz)/dz
	plug in t for x and y coords
	fin
	*/
	
	float a = pow(ray[0],2)+pow(ray[1],2);
	float b = 2*rayOrigin[0]*ray[0] + 2*rayOrigin[1]*ray[1];
	float c = pow(rayOrigin[0],2) + pow(rayOrigin[1],2) - pow(radius,2);
	float radical = pow(b,2) - 4 * a * c;
	arVector3 point;
	if(radical < 0){
		return point;  //we should never get here, this is the case where there is no intersection
	}
	float t = (-b + sqrt(radical))/(2*a);  //quadratic formula
	/*
	if(t <= 0){								// we should get out a positive and negative t for each case since we're doing an intersection from inside the cylinder									//
	t = (-b + sqrt(radical))/(2*a);    //  so we can assume it's the "+" version of quadratic formula
	}
	*/
	//here we should have a value of t
	//so extrapolate that and find our point on hte cylinder
	point = t*ray;  //this is relative to the vertexPosition

	rayOrigin[2] = rayOrigin[2]-height/2.;  //here, vertex position is stored ranging from 0 - 40 (eg, bottom of the cylinder is at 0), whereas the stored geometry at this point puts the bottom at -20 and top at 20.  Changing it before would mess up calculations, so change it here (and only temporarily)
	arVector3 finalPoint = point + rayOrigin;  //this is the actual point of contact with the cylinder

	if(finalPoint[2] > height/2.){ //if we hit the top
		t = ((height/2.) - rayOrigin[2]) / abs(ray[2]);	
		point = t * ray;
	}
	if(finalPoint[2] < (-height/2.)){ //we hit the bottom
		t = (rayOrigin[2]+(height/2.)) / abs(ray[2]);
		point = t * ray;
	}
	return point;
}

//method to draw everything in the scene
void drawScene(arMasterSlaveFramework& framework)
{
	debugText("began draw scene");
	glPushMatrix();
	//    glScalef(5.0,5.0,5.0);  //use this line to scale the environment
	gluQuadricDrawStyle(quadObj, GLU_LINE);  //sets draw style of the cylinder to wireframe (GLU_LINE)
	glColor3f(1.0, 1.0, .8);
	glTranslatef(0,20,0);
	glRotatef(90, 1.0, 0.0, 0.0);  //rotate to set cylinder up/down

	//inner cyliner:
	gluCylinder(quadObj, RADIUS, RADIUS, HEIGHT, 50, 30); //inner cylinder wireframe

	//wireframe caps:
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_LINES);
	for(int i = 0; i < 2; i++){
		int num_verticies = 50;
		for(double j = PI/num_verticies; j < PI; j+=(2*PI/num_verticies)){
			glVertex3f(RADIUS*cos(j),RADIUS*sin(j),i*HEIGHT-0.1);
			glVertex3f(RADIUS*cos(j),-RADIUS*sin(j),i*HEIGHT-0.1);
		}
	}
	for(int i = 0; i < 2; i++){
		int num_verticies = 50;
		for(double j = PI/num_verticies; j < 2*PI; j+=(2*PI/num_verticies)){
			glVertex3f(RADIUS*cos(j),RADIUS*sin(j),i*HEIGHT-0.1);
			glVertex3f(-RADIUS*cos(j),RADIUS*sin(j),i*HEIGHT-0.1);
		} 
	}
	glEnd();

	//draw the dots
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	drawDots();

	
	//transform to "grabber" position
	/*
	glPushMatrix();
		arMatrix4 myPosMatrix( ar_getNavInvMatrix() );
		double navX = myPosMatrix[12];
		double navY = myPosMatrix[13];
		double navZ = myPosMatrix[14];
		navY += 20;
		double holder = navY;
		navY = navZ;
		navZ = holder;
		//now navx, navy, navz are in coordinates of cylinder ... that being, y and z are flipped
		
		arMatrix4 myHandMatrix = framework.getMatrix(1);
		double handPosX = myHandMatrix[12];  //this is hand position
		double handPosY = myHandMatrix[13];
		double handPosZ = myHandMatrix[14];
		
		double handDirX = myHandMatrix[8];
		double handDirY = myHandMatrix[9];
		double handDirZ = myHandMatrix[10];
		
		glTranslatef(-navX, -navY, navZ);
		glTranslatef(handPosX, handPosZ, -handPosY);
		glTranslatef(-2*handDirX, -2*handDirZ, 2*handDirY);
		glColor3f(1,1,1);
		glutSolidSphere(.05,50,50);
	glPopMatrix();
	*/
	
	
	//do cherenkov cones
	if(!doTimeCompressed){  //only do the cherenkov cones themselves if we're not doing time compressed mode (would be too costly)
		glPushMatrix(); //place particle and cherenkov cone
			//enable transparency and lighting for a less flat / awkward looking particle and cone that won't be too obnoxious
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
			if (isGrabbingVertex && itemTouching == 0)
				glColor4f(.1f, .9f, .1f, 1.f);
			else if(isTouchingVertex && itemTouching == 0)
				glColor4f(.1f, .7f, .1f, 1.f);
			else
				glColor4f(1.f,1.f,.69f,1.f); 
			glTranslatef(currentDots.vertexPosition[0],currentDots.vertexPosition[1],currentDots.vertexPosition[2]);
			glutSolidSphere(.25,50,50);  //this is the particle itself
			glColor4f(1.f,1.f,.69f,1.f); 
			
/*
			//add a wireframe if hand is "touching" particle
			if(isTouchingVertex){
				glutWireCube(1);
			} //*/
			
			//place cones! 
			for(int i = 0; i < currentDots.particleType.size();i++){  //for each final state particle we have stored / will generate cone data
				if(!(currentDots.doDisplay[i])){
					continue;
				}
				
				//add line in direction of cone
				glPushMatrix();
					if (isGrabbingVertex && itemTouching == i+1)
						glColor4f(.1f, .9f, .1f, 1.f);
					else if(isTouchingVertex && itemTouching == i+1)
						glColor4f(.1f, .7f, .1f, 1.f);
					else
						glColor4f(1.f,1.f,.69f,1.f); 
					glLineWidth(3.0);
					glBegin(GL_LINES);
						glVertex3f(0,0,0);
						glVertex3f(currentDots.coneDirection[i][0], currentDots.coneDirection[i][1], currentDots.coneDirection[i][2]);
					glEnd();
					glTranslatef(currentDots.coneDirection[i][0], currentDots.coneDirection[i][1], currentDots.coneDirection[i][2]);
					arVector3 direction = normalize(currentDots.coneDirection[i]);
					arVector3 currentRotation(0,0,1);
					arVector3 perpAxis = crossProduct(currentRotation, direction);
					float dotResult = dotProduct(currentRotation, direction);
					float angle = acos(dotResult);
					glRotatef(angle*180./PI, perpAxis[0], perpAxis[1], perpAxis[2]);
					glTranslatef(0,0,-.5);
					gluCylinder(quadObj, .2, 0, .5, 50, 30); //inner cylinder wireframe
				glPopMatrix();
				
				glColor4f(1.,1.,.69,.4);

				//raycasted cone
				float slices = 270.;
				float dist = 10;

				glEnable (GL_BLEND); 
				glLineWidth(3.0);
				//draw ring points, if they are memorized already
				if(currentDots.haveRingPoints[i]){//this trades memory for processing...we only generate the points on the first frame of an event, then save them.
					//and, if we come back to the event, we still don't need to redo it!  unless we tell it to by changing haveRingPoints to false
					for(int c = 0; c < 2; c++){
						glPushMatrix();
						if(c == 1)
							glTranslatef(currentDots.coneDirection[i][0], currentDots.coneDirection[i][1], currentDots.coneDirection[i][2]);
						for(int k = 0; k < currentDots.ringPoints[i][c].ringPoints.size();k++){
							arVector3 currentPoint = currentDots.ringPoints[i][c].ringPoints[k];
							if(doCherenkovCone){
								glBegin(GL_LINES);
								glVertex3f(0,0,0);
								glVertex3f(currentPoint[0],currentPoint[1],currentPoint[2]);
								glEnd();
							}
						}
						glPopMatrix();
					}
				}
				else{  //we need to generate our ring points.  We'll do <slices> number of interations and intersect a bunch of rays rotated around our cone direction with the cylinder, and store the points in a big vector so they can be quickly drawn in the future.
					for(int c = 0; c < 2; c++){
						glPushMatrix();
						if(c == 1)
							glTranslatef(currentDots.coneDirection[i][0], currentDots.coneDirection[i][1], currentDots.coneDirection[i][2]);
						currentDots.ringPoints[i][c].ringPoints.clear();
						//generate our main vectors outside of for loop to save a little computation
						arVector3 lastPoint;  //this will save us computation
						arVector3 vertexPosition = arVector3(currentDots.vertexPosition[0],currentDots.vertexPosition[1],currentDots.vertexPosition[2]);
						if(c == 1){
							vertexPosition = add(vertexPosition, currentDots.coneDirection[i]);
						}
						arVector3 coneDirection = normalize(arVector3(currentDots.coneDirection[i][0],currentDots.coneDirection[i][1],currentDots.coneDirection[i][2]));
						arVector3 arbitraryDirection = arVector3(0,0,1); 
						arVector3 perpCompX = normalize(crossProduct(coneDirection, arbitraryDirection));
						arVector3 perpCompY = normalize(crossProduct(coneDirection, perpCompX));
						arVector3 perpendicularComponent = perpCompX;
						arVector3 rayDirection = coneDirection + tan(currentDots.coneAngle[i]*PI/180)*perpendicularComponent;  //coneDirection should be normalized too
						//set up a cone intersection algorithm
						lastPoint = intersectCylinder(vertexPosition,normalize(rayDirection),RADIUS,HEIGHT);
						arVector3 newPoint;
						for(int a = 360/slices; a <= 360; a+=360./slices){
							perpendicularComponent = normalize( add( cos(a*PI/180.) * perpCompX, sin(a*PI/180) * perpCompY));
							rayDirection = coneDirection + tan(currentDots.coneAngle[i]*PI/180)*perpendicularComponent;  //coneDirection should be normalized too
							newPoint = intersectCylinder(vertexPosition,normalize(rayDirection),RADIUS,HEIGHT);
							if(doCherenkovCone){
								glBegin(GL_LINES);
								glVertex3f(0,0,0);
								glVertex3f(lastPoint[0],lastPoint[1],lastPoint[2]);
								glVertex3f(newPoint[0],newPoint[1],newPoint[2]);
								glVertex3f(0,0,0);
								glEnd();
							}

							currentDots.ringPoints[i][c].ringPoints.push_back(newPoint);
							lastPoint = newPoint;
						}
						currentDots.haveRingPoints[i] = true;
						glPopMatrix();
					}
				}

				//go ahead and connect all the points we have generated by now
				glColor4f(1.0,0,1.0,1.0); //purple is the color if we don't know what the particle is
				if(abs(currentDots.particleType[i]) == 11){  //electron blue
					glColor4f(0,0,1,1);
				}
				if(abs(currentDots.particleType[i]) == 13){ //muon is green
					glColor4f(0,1,0,1);
				}
				if(abs(currentDots.particleType[i]) == 211){ //pion is red
					glColor4f(1,0,0,1);
				}
				glLineWidth(5.0);
				for(int c = 0; c < 2; c++){
					glPushMatrix();
					if(c == 1)
							glTranslatef(currentDots.coneDirection[i][0], currentDots.coneDirection[i][1], currentDots.coneDirection[i][2]);
					arVector3 lastPointHolder = currentDots.ringPoints[i][c].ringPoints[currentDots.ringPoints[i][c].ringPoints.size()-1];
					for(int k = 0; k < currentDots.ringPoints[i][c].ringPoints.size();k++){
						arVector3 lastPoint = currentDots.ringPoints[i][c].ringPoints[k];
						glBegin(GL_LINES);
						glVertex3f(lastPoint[0],lastPoint[1],lastPoint[2]);
						glVertex3f(lastPointHolder[0],lastPointHolder[1],lastPointHolder[2]);
						glEnd();
						lastPointHolder = lastPoint;
					}
					glPopMatrix();
				}
				glLineWidth(1.0);
			
			
				
			}

			glDisable(GL_BLEND);
		glPopMatrix();
	}
	else{  //doing time compression, just drawing VERTICES that are stored in ENERGY
		glPushMatrix();
		glColor4f(1.f,1.f,.8,1.f);
		for(int p = 0; p < currentDots.energy.size()/3; p++){
			glPushMatrix();
			glTranslatef(currentDots.energy[p*3],currentDots.energy[p*3+1],currentDots.energy[p*3+2]);
			glutSolidSphere(.25,50,50);  //this is the particle
			glPopMatrix();
		}
		glPopMatrix();
	}
	glPopMatrix();


	debugText("ended draw scene");
	//   glutSwapBuffers(); //this is not necessary in syzygy.  This crashes things in syzygy.  -jaz
}

//takes a vector of dots and organizes it by time, lowest first and highest last
vector<dot> organizeByTime(vector<dot> dots){
	for(int i = 0; i < dots.size(); i++){
		int minSoFar = i;
		for(int j = i+1; j < dots.size(); j++){
			if(dots[j].time < dots[minSoFar].time){
				minSoFar = j;
			}
		}
		dot holder = dots[i];
		dots[i] = dots[minSoFar];
		dots[minSoFar] = holder;
	}
	return dots;
}

/*  logic to fill a dotVector.  Reads from the file and populates a dotVector until it hits a NEXTEVENT line, where it will do the physics calculation for each final particle to determine cone angle, and then stores the dot vector and exits */
void loadNextEvent(void) {
	int hit;
	arVector3 theta;
	float x,y,z,q,t;
	string type;
	float filler;
	float time;
	float vx, vy,vz;
	vector<float> particleType, dx, dy, dz, momentum, id;
	
	double maxTime, minTime;  //max and min times set at 1 standard deviation

	//float particleType, dx, dy, dz, momentum, id;
	float particleType2, dx2, dy2, dz2, momentum2, id2;
	//momentum = 0;
	float momentumHold = 0;
	vector<dot> dots;
	vector<dot> outerDots;
	bool debug = false;
	if(dataFile.is_open()) {
		while (dataFile.good()) {
			dataFile >> type;
			if(type == "ID"){  //parse inner detector
				dataFile >> hit >> x >> y >> z >> q >> t;
				x = x / 100.0;
				y = y / 100.0;
				z = z * 20.0 / 1810.0 + 20.0;
				theta = (z==0 || z == 40) ? arVector3(0.0,0.0,1.0): arVector3(x,y,0.0);
				tempDot = dot(x,y,z,theta,q,t,innerDotRad);
				dots.push_back(tempDot);
			}
			if(type == "OD"){  //parse outer detector
				dataFile >> hit >> x >> y >> z >> q >> t;
				x = x / 100.0;
				y = y / 100.0;
				z = z * 20.0 / 1810.0 + 20.0;
				theta = (z <= -0.6 || z >= 40.6) ? arVector3(0.0,0.0,1.0): arVector3(x,y,0.0);
				tempDot = dot(x,y,z,theta,q,t,outerDotRad);
				outerDots.push_back(tempDot);
			}
			if(type == "TIME"){ //parse time info
				dataFile >> time;
			}
			if(type == "VERTEX"){  //vertex location of particle
				dataFile >> vx >> vy >> vz;
				vx = vx / 100 * 3.28;
				vy = vy / 100 * 3.28;
				vz = vz * 20.0 / 1810.0 + 20.0;
				vz = vz * 3.28;
			}
			if(type == "PARTICLE"){  //particle information -- momentum and direction of cone
				dataFile >> particleType2 >> dx2 >> dy2 >> dz2 >> momentum2 >> id2;

				particleType.push_back(particleType2);
				dx.push_back(dx2);
				dy.push_back(dy2);
				dz.push_back(dz2);
				momentum.push_back(momentum2);
				id.push_back(id2);
			}
			if(type == "NEXTEVENT"){  //store everything, create a new event
				currentDots.dots = dots;
				currentDots.outerDots = outerDots;
				currentDots.endTime = time;
				currentDots.vertexPosition[0] = vx;
				currentDots.vertexPosition[1] = vy;
				currentDots.vertexPosition[2] = vz;

				//store type
				for(int i = 0; i < particleType.size();i++){
					currentDots.particleType.push_back(particleType[i]);

					//normalize conedirection and store
					double mag = pow(dx[i],2) + pow(dy[i],2) + pow(dz[i],2);
					mag = sqrt(mag);
					arVector3 coneDirHold = arVector3(dx[i]/mag,dy[i]/mag,dz[i]/mag);
					currentDots.coneDirection.push_back(coneDirHold);

					//calculate angle
#ifdef WINNEUTRINO
					double momentumConverted = momentum[i] * pow(10.0,6) * 1.6e-19 / 2.998e8;
#else
					double momentumConverted = momentum[i] * pow(10,6) * 1.6e-19 / 2.998e8;
#endif
					double mass = electronMass;  //assume electron to start .. ID of electron is 11 / -11
					double cherenkovThreshold = cherenkovElectronThreshold;
					if(abs(currentDots.particleType[i]) == 13){  //id 13 and -13 is muon
						mass = muonMass;
						cherenkovThreshold = cherenkovMuonThreshold;
					}
					if(abs(currentDots.particleType[i]) == 211){  //id 211 and -211 is pion
						mass = pionMass;
						cherenkovThreshold = cherenkovPionThreshold;
					}
					double velocity = sqrt(pow(momentumConverted,2) / (pow(mass,2)+pow(momentumConverted,2)/pow(speedOfLight,2)));  //calculating velocity from momentum and mass, have to take in to account lorentz factor
					double energy = sqrt(pow(momentumConverted,2)*pow(speedOfLight,2)+pow(mass,2)*pow(speedOfLight,4)) / 1.602e-13;  //calculate total energy from velocity, convert to MeV

					if(energy > cherenkovThreshold){ //checks if it's over cherenkov energy threshold  ... I don't think this is actually doing anything meaningful right now
						double beta = velocity / speedOfLight;  //in m/s  
						double n = 1.33; 
						double angle = acos(1.0 / (beta * n)) * 180. / PI;  //angle in degrees, using equation cos(theta) = 1 / (n*beta) for cherenkov energy
						currentDots.coneAngle.push_back(angle);
					}
					else{
						currentDots.coneAngle.push_back(0);
					}
					currentDots.momentum.push_back(momentum[i]);
					currentDots.energy.push_back(energy);
					//and start an empty ringPoints class to be filled later
					vector<ringPointHolder> me;
					ringPointHolder me2;
					me.push_back(me2);
					me.push_back(me2);
					currentDots.ringPoints.push_back(me);
					currentDots.haveRingPoints.push_back(false); //since we haven't generated any ring points, fill this with false
					currentDots.doDisplay.push_back(false); //turn off all displays, in the next step we'll go ahead and turn on only the highest momentum particle
					//set haveRingPoints to false, this will have them be generated on the first frame

					string text = "???";
					if(currentDots.particleType[i] == 11){
						text = "Electron";
					}
					else if(currentDots.particleType[i] == -11){
						text = "Positron";
					}
					else if(currentDots.particleType[i] == 13){
						text = "Muon";
					}
					else if(currentDots.particleType[i] == -13){
						text = "Antimuon";
					}
					else if(currentDots.particleType[i] == 211){
						text = "Pion+";
					}
					else if(currentDots.particleType[i] == -211){
						text = "Pion-";
					}
					else{
						char me[100];
						printf(me,"GEANT %i",(int)currentDots.particleType[i]);
						text = me;
					}
					currentDots.particleName.push_back(text);

				}

				//we'll go ahead and load in the START TIME which is the END TIME of the previous event
				if(dotVectors.size() == 0){ //if we're the first event, start time is 0, length is thus 'time'
					currentDots.startTime = 0.0;
					currentDots.length = time;
				}else{
					int tempIndex = dotVectors.size()-1;  //this is the most recently added event (before the currently generated one)
					currentDots.startTime = dotVectors[tempIndex].endTime; 
					currentDots.length = time - currentDots.startTime;
				}
				return;
			}
			if(dataFile.fail()) break;

		}
		if(dataFile.fail() || !dataFile.good()) dataFile.close();
	} else {
		printf("file was not open! \n");
		throw 9001; //really?
	}
}

//reads in file, looping over loadNextEvent until an error is thrown (eg, the file has no more data)
void readInFile(arMasterSlaveFramework& fw){
	debugText("started read in file");
	char me[100];
	strcpy(me, filename);
	//strcpy(me,"data/");
	//strcat(me,filename);
	//dataFile.open("C:/users/owner/desktop/glut_cylinder/temp");
	dataFile.open(me);  //relative pathing, may be really temperamental ... run from SRC
	if(!dataFile.is_open()) {
		cout << "Unable to open file";
		exit(0);
	}
	index = 0;
	while(true){
		try{
			loadNextEvent();
			dotVectors.push_back(currentDots);
			currentDots = dotVector();
		}
		catch (int e){
			break;
		}
	}

	//file is read by now.  Now we're going to go ahead and compress events if we're doing time compression

	float timeStep = .05;
	index = 0;
	if(doTimeCompressed){  //here we have to compress all events in to a smaller number of events
		vector<dotVector> newDotVectors;
		newDotVectors.push_back(dotVectors[0]);
		currentDots = dotVector();
		currentDots.startTime = 0;
		for(int i = 1; i < dotVectors.size(); i++){
			if(dotVectors[i].startTime >= timeStep * index){
				currentDots.endTime = timeStep*index;
				currentDots.length = currentDots.endTime - currentDots.startTime;
				newDotVectors.push_back(currentDots);
				currentDots = dotVector();
				currentDots.startTime = timeStep*index;
				index++;
				continue;
			}
			for(int k = 0; k < dotVectors[i].dots.size(); k++){
				bool found = false;
				//search all existing dots in this event, see if any have the same vertex position, if so, just add this one's charge to that one
				for(int l = 0; l < currentDots.dots.size(); l++){
					if(dotVectors[i].dots[k].cx == currentDots.dots[l].cx && dotVectors[i].dots[k].cy == currentDots.dots[l].cy && dotVectors[i].dots[k].cz == currentDots.dots[l].cz){
						found = true;
						currentDots.dots[l].charge += dotVectors[i].dots[k].charge;
						break;
					}
				}
				if(!found){
					currentDots.dots.push_back(dotVectors[i].dots[k]);
				}
			}
			for(int k = 0; k < dotVectors[i].outerDots.size();k++){
				bool found = false;
				//same deal with outer dots.
				for(int l = 0; l < currentDots.outerDots.size(); l++){
					if(dotVectors[i].outerDots[k].cx == currentDots.outerDots[l].cx && dotVectors[i].outerDots[k].cy == currentDots.outerDots[l].cy && dotVectors[i].outerDots[k].cz == currentDots.outerDots[l].cz){
						found = true;
						currentDots.outerDots[l].charge += dotVectors[i].outerDots[k].charge;
						break;
					}
				}
				if(!found){
					currentDots.outerDots.push_back(dotVectors[i].outerDots[k]);
				}
			}
			currentDots.energy.push_back(dotVectors[i].vertexPosition[0]);
			currentDots.energy.push_back(dotVectors[i].vertexPosition[1]);
			currentDots.energy.push_back(dotVectors[i].vertexPosition[2]);
		}
		currentDots.endTime = dotVectors[dotVectors.size()-1].endTime;
		currentDots.length = currentDots.endTime - currentDots.startTime;
		newDotVectors.push_back(currentDots);
		dotVectors = newDotVectors;
		
		//and now that dotVectors contains compressed items, set the "time" value for each dot to be its current value plus its events start time
		for(int i = 0; i < dotVectors.size(); i++){
			for(int j = 0; j < dotVectors[i].dots.size(); j++){
				dotVectors[i].dots[j].time += dotVectors[i].startTime;
			}
			for(int j = 0; j < dotVectors[i].outerDots.size(); j++){
				dotVectors[i].outerDots[j].time += dotVectors[i].startTime;
			}
		}
	}

	index = 0;

	//order the particles in their vectors by time
	for(int i = 0; i < dotVectors.size(); i++){
		dotVectors[i].dots = organizeByTime(dotVectors[i].dots);
		dotVectors[i].outerDots = organizeByTime(dotVectors[i].outerDots);
	}
	
	//find the standard deviations, set maxTime and minTime
	//calculate mean
	for(int i = 0; i < dotVectors.size(); i++){
		float mean = 0;
		for(int j = 0; j < dotVectors[i].dots.size(); j++){
			mean += dotVectors[i].dots[j].time;
		}
		mean = mean / (double) dotVectors[i].dots.size();
		
		vector<float> deviances;
		for(int j = 0; j < dotVectors[i].dots.size(); j++){
			deviances.push_back( dotVectors[i].dots[j].time - mean);
			deviances[j] = deviances[j]*deviances[j];
		}	
		float sumOfDeviances = 0;
		for(int j = 0; j < deviances.size(); j++){
			sumOfDeviances += deviances[i];
		}
		float stdSquared = sumOfDeviances / (double)(dotVectors[i].dots.size());
		float std = sqrt(stdSquared);
		
		dotVectors[i].minTime = mean - 1 * std;
		dotVectors[i].maxTime = mean + 1 * std;
	}

	//now, we turn on display for first listed final state particle of each event
	//we do it here and not in Loadevent because I was getting a weird bug trying to do it there and got lazy
	for(int e = 0; e < dotVectors.size(); e++){
		int holder = 0;
		/*
		for(int i = 0; i < dotVectors[e].doDisplay.size(); i++){
		if(dotVectors[e].energy[i] > dotVectors[e].energy[holder]){
		holder = i;
		}
		}
		*/    if(dotVectors[e].doDisplay.size() > 0){
			dotVectors[e].doDisplay[0] = true;
		}

		//also go ahead and draw the scene to pre-load cones
		currentDots = dotVectors[e];
		drawScene(fw);
	}

	currentDots = dotVectors[index];
	debugText("ended read in file");
}

//helper function, returns true if i == menu index
bool updateMenuIndexState(int i){
	if(i == menuIndex){
		return true;
	}
	return false;
}

//function to draw a display.  Helper function for the GUI.  This will draw one display  with given parameters.
void menuItem::drawSelf(bool highlighted){
	glPushMatrix();  //do one display ... test process
	//for rotations, let's say we're 2 units away from the hand.
	glColor3f(.75,.75,.75);
	if(doHUD){
		glTranslatef(0,0,-5);
		glTranslatef(4,0,0);
		glRotatef(-atan(5/3.)*180/PI,0,1,0);
		glRotatef(atan(index/3.)*180/PI/3.,1,0,0);
		glTranslatef(0,index,0);
	}
	else{
		glTranslatef(index,0,0);
		glRotatef(-atan(index/3.)*180/PI,0,1,0);
		glRotatef(-20,1,0,0);
	}
	glScalef(1.0,1.0,.1);
	glutSolidCube(1.0);
	if(highlighted){
		glPushMatrix();
		glColor4f(0,1,0,.5);
		if(triggerDepressed){
			glScalef(1.2,1.2,.5);
		}else{
			glScalef(1.1,1.1,.5);
		}
		glutSolidCube(1);
		glColor3f(0,0,0);
		glutWireCube(1.001);
		glPopMatrix();
	}
	glColor3f(0,0,0);
	glLineWidth(1);
	glutWireCube(1.001);
	glScalef(.9,.9,1.2);
	glutSolidCube(1.0);

	//now we have the distance-tablet deal, let's try to write some text
	glScalef(.001,.001,.001);  //same scaling as we had to do before ish
	glTranslatef(-300,0,600);
	glColor3f(1.,1.,1.);
	glLineWidth(2);
	glTranslatef(startOffSetX,startOffSetY,0);
	for(int i = 0; i < numLines;i++){
		glPushMatrix();
		glScalef(scale,scale,scale);
		glTranslatef(0,-150 * i,0);
		char * text = content[i];
		for (char * p = text; *p; p++)
			glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
		glPopMatrix();
	}
	glPopMatrix();
}

void menu::drawSelf(arMasterSlaveFramework& fw){
	//figure out which one is selected
	int selected = menuIndex;  //should directly index to myItems.size()
	int intHolder = 0;
	if(b5){
		bool found = false;
		for(int i = 0; i < myItems.size(); i++){
			if(myItems[i].index == selected){
				intHolder = i;
				found = true;
				break;
			}
		}
		if(found){
			nextMenu = children[intHolder];
			if(!displayRanThisRound)
			{
				(*myItems[intHolder].onClick)();
				displayRanThisRound = true;
			}
		} else { //we aren't selecting anything at the moment
			nextMenu = mainMenu;
		}
	}
	glPushMatrix();
		glMultMatrixf(menuRotationMatrix.v);
		glTranslatef(menuPosition[0], menuPosition[1], menuPosition[2]);	
		for(int i = 0; i < myItems.size(); i++){
			myItems[i].drawSelf(selected == myItems[i].index);
		}
	glPopMatrix();
	
}

void toggleColorFunc(){
	colorByCharge = !colorByCharge;
}

void toggleScaleFunc(){
	doScaleByCharge = !doScaleByCharge;
}

void toggleODFunc(){
	doCylinderDivider = !doCylinderDivider;
}

void eventNumberDecrement(){
	if(doTimeCompressed){
		switch(autoPlay){
			case 0:  autoPlay = -1;  return;
			case -1:  return;
			case 1:  autoPlay = 0; return;
		}
	}
	if(index != 0){
		index--;
	}
}

void goToChild(){
	currentMenu = nextMenu;
}

void toggleCherenkovLines(){
	doCherenkovCone = !doCherenkovCone;
}

void eventNumberIncrement(){
	if(doTimeCompressed){
		switch(autoPlay){
			case 0:  autoPlay = 1;  return;
			case -1:  autoPlay = 0;  return;
			case 1:  return;
		}
	}
	if(index != dotVectors.size() - 1){
		index++;
	}
}

void closeMenuFunc(){
	doMenu = false;
}

void mainMenuFunc(){
	currentMenu = mainMenu;
	debugText("Switched to main menu \n");
}

void toggleColorKey(){
	if(myDataDisplays.size() >= 2){
		myDataDisplays[0].isVisible = !myDataDisplays[0].isVisible;
		myDataDisplays[1].isVisible = !myDataDisplays[1].isVisible;
	}
}

void toggleEventInfo(){
	if(myDataDisplays.size() >= 3){
		myDataDisplays[2].isVisible = !myDataDisplays[2].isVisible;
	}
}

void toggleWristMotion(){
	doWristBoundTime = !doWristBoundTime;
	debugText("Did Toggle Wrist Motion \n");
}

void toggleCherenkovCone(){
	int holder = menuIndex + 1;
	if(holder == -1)
		return;
	if(holder >= currentDots.doDisplay.size())
		return;
		
		
	debugText("did ToggleCherenkovCone \n");
	currentDots.doDisplay[holder] = !currentDots.doDisplay[holder];
	dotVectors[index].doDisplay[holder] = !dotVectors[index].doDisplay[holder];
}

void doInterface(arMasterSlaveFramework& framework){

	(currentMenu).drawSelf(framework);
}


// Unit conversions.  Tracker (and cube screen descriptions) use feet.
// Atlantis, for example, uses 1/2-millimeters, so the appropriate conversion
// factor is 12*2.54*20.
const float FEET_TO_LOCAL_UNITS = 1.;

// Near & far clipping planes.
const float nearClipDistance = .1*FEET_TO_LOCAL_UNITS;
const float farClipDistance = 1000.*FEET_TO_LOCAL_UNITS;

// Class definitions & imlpementations for effector
class RodEffector : public arEffector {
public:
	// set it to use matrix #1 (#0 is normally the user's head) and 3 buttons starting at 0
	// (no axes, i.e. joystick-style controls; those are generally better-suited for navigation
	// than interaction)
	RodEffector() : arEffector( 1, 3, 0, 0, 0, 0, 0 ) {

		// set length to 5 ft.
		setTipOffset( arVector3(0,0,-5) );

		// set to interact with closest object within 1 ft. of tip
		// (see interaction/arInteractionSelector.h for alternative ways to select objects)
		setInteractionSelector( arDistanceInteractionSelector(2.) );

		// set to grab an object (that has already been selected for interaction
		// using rule specified on previous line) when button 0 or button 2
		// is pressed. Button 0 will allow user to drag the object with orientation
		// change, button 2 will allow dragging but square will maintain orientation.
		// (see interaction/arDragBehavior.h for alternative behaviors)
		// The arGrabCondition specifies that a grab will occur whenever the value
		// of the specified button event # is > 0.5.
		setDrag( arGrabCondition( AR_EVENT_BUTTON, 0, 0.5 ), arWandRelativeDrag() );
		setDrag( arGrabCondition( AR_EVENT_BUTTON, 2, 0.5 ), arWandTranslationDrag() );
	}
	// Draw it.
	// The matrix can be gotten from updateState() on the slaves' postExchange().
	void draw(arMasterSlaveFramework& framework) const;
private:
};  //
void RodEffector::draw(arMasterSlaveFramework& framework) const {
	debugText("began drawing rod effector");
	glPushMatrix();
	glMultMatrixf( getCenterMatrix().v );  //transforms to hand position
	

	//we're going to draw the text on the wand, bound to hand, like a tablet
	//draw the tablet surface, which will be a scaled cube
	glColor3f(.5,.5,.5);
	glScalef(2./3,6./120,1.5);
	glTranslatef(0,1.5,1);
	glutSolidCube(1.);
	glColor3f(0,0,0);
	glutWireCube(1.01); //black wireframe, makes it easier to see
	glTranslatef(0,.1,0);
	glutSolidCube(.9);  //screen (black surface)
	
	//making a pincer-style addition at the top to point to the center of our "grabber"
	glPushMatrix();
		glTranslatef(.33,-.1,-.55);
		glColor3f(.5,.5,.5);
		glScalef(1,3,1);
		glutSolidCube(.25);
		glColor3f(0,0,0);
		glutWireCube(.26);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-.33,-.1,-.55);
		glColor3f(.5,.5,.5);
		glScalef(1,3,1);
		glutSolidCube(.25);
		glColor3f(0,0,0);
		glutWireCube(.26);
	glPopMatrix();
	


	//HERE WE DO THE TABLET DISPLAY
	//WHICH is currently hardcoded, sorry about that
	glScalef(.007,.007,.007); 
	glRotatef(90,1,0,0);  
	glColor3f(1.,1,1);  
	glTranslatef(0,0,-90.0);
	glRotatef(180,0,0,1);
	glRotatef(180,0,1,0);
	glTranslatef(-50,-50,0);
	//at this point we're centered on bottom left of display, and oriented correctly
	glScalef(.1,.1,.1);  //this will determine text size
	glLineWidth(1.9);  //and width

	//at the current settings, the top right is (1000,1000,0)
	glPushMatrix();
	glTranslatef(-50,950,0);
	char * text = "Event Data";
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0,925,0);
	text = "__________";
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-50,700,0);
	text = "Color By: ";
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	if(colorByCharge){
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,'Q');
	}
	else{
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,'T');
	}
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-50,550,0);
	text = "Scale by Q?: ";
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	if(doScaleByCharge){
		text = "ON";
	}
	else{
		text = "OFF";
	}
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-50,400,0);
	text = "OD?: ";
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	if(doCylinderDivider){
		text = "ON";
	}
	else{
		text = "OFF";
	}
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-50,250,0);
	text = "Cone visible?: ";
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	if(doCherenkovCone){
		text = "ON";
	}
	else{
		text = "OFF";
	}
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
	/*
	glPushMatrix();
	glTranslatef(0,300,0);
	text = "Particle: ";
	for (char * p = text; *p; p++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	//glPopMatrix();
	//glPushMatrix();
	//	glTranslatef(0,200,0);
	text = "???";
	if(currentDots.particleType[0] == 11){
	text = "Electron";
	}
	if(currentDots.particleType[0] == -11){
	text = "Positron";
	}
	if(currentDots.particleType[0] == 13){
	text = "Muon";
	}
	if(currentDots.particleType[0] == -13){
	text = "Antimuon";
	}
	if(currentDots.particleType[0] == 211){
	text = "Pion+";
	}
	if(currentDots.particleType[0] == -211){
	text = "Pion-";
	}
	for (char * p = text; *p; p++)
	glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
	*/
	glPushMatrix();
	glTranslatef(-50,100,0);
	text = "Event ";
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);

	char buffer[30];
	itoa(index+1,buffer,10);
	text = buffer;
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);

	text = " / ";
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);

	itoa(dotVectors.size(),buffer,10);
	text = buffer;
	for (char * p = text; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);

	glPopMatrix();
	glLineWidth(1.0);
	glPopMatrix();

	debugText("Ended drawing wand");

}

// End of classes



//global variable for our effector
RodEffector theEffector;


// start callback (called in arMasterSlaveFramework::start()
//
// Note: DO NOT do OpenGL initialization here; this is now called
// __before__ window creation. Do it in the WindowStartGL callback.
//
bool start( arMasterSlaveFramework& framework, arSZGClient& /*cli*/ ) {
	// Register shared memory.
	//  framework.addTransferField( char* name, void* address, arDataType type, int numElements ); e.g.
	//  framework.addTransferField("squareHighlighted", &squareHighlightedTransfer, AR_INT, 1);

	//add transfer field for current index
	framework.addTransferField("indexTransferField",&index,AR_INT,1);
	framework.addTransferField("autoPlayTransfer",&autoPlay,AR_INT,1);
	framework.addTransferField("colorByChargeTransfer",&colorByCharge,AR_INT,1);
	framework.addTransferField("doCylinderDividerTransfer",&doCylinderDivider,AR_INT,1);
	framework.addTransferField("doScaleByChargeTransfer",&doScaleByCharge,AR_INT,1);
	framework.addTransferField("doMenuTransfer",&doMenu,AR_INT,1);
	framework.addTransferField("menuIndexTransfer",&menuIndex,AR_INT,1);
	framework.addTransferField("optionsTransfer",&doOptionsMenu,AR_INT,1);
	framework.addTransferField("menuIndexX",&menuIndex,AR_INT,1);
	framework.addTransferField("colorKeyX",&doColorKey,AR_INT,1);
	framework.addTransferField("doCherenkovMenuTransfer",&doCherenkovConeMenu,AR_INT,1);
	framework.addTransferField("doMainMenuTransfer",&doMainMenu,AR_INT,1);
	framework.addTransferField("CherenkovConeMenuIndexIndexTransfer",&cherenkovConeMenuIndex,AR_INT,1);
	framework.addTransferField("modifiedCherenkovConeItem",&modifiedCherenkovConeIndex,AR_INT,1);	
	framework.addTransferField("isTouchingVertex",&isTouchingVertex,AR_INT,1);
	framework.addTransferField("isGrabbingVertex",&isGrabbingVertex,AR_INT,1);
	framework.addTransferField("itemTouching",&itemTouching ,AR_INT,1);
	framework.addTransferField("starttransfer", &triggerDepressed, AR_INT, 1);
	framework.addTransferField("doTimePlaybackTransfer", &doTimePlayback, AR_INT, 1);
	framework.addTransferField("tt1r", &elapsedTime, AR_DOUBLE, 1);
	framework.addTransferField("tt2r", &eventBegan, AR_DOUBLE, 1);
	framework.addTransferField("b5", &b5, AR_INT, 1);
	framework.addTransferField("doWrist", &doWristBoundTime, AR_INT, 1);
	framework.addTransferField("rotMatrixTransfer", menuRotationMatrix.v, AR_FLOAT, 16);
	framework.addTransferField("menuPosTransfer", menuPosition.v, AR_FLOAT, 3);
	framework.addTransferField("lastButtonPressxfer", &lastButtonPress, AR_DOUBLE, 1);
	framework.addTransferField("Grabbingdisplayxfer", &grabbingDisplay, AR_INT, 1);


	// Setup navigation, so we can drive around with the joystick
	//
	// Tilting the joystick by more than 20% along axis 1 (the vertical on ours) will cause
	// translation along Z (forwards/backwards). This is actually the default behavior, so this
	// line isn't necessary.
	framework.setNavTransCondition( 'z', AR_EVENT_AXIS, 1, 0.2 );      

	// Tilting joystick left or right will rotate left/right around vertical axis (default is left/right
	// translation)
	//framework.setNavRotCondition( 'y', AR_EVENT_AXIS, 0, 0.2 );      
	framework.setNavTransCondition('x',AR_EVENT_AXIS,0,.1);

	// Set translation & rotation speeds to 5 ft/sec & 30 deg/sec (defaults)
	framework.setNavTransSpeed( 15. );
	//framework.setNavRotSpeed( 0.2 );  //sets it to not rotate at all so we can use left/right joystick as event controller

	
	
	//initialize data displays
	//two displays, one for color key, one for neutrino information
	dataDisplay colorKeyCharge;
	colorKeyCharge.contents.push_back(textItem("Color Key Charge:", arVector3(1,1,1)));
	float values []= {26.7, 23.3, 20.2, 17.3, 14.7, 12.2, 10, 8, 6.2, 4.7, 3.3, 2.2, 1.3, .7, .2};
	int sections []= {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	for(int i = 0; i < sizeof(values)/4; i++){
		int section = sections[i];
		char dest[50];
		sprintf(dest, "%f MeV", values[i]);
		colorKeyCharge.contents.push_back(textItem(dest, arVector3( red_values[section] / 255.0, blue_values[section] / 255.0, green_values[section] / 255.0)));
	}
	colorKeyCharge.myPos = arVector3(0,0,0);
	colorKeyCharge.myRotationMatrix = arMatrix4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	colorKeyCharge.isVisible = false;
	myDataDisplays.push_back(colorKeyCharge);
	
	dataDisplay colorKeyTime;
	colorKeyTime.contents.push_back(textItem("Color Key Time:", arVector3(1,1,1)));
	float values2 [] = {893, 909, 925, 941, 957, 973, 989, 1005, 1021, 1037, 1053, 1069, 1085, 1101, 1117};
	int sections2 [] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	for(int i = 0; i < sizeof(values2)/4; i++){
		int section = sections2[i];
		char dest[50];
		sprintf(dest, "%f ns", values2[i]);
		colorKeyTime.contents.push_back(textItem(dest, arVector3( red_values[section] / 255.0, blue_values[section] / 255.0, green_values[section] / 255.0)));
	}
	colorKeyTime.myPos = arVector3(0,0,0);
	colorKeyTime.myRotationMatrix = arMatrix4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	colorKeyTime.isVisible = false;
	myDataDisplays.push_back(colorKeyTime);
	
	//third display containing information on current event (will have to be updated every event increment)
	dataDisplay eventInformation;
	eventInformation.myPos = arVector3(0,0,0);
	eventInformation.myRotationMatrix = arMatrix4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	eventInformation.isVisible = false;
	myDataDisplays.push_back(eventInformation);

	
	
	//initialize menus
	menuItem eventMinus;
	eventMinus.index = -2;
	eventMinus.numLines = 3;
	eventMinus.startOffSetX = 0;
	eventMinus.startOffSetY = 200;
	eventMinus.scale = 2;
	eventMinus.content[0] = " -";
	eventMinus.content[1] = "Event";
	eventMinus.content[2] = " -";
	eventMinus.onClick = &eventNumberDecrement;
	mainMenu.myItems.push_back(eventMinus);
	
	menuItem optionsItem;
	optionsItem.index = -1;
	optionsItem.numLines = 1;
	optionsItem.startOffSetX = -50;
	optionsItem.startOffSetY = 0;
	optionsItem.scale = 1.75;
	optionsItem.content[0] = "Options";
	optionsItem.onClick = &goToChild;
	mainMenu.myItems.push_back(optionsItem);
	
	menuItem gotoCones;
	gotoCones.index = 0;
	gotoCones.numLines = 2;		
	gotoCones.content[0] = "Cherenkov";
	gotoCones.content[1] = " Cones";
	gotoCones.startOffSetX = -100;
	gotoCones.startOffSetY = 50;
	gotoCones.scale = 1.5;
	gotoCones.onClick = &goToChild;
	mainMenu.myItems.push_back(gotoCones);
	
	menuItem displaysItem;
	displaysItem.index = 1;
	displaysItem.numLines = 2;
	displaysItem.startOffSetX = -50;
	displaysItem.startOffSetY = 50;
	displaysItem.scale = 1.75;
	displaysItem.content[0] = "Time";
	displaysItem.content[1] = "Playback";
	displaysItem.onClick = &goToChild;
	mainMenu.myItems.push_back(displaysItem);
	
	menuItem eventPlus;
	eventPlus.index = 2;
	eventPlus.numLines = 3;
	eventPlus.startOffSetX = 0;
	eventPlus.startOffSetY = 200;
	eventPlus.scale = 2;
	eventPlus.content[0] = " +";
	eventPlus.content[1] = "Event";
	eventPlus.content[2] = " +";
	eventPlus.onClick = &eventNumberIncrement;
	mainMenu.myItems.push_back(eventPlus);
	
	
	//populate options menu
	menuItem goBack;
	goBack.index = -1;
	goBack.numLines = 2;		
	goBack.content[0] = "Main";
	goBack.content[1] = "Menu";
	goBack.startOffSetX = 0;
	goBack.startOffSetY = 100;
	goBack.scale = 2;
	(goBack.onClick) = &mainMenuFunc;
	optionsMenu.myItems.push_back(goBack);
	
	menuItem displaysMenuItem;
	displaysMenuItem.index = 0;
	displaysMenuItem.numLines = 2;		
	displaysMenuItem.content[0] = "Displays";
	displaysMenuItem.content[1] = " Menu";
	displaysMenuItem.startOffSetX = -150;
	displaysMenuItem.startOffSetY = 100;
	displaysMenuItem.scale = 2;
	(displaysMenuItem.onClick) = &goToChild;
	optionsMenu.myItems.push_back(displaysMenuItem);
	
	menuItem visMenuItem;
	visMenuItem.index = 1;
	visMenuItem.numLines = 2;		
	visMenuItem.content[0] = "Vis";
	visMenuItem.content[1] = "Menu";
	visMenuItem.startOffSetX = 0;
	visMenuItem.startOffSetY = 100;
	visMenuItem.scale = 2;
	(visMenuItem.onClick) = &goToChild;
	optionsMenu.myItems.push_back(visMenuItem);
	
	
	//populate vis menu
	goBack.index = -2;
	goBack.numLines = 2;		
	goBack.content[0] = "Main";
	goBack.content[1] = "Menu";
	goBack.startOffSetX = 0;
	goBack.startOffSetY = 100;
	goBack.scale = 2;
	(goBack.onClick) = &mainMenuFunc;
	visMenu.myItems.push_back(goBack);
	
	menuItem doRadiation;
	doRadiation.index = -1;
	doRadiation.numLines = 2;		
	doRadiation.content[0] = "Draw";
	doRadiation.content[1] = "Radiation";
	doRadiation.startOffSetX = 0;
	doRadiation.startOffSetY = 100;
	doRadiation.scale = 1.5;
	(doRadiation.onClick) = &toggleCherenkovLines;
	visMenu.myItems.push_back(doRadiation);
	
	
	menuItem toggleColor;
	toggleColor.index = 0;
	toggleColor.numLines = 2;		
	toggleColor.content[0] = "Toggle";
	toggleColor.content[1] = "Color";
	toggleColor.startOffSetX = 0;
	toggleColor.startOffSetY = 100;
	toggleColor.scale = 2;
	(toggleColor.onClick) = &toggleColorFunc;
	visMenu.myItems.push_back(toggleColor);
	
	
	menuItem toggleScale;
	toggleScale.index = 1;
	toggleScale.numLines = 3;		
	toggleScale.content[0] = "Scale";
	toggleScale.content[1] = "By";
	toggleScale.content[2] = "Charge";
	toggleScale.startOffSetX = 0;
	toggleScale.startOffSetY = 100;
	toggleScale.scale = 1.5;
	(toggleScale.onClick) = &toggleScaleFunc;
	visMenu.myItems.push_back(toggleScale);
	
	
	menuItem toggleOD;
	toggleOD.index = 2;
	toggleOD.numLines = 2;		
	toggleOD.content[0] = "Toggle";
	toggleOD.content[1] = "OD";
	toggleOD.startOffSetX = 0;
	toggleOD.startOffSetY = 100;
	toggleOD.scale = 2;
	(toggleOD.onClick) = &toggleODFunc;
	visMenu.myItems.push_back(toggleOD);
	
	
	//populate displays menu
	goBack.index = -1;
	goBack.numLines = 2;		
	goBack.content[0] = "Main";
	goBack.content[1] = "Menu";
	goBack.startOffSetX = 0;
	goBack.startOffSetY = 100;
	goBack.scale = 2;
	(goBack.onClick) = &mainMenuFunc;
	displaysMenu.myItems.push_back(goBack);
	
	menuItem toggleKey;
	toggleKey.index = 0;
	toggleKey.numLines = 3;		
	toggleKey.content[0] = "Toggle";
	toggleKey.content[1] = "Color";
	toggleKey.content[2] = " Key";
	toggleKey.startOffSetX = 0;
	toggleKey.startOffSetY = 100;
	toggleKey.scale = 1.5;
	(toggleKey.onClick) = &toggleColorKey;
	displaysMenu.myItems.push_back(toggleKey);
	
	menuItem toggleInfo;
	toggleInfo.index = 1;
	toggleInfo.numLines = 3;		
	toggleInfo.content[0] = "Toggle";
	toggleInfo.content[1] = "Event";
	toggleInfo.content[2] = " Info";
	toggleInfo.startOffSetX = 0;
	toggleInfo.startOffSetY = 100;
	toggleInfo.scale = 1.5;
	(toggleInfo.onClick) = &toggleEventInfo;
	displaysMenu.myItems.push_back(toggleInfo);
	
	//populate cones menu
	goBack.index = -2;
	goBack.numLines = 2;		
	goBack.content[0] = "Main";
	goBack.content[1] = "Menu";
	goBack.startOffSetX = 0;
	goBack.startOffSetY = 100;
	goBack.scale = 2;
	(goBack.onClick) = &mainMenuFunc;
	conesMenu.myItems.push_back(goBack);
	
	//populate controlMenu
	goBack.index = -1;
	goBack.numLines = 2;		
	goBack.content[0] = "Main";
	goBack.content[1] = "Menu";
	goBack.startOffSetX = 0;
	goBack.startOffSetY = 100;
	goBack.scale = 2;
	(goBack.onClick) = &mainMenuFunc;
	controlsMenu.myItems.push_back(goBack);
	
	menuItem activateWrist;
	activateWrist.index = 0;
	activateWrist.numLines = 3;		
	activateWrist.content[0] = "Toggle";
	activateWrist.content[1] = "Wrist";
	activateWrist.content[2] = "Binding";
	activateWrist.startOffSetX = 0;
	activateWrist.startOffSetY = 200;
	activateWrist.scale = 1.5;
	(activateWrist.onClick) = &toggleWristMotion;
	controlsMenu.myItems.push_back(activateWrist);
	
	
	controlsMenu.children.push_back(mainMenu);
	controlsMenu.children.push_back(mainMenu);
	optionsMenu.children.push_back(displaysMenu);
	optionsMenu.children.push_back(displaysMenu);
	optionsMenu.children.push_back(visMenu);
	optionsMenu.children.push_back(visMenu);
	optionsMenu.children.push_back(visMenu);
	optionsMenu.children.push_back(visMenu);
	optionsMenu.children.push_back(visMenu);
	for(int i = 0; i < 25; i++){
		conesMenu.children.push_back(mainMenu);
		visMenu.children.push_back(mainMenu);
	}
	mainMenu.children.push_back(optionsMenu);
	mainMenu.children.push_back(optionsMenu);
	mainMenu.children.push_back(conesMenu);
	mainMenu.children.push_back(controlsMenu);
	mainMenu.children.push_back(controlsMenu);
	
	
	
	
	currentMenu = mainMenu;
	
	return true;
}

// Callback to initialize each window (because now a Syzygy app can
// have more than one).
void windowStartGL( arMasterSlaveFramework& fw, arGUIWindowInfo* ) {
	debugText("started window start gl");
	
	glClearColor(0,0,0,0);
	quadObj = gluNewQuadric();  //this just needs to go in any of the initialization files
	readInFile(fw);  //opens the file
	
	//lighting, just for the particle at the moment
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat light_diffuse[] = {1.0,1.0,.69,.7};
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);

	glMaterialfv(GL_FRONT, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	debugText("ended window start gl");
}


	// Callback called before data is transferred from master to slaves. Only called
	// on the master. This is where anything having to do with
	// processing user input or random variables should happen.
	void preExchange( arMasterSlaveFramework& fw ) {
		debugText("started preExchange");

		
		// Do stuff on master before data is transmitted to slaves.

		// handle joystick-based navigation (drive around). The resulting
		// navigation matrix is automagically transferred to the slaves.
		fw.navUpdate();

		// update the input state (placement matrix & button states) of our effector.
		theEffector.updateState( fw.getInputState() );

		//update our global time (recoreded on master)
		currentTime = clock()/(double)CLOCKS_PER_SEC;
		
		//update elapsed time
		elapsedTime = clock()/(double)CLOCKS_PER_SEC - eventBegan;
		
		

		//handle autoplay end-stopping (For supernova mode)
		if(index == 0 || index == dotVectors.size()-1){
			autoPlay = 0;
		}
		
		
		if(autoPlay == 1){
			if(elapsedTime > currentDots.length){
				index++;
				eventBegan = currentTime;
			}
		}
		
		if(autoPlay == -1){
			if(elapsedTime > currentDots.length){
				index--;
				eventBegan = currentTime;
			}
		}
		
		//button mapping for reference:
		// 0 = yellow
		// 1 = red
		// 2 = green
		// 3 = blue
		// 4 = joystick press
		// 5 = back button (master)


		//do button presses
		if(fw.getOnButton(5) && clock() / (double) CLOCKS_PER_SEC - lastButtonPress > buttonPressThreshold){
			b5 = true;
			lastButtonPress = clock() / (double) CLOCKS_PER_SEC;
		} else {
			b5 = false;
		}

		//figure out if we're touching the vertex
		arMatrix4 myPosMatrix( ar_getNavInvMatrix() );
		double navX = myPosMatrix[12];
		double navY = myPosMatrix[13];
		double navZ = myPosMatrix[14];
		navY += 20;
		double holder = navY;
		navY = navZ;
		navZ = holder;
		//now navx, navy, navz are in coordinates of cylinder ... that being, y and z are flipped
		
		arMatrix4 myHandMatrix = fw.getMatrix(1);
		double handPosX = myHandMatrix[12];  //this is hand position
		double handPosY = myHandMatrix[13];
		double handPosZ = myHandMatrix[14];
		
		double handDirX = myHandMatrix[8];
		double handDirY = myHandMatrix[9];
		double handDirZ = myHandMatrix[10];
		
		double handUpX = myHandMatrix[4];
		double handUpY = myHandMatrix[5];
		double handUpZ = myHandMatrix[6];
		
		double finalX = -navX + handPosX + -2*handDirX;
		double finalY = -navY + handPosZ + -2*handDirZ;
		double finalZ = navZ + -handPosY + 2*handDirY;
		
		//figure out which item we're grabbing ... go for closest one
		arVector3 handPos = arVector3(finalX, finalY, finalZ);
		arVector3 vertexPos = arVector3(currentDots.vertexPosition[0], currentDots.vertexPosition[1], currentDots.vertexPosition[2]);
		float minDist = magnitude( subtract(vertexPos, handPos));
		itemTouching = 0;
		for(int i = 0; i < currentDots.coneDirection.size(); i++){
			arVector3 myNewPos = add(vertexPos, currentDots.coneDirection[i]);
			float newDist = magnitude(subtract( myNewPos, handPos));
			if( newDist < minDist){
				minDist = newDist;
				itemTouching = i+1;
			}
		}
		
		if(minDist < .5){
			isTouchingVertex = true;
			originalPosition = arVector3(handPosX, handPosY, handPosZ);
			originalDirection = arVector3(handDirX, handDirY, handDirZ);
		}
		else
			isTouchingVertex = false;
			
		//figure out wrist rotation, which is the angle between ( <0,1,0> + handDir ) and <handUp>
		arVector3 upHolder = arVector3(0,1,0);
		arVector3 handUp = arVector3(handUpX, handUpY, handUpZ);
		double angleHolder = acos(dotProduct(normalize(handUp), normalize(upHolder)));
		if( dotProduct(crossProduct(handUp, upHolder),arVector3(handDirX, handDirY, handDirZ)) > 0)
			wristRotation = angleHolder * 180.0 / PI;
		else 
			wristRotation = -angleHolder* 180.0 / PI;



		//update menuIndex (based on raycasting)
		arMatrix4 currentHand = ar_getNavMatrix() * fw.getMatrix(1);
		arVector3 lineOrigin(currentHand[12], currentHand[13], currentHand[14]);
		arVector3 lineDirection(currentHand[8], currentHand[9], currentHand[10]);
		arVector3 pointOnPlane(menuRotationMatrix[12], menuRotationMatrix[13], menuRotationMatrix[14]);
		arVector3 planeNormal(menuRotationMatrix[8],menuRotationMatrix[9], menuRotationMatrix[10]);
		pointOnPlane = pointOnPlane - menuDist * normalize(planeNormal);
		planeNormal = normalize(planeNormal);
		lineDirection = normalize(lineDirection);
		
		float d = dotProduct( arVector3(pointOnPlane - lineOrigin), planeNormal);
		d = d / dotProduct(lineDirection, planeNormal);
		
		arVector3 intersectionPoint = d * lineDirection + lineOrigin;
		
		//now intersectionpoint - pointonplane should be the offset in a combination of x and y vectors, and offset of the keyboard components  (do fine tuning here)
		float yvalue = dotProduct(intersectionPoint-pointOnPlane, normalize(arVector3(menuRotationMatrix[4], menuRotationMatrix[5], menuRotationMatrix[6])));
		float xvalue = dotProduct(intersectionPoint-pointOnPlane, normalize(arVector3(menuRotationMatrix[0], menuRotationMatrix[1], menuRotationMatrix[2])));
		
		if(yvalue > .5 || yvalue < -.5){
			menuIndex = 100;
		}else{
			menuIndex = (int) floor((xvalue+.5));
			
		}
		
		
		triggerDepressed = fw.getButton(5);

		
		if(!fw.getButton(5)){
			isGrabbingVertex = false;
		}
		if(fw.getButton(5) && isTouchingVertex){
			isGrabbingVertex = true;
			doMenu = false;
		}
		if(b5 && doMenu && !isGrabbingVertex && !grabbingDisplay){
			bool found = false;
			for(int i = 0; i < currentMenu.myItems.size(); i++){
				if(menuIndex == currentMenu.myItems[i].index){
					found = true;
					break;
				}
			}
			doMenu = found;
		}
		else if(b5 && !isGrabbingVertex && !grabbingDisplay){  //this is the master control button.  It turns on the menu, and selects menu items
			if(!doMenu && !isTouchingVertex){
				doMenu = true;
				menuPosition = arVector3(0,0,-menuDist);
				menuRotationMatrix = ar_getNavMatrix() * fw.getMatrix(1);
			}
		}
		if(isGrabbingVertex && isTouchingVertex || grabbingDisplay){
			doMenu = false;
		}
		
		doMenu = doMenu && !grabbingDisplay;
		
		
		grabbingDisplay = false;
		
		
		debugText("ended preexchange");
}

// Callback called after transfer of data from master to slaves. Mostly used to
// synchronize slaves with master based on transferred data.
void postExchange( arMasterSlaveFramework& fw ) {
	// Do stuff after slaves got data and are again in sync with the master.
	if (!fw.getMaster()) {
		// Update effector's input state. On the slaves we only need the matrix
		// to be updated, for rendering purposes.
		theEffector.updateState( fw.getInputState() );

		// Unpack our transfer variables.
	}
	
		//do button presses
		/*
	cout << "Current time is " << clock() / (double) CLOCKS_PER_SEC << "\n";
	cout << "Current b5 value is" << b5 << "\n";
	cout << "Last button pres was " << lastButtonPress << "\n";
	cout << "Button press threshold is " << buttonPressThreshold << "\n\n";
*/

	for(int i = 0; i < needsReleasing.size(); i++){
		delete [] needsReleasing[i];
	}
	needsReleasing.clear();

	displayRanThisRound = false;

	if(!doMenu){
		currentMenu = mainMenu;
	}
	
	//handle vertex motion
		arMatrix4 myPosMatrix( ar_getNavInvMatrix() );
	double navX = myPosMatrix[12];
	double navY = myPosMatrix[13];
	double navZ = myPosMatrix[14];
	navY += 20;
	double holder = navY;
	navY = navZ;
	navZ = holder;
	//now navx, navy, navz are in coordinates of cylinder ... that being, y and z are flipped
	
	arMatrix4 myHandMatrix = fw.getMatrix(1);
	double handPosX = myHandMatrix[12];  //this is hand position
	double handPosY = myHandMatrix[13];
	double handPosZ = myHandMatrix[14];
	
	double handDirX = myHandMatrix[8];
	double handDirY = myHandMatrix[9];
	double handDirZ = myHandMatrix[10];
	
	double handUpX = myHandMatrix[4];
	double handUpY = myHandMatrix[5];
	double handUpZ = myHandMatrix[6];
	
	double finalX = -navX + handPosX + -2*handDirX;
	double finalY = -navY + handPosZ + -2*handDirZ;
	double finalZ = navZ + -handPosY + 2*handDirY;
	if(isGrabbingVertex){

			
		if(itemTouching == 0){
			//set vertex position to hand position
			currentDots.vertexPosition[0] = finalX;
			currentDots.vertexPosition[1] = finalY;
			currentDots.vertexPosition[2] = finalZ;
			dotVectors[index].vertexPosition[0] = finalX;
			dotVectors[index].vertexPosition[1] = finalY;
			dotVectors[index].vertexPosition[2] = finalZ;
			
			/*
			//now change cone direction
			float dirX = -handDirX;
			float dirY = -handDirZ;
			float dirZ = handDirY;
			
			currentDots.coneDirection[0][0] = dirX;
			currentDots.coneDirection[0][1] = dirY;
			currentDots.coneDirection[0][2] = dirZ;
			dotVectors[index].coneDirection[0][0] = dirX;
			dotVectors[index].coneDirection[0][1] = dirY;
			dotVectors[index].coneDirection[0][2] = dirZ;
			//*/
		}
		else { //touching a cone controller .. now we'll instead drag around the point representing cone direction
			//we want to make it so currentDots.coneDirection[itemtouching-1]+currentDots.vertexPosition = out hand position
			arVector3 vertexPos = arVector3(currentDots.vertexPosition[0],currentDots.vertexPosition[1], currentDots.vertexPosition[2]);
			arVector3 newDirection = subtract(arVector3(finalX, finalY, finalZ) , vertexPos);
			currentDots.coneDirection[itemTouching-1][0] = newDirection[0];
			currentDots.coneDirection[itemTouching-1][1] = newDirection[1];
			currentDots.coneDirection[itemTouching-1][2] = newDirection[2];
			dotVectors[index].coneDirection[itemTouching-1][0] = newDirection[0];
			dotVectors[index].coneDirection[itemTouching-1][1] = newDirection[1];
			dotVectors[index].coneDirection[itemTouching-1][2] = newDirection[2];
			
		}
	}
	modifiedCherenkovConeIndex = -1;
	
				
	//figure out wrist rotation, which is the angle between ( <0,1,0> + handDir ) and <handUp>
	arVector3 upHolder = arVector3(0,1,0);
	arVector3 handUp = arVector3(handUpX, handUpY, handUpZ);
	double angleHolder = acos(dotProduct(normalize(handUp), normalize(upHolder)));
	if( dotProduct(crossProduct(handUp, upHolder),arVector3(handDirX, handDirY, handDirZ)) > 0)
		wristRotation = angleHolder * 180.0 / PI;
	else 
		wristRotation = -angleHolder* 180.0 / PI;

	
	//set up contents of eventInformation data display (if it is visible)
	if(myDataDisplays.size() >= 3 && myDataDisplays[2].isVisible){
		myDataDisplays[2].contents.clear();
		myDataDisplays[2].contents.push_back(textItem("Event Information:"));
		myDataDisplays[2].contents.push_back(textItem(""));
		myDataDisplays[2].contents.push_back(textItem("Event Primary Type:"));
		if(currentDots.particleName.size() > 0){
			char buffer[100];
			sprintf(buffer, "%s : %.2f MeV", currentDots.particleName[0].c_str(), currentDots.energy[0]);
			myDataDisplays[2].contents.push_back(textItem(buffer, arVector3(1, 0, 0)));
		}
		myDataDisplays[2].contents.push_back(textItem(""));
		myDataDisplays[2].contents.push_back(textItem("Interacton Position:"));
		char buffer[100];
		sprintf(buffer, "  <%.2f, %.2f, %.2f> ", currentDots.vertexPosition[0], currentDots.vertexPosition[1], currentDots.vertexPosition[2]);
		myDataDisplays[2].contents.push_back(textItem(buffer, arVector3(1, 0, 0)));
		myDataDisplays[2].contents.push_back(textItem(""));
		myDataDisplays[2].contents.push_back(textItem("Primary Particle Direction:"));
		if(currentDots.coneDirection.size() > 0){
			char buffer[100];
			sprintf(buffer, "  <%.2f, %.2f, %.2f> ", currentDots.coneDirection[0][0], currentDots.coneDirection[0][1], currentDots.coneDirection[0][2]);
			myDataDisplays[2].contents.push_back(textItem(buffer, arVector3(1, 0, 0)));
		}
		
		myDataDisplays[2].contents.push_back(textItem(""));
		myDataDisplays[2].contents.push_back(textItem("Other Final State Particles:"));
		for(int i = 1; i < currentDots.particleName.size(); i++){
			char buffer[100];
			if(currentDots.doDisplay[i])
				sprintf(buffer, " -%s Cone %i : %.1f MeV : On", currentDots.particleName[i].c_str(), i + 1, currentDots.energy[i]);
			else
				sprintf(buffer, " -%s Cone %i : %.1f MeV : Off", currentDots.particleName[i].c_str(), i + 1, currentDots.energy[i]);
			myDataDisplays[2].contents.push_back(textItem( buffer));
		}
		
		//for rest of cones just have one line for each
		
	}	
	
	//regenerate the contents of the conesMenu
	conesMenu.myItems.clear();
	menuItem goBack;
	goBack.index = -2;
	goBack.numLines = 2;		
	goBack.content[0] = "Main";
	goBack.content[1] = "Menu";
	goBack.startOffSetX = 0;
	goBack.startOffSetY = 100;
	goBack.scale = 2;
	(goBack.onClick) = &mainMenuFunc;
	conesMenu.myItems.push_back(goBack);
	
	for(int i = 0; i < dotVectors[index].particleName.size(); i++){
		char * line1 = new char[100];	
		if(dotVectors[index].doDisplay[i])
			sprintf(line1, "Cone %i : On", i+1);
		else
			sprintf(line1, "Cone %i : Off", i+1);
		char * line2 = new char[100];
		sprintf(line2, "%s", dotVectors[index].particleName[i].c_str());
		char * line3 = new char[100];
		sprintf(line3, "%.1f MeV", dotVectors[index].energy[i]);
		//push into menu structure

			
		needsReleasing.push_back(line1);
		needsReleasing.push_back(line2);
		needsReleasing.push_back(line3);
		
		menuItem coneItem;
		coneItem.index = i-1;
		coneItem.numLines = 3;
		coneItem.startOffSetX = -125;
		coneItem.startOffSetY = 200;
		coneItem.scale = 1;
		coneItem.content[0] = line1;
		coneItem.content[1] = line2;
		coneItem.content[2] = line3;
		coneItem.onClick = &toggleCherenkovCone;
		conesMenu.myItems.push_back(coneItem);
	}
	mainMenu.children[2] = conesMenu;

//and now just in case use the menu id system ot make sure currentMenu is the right version of anything we've dynamically modified
	if(currentMenu.id == conesMenu.id){
		currentMenu = conesMenu;
	}
	
	/*
	for(int i = 0; i < currentMenu.myItems.size(); i++){
		cout << currentMenu.myItems[i].scale << "\n";
	}//*/
	
}

void display( arMasterSlaveFramework& fw ) {
	debugText("started display");
	// Load the navigation matrix.
	fw.loadNavMatrix();
	// Draw effector
	theEffector.draw(fw);
	currentDots = dotVectors[index];
	//draws cylinder
	drawScene(fw);
	
	
	//draws data displays
	for(int i = 0; i < myDataDisplays.size(); i++){
		myDataDisplays[i].drawDataDisplay(fw);
	}
	if(doMenu){
		doInterface(fw);
	}
	//draw data displays (if visible)
	debugText("finished display");
}


// Catch key events.
void keypress( arMasterSlaveFramework& fw, arGUIKeyInfo* keyInfo ) {
	cout << "Key ascii value = " << keyInfo->getKey() << endl;
	cout << "Key ctrl  value = " << keyInfo->getCtrl() << endl;
	cout << "Key alt   value = " << keyInfo->getAlt() << endl;
	string stateString;
	arGUIState state = keyInfo->getState();
	if (state == AR_KEY_DOWN) {
		stateString = "DOWN";
		if(keyInfo->getKey() == 107){
			if(isTouchingVertex){
				isGrabbingVertex = true;
			}
		}
	} else if (state == AR_KEY_UP) {
		if(keyInfo->getKey() == 112){  //p
			autoPlay = 1;
		}
		if(keyInfo->getKey() == 111){  //o
			doWristBoundTime = !doWristBoundTime;
		}
		if(keyInfo->getKey() == 98){  //pressing "n" will load the next event until there are no more, at which point the program will close
			//   loadNextEvent();
			 if(doMenu){
			   if(!(menuIndex < -1)){
				   menuIndex--;
			   } else {
				   menuIndex = 2;
			   }
		   }
		   else{
            if(!doTimeCompressed){
			      if(index > 0){
				      index--;
			      }
			      autoPlay = 0;
            } else {
              if(autoPlay == 0){
		              autoPlay = -1;
	              }
	              else if(autoPlay == 1){
		              autoPlay = 0;
	            }
			 }
		 }
		}
		if(keyInfo->getKey() == 110){
			if(doMenu){
				if(!(menuIndex > 1)){
					menuIndex++;
				} else {
					menuIndex = -2;
				}
			}
			else{
				if(!doTimeCompressed){
					if(index < dotVectors.size() - 1){
						index++;
					}
					autoPlay = 0;
				}else{
					if(autoPlay == 0){
						autoPlay = 1;
					}
					else if(autoPlay == -1){
						autoPlay = 0;
					}
				}
			}
		}
		if(keyInfo->getKey() == 111){
			doCylinderDivider = !doCylinderDivider;
		}
		if(keyInfo->getKey() == 107){
			isGrabbingVertex = false;
			if(!doMenu){
				doMenu = true;
			}else{  //the menu is on, here write code to toggle menu items
				if(doMainMenu){ //main menu
					if(menuIndex == -2){
						if(index > 0){
							index--;
						}
						autoPlay = 0;
					}
					if(menuIndex == -1){
						doOptionsMenu = true;
						doMainMenu = false;
					}
					if(menuIndex == 0){
						doMenu = false;
					}
					if(menuIndex == 1){
						doCherenkovConeMenu = true;
						doMainMenu = false;
					}
					if(menuIndex == 2){
						if(index < dotVectors.size() - 1){
							index++;
						}
						autoPlay = 0;
					}
				}
				else if(doOptionsMenu){
					if(menuIndex == -2){
						doOptionsMenu = false;
						doMainMenu = true;
					}
					if(menuIndex == -1){
						doCylinderDivider = !doCylinderDivider;
					}
					if(menuIndex == 0){
						doCherenkovCone = !doCherenkovCone;
					}
					if(menuIndex == 1){
						colorByCharge = !colorByCharge;
					}
					if(menuIndex == 2){
						doScaleByCharge = !doScaleByCharge;
					}
				}
				else if(doCherenkovConeMenu){  //
					if(menuIndex == -2){
						if(cherenkovConeMenuIndex == 0){
							doCherenkovConeMenu = false;
							doMainMenu = true;
						}
						else{
							cherenkovConeMenuIndex--;
						}
					}
					if(menuIndex == -1){
						if((cherenkovConeMenuIndex * 3 + 0) < currentDots.particleType.size()){
							dotVectors[index].doDisplay[cherenkovConeMenuIndex*3 + 0] = !currentDots.doDisplay[cherenkovConeMenuIndex*3 + 0];
							modifiedCherenkovConeIndex = cherenkovConeMenuIndex*3 + 0;
						}
					}
					if(menuIndex == 0){
						if((cherenkovConeMenuIndex * 3 + 1) < currentDots.particleType.size()){
							dotVectors[index].doDisplay[cherenkovConeMenuIndex*3 + 1] = !currentDots.doDisplay[cherenkovConeMenuIndex*3 + 1];
							modifiedCherenkovConeIndex = cherenkovConeMenuIndex*3 + 1;
						}
					}
					if(menuIndex == 1){
						if((cherenkovConeMenuIndex * 3 + 2) < currentDots.particleType.size()){
							dotVectors[index].doDisplay[cherenkovConeMenuIndex*3 + 2] = !currentDots.doDisplay[cherenkovConeMenuIndex*3 + 2];
							modifiedCherenkovConeIndex = cherenkovConeMenuIndex*3 + 2;
						}
					}
					if(menuIndex == 2){
						if(cherenkovConeMenuIndex <= (currentDots.particleType.size()-1) / 3){
							cherenkovConeMenuIndex++;
						}
					}
				}

			}
		}
		stateString = "UP";
	} else if (state == AR_KEY_REPEAT) {
		stateString = "REPEAT";
	} else {
		stateString = "UNKNOWN";
	}
	cout << "Key state = " << stateString << endl;
}


void logStuffs(arMasterSlaveFramework& fw){
	//export logfile
		ofstream myfile;
		myfile.open ("X:/Dive/szg_mingw/logs/logfile_super-KAVE.txt");
		for(int i = 0; i < dotVectors.size(); i++){
			myfile << "Event " << i << ": \n";
			vector<arVector3> dirs;
			for(int j = 0; j < dotVectors[i].coneDirection.size(); j++){
				dirs.push_back(dotVectors[i].coneDirection[j]);
			}
			myfile << "Pos: <" << dotVectors[i].vertexPosition[0] << ", " << dotVectors[i].vertexPosition[1] << ", " << dotVectors[i].vertexPosition[2] << "> \n";
			for(int j = 0; j < dirs.size(); j++){
				myfile << "Dir with type " << dotVectors[i].particleType[j] << ": <" << dirs[j][0] << ", " << dirs[j][1] << ", " << dirs[j][2] << "> \n";
			}
			myfile << "\n";
		}
		myfile.close();
}

// This is how we have to catch reshape events now, still
// dealing with the fallout from the GLUT->arGUI conversion.
// Note that the behavior implemented below is the default.
void windowEvent( arMasterSlaveFramework& fw, arGUIWindowInfo* winInfo ) {
	// The values are defined in src/graphics/arGUIDefines.h.
	// arGUIWindowInfo is in arGUIInfo.h
	// The window manager is in arGUIWindowManager.h
	if (winInfo->getState() == AR_WINDOW_RESIZE) {
		const int windowID = winInfo->getWindowID();
#ifdef UNUSED
		const int x = winInfo->getPosX();
		const int y = winInfo->getPosY();
#endif
		const int width = winInfo->getSizeX();
		const int height = winInfo->getSizeY();
		fw.getWindowManager()->setWindowViewport( windowID, 0, 0, width, height );
	}
}

int main(int argc, char** argv) {
#ifndef ICECUBE
	filename = "temp";
	if(argc > 1){
		filename = argv[1];
	}
	if(argc > 2){
		cout << argv[2];
		cout << "\n";
		if(argv[2][0] == '-' && argv[2][1] == 't' && argv[2][2] == 'c'){
			doTimeCompressed = true;
		}
	}

   if(doTimeCompressed){
      colorByCharge = true;
   }
	arMasterSlaveFramework framework;
	// Tell the framework what units we're using.
	framework.setUnitConversion(FEET_TO_LOCAL_UNITS);
	framework.setClipPlanes(nearClipDistance, farClipDistance);
	framework.setStartCallback(start);
	framework.setWindowStartGLCallback( windowStartGL );
	framework.setPreExchangeCallback(preExchange);
	framework.setPostExchangeCallback(postExchange);
	framework.setDrawCallback(display);
	framework.setKeyboardCallback( keypress );
	framework.setWindowEventCallback( windowEvent );
	framework.setEyeSpacing( 0.00001f );
	framework.setExitCallback( logStuffs );
	// also setExitCallback(), setUserMessageCallback()
	// in demo/arMasterSlaveFramework.
	

	if (!framework.init(argc, argv)) {
		return 1;
	}
#else
	IceCubeFramework framework;
	// Tell the framework what units we're using.
	//framework.setUnitConversion(FEET_TO_LOCAL_UNITS);
	//framework.setClipPlanes(nearClipDistance, 10000.f);//farClipDistance);

	if (!framework.init(argc, argv)) {
		return 1;
	}

#endif

	// Never returns unless something goes wrong
	framework.start();	
}

