//********************************************************
// Syzygy is licensed under the BSD license v2
// see the file SZG_CREDITS for details
//********************************************************

//  -Ben Izatt
// bji@berkeley.edu

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

#ifdef WINNEUTRINO
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

class dataDisplay{ 
public:
	arVector3 myPos;  //location of very center
	arMatrix4 myRotationMatrix;  //location outward of screen
	vector<string> contents;  //contents, to be drawn from top to bottom, line by line
	void drawDataDisplay(arMasterSlaveFramework& fw);
};


//Global variables
bool debug = false;  //enables some useful print statements
//menu variables
bool doHUD = false;  //outdated
bool doMenu = true;  //whether or not the menu is currently displayed
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
//string bufferLine;  

//PRE_PICKED COLOR ARRAYS
int red_values [] =		{132	,74,	22,		4,		4,		6,		40,		115,	193,	249,	253,	249,	219,	219,	219,	219};  //r
int green_values [] =	{4		,12,	4,		124,	175,	184,	183,	114,	193,	249,	213,	191,	143,	129,	102,	33};    //g
int blue_values [] =	{186	,178,	186,	186,	186,	86,		7,		0,		6,		21,		80,		1,		12,		12,		12,		12};   //b



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

	//figure out if we're touching the display
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
	bool isTouching = false;
	if(magnitude(subtract(handPos, myPos)) < 2.5){  //later do check with intersection of box
		isTouching = true;
		if(fw.getButton(5)){ //is grabbing
			//update location based on hand movement
			myPos = handPos;
			myHandMatrix[12] = -2*handDirX;
			myHandMatrix[13] = -2*handDirY;
			myHandMatrix[14] = -2*handDirZ;
			myHandMatrix[8] = -handUpX;
			myHandMatrix[9] = -handUpY;
			myHandMatrix[10] = -handUpZ;
			myHandMatrix[4] = handDirX;
			myHandMatrix[5] = handDirY;
			myHandMatrix[6] = handDirZ;
			
			myRotationMatrix = myHandMatrix;
		}
	} else{
		isTouching = false;
	}
		
	glPushMatrix();
	
		
		glTranslatef(myPos[0], myPos[1], myPos[2]);
		
		/*
		arVector3 myRotationAxis = crossProduct(myDir, arVector3(0,0,-1));
		float myRotationAngle = acos(dotProduct(myDir, arVector3(0,0,-1)));	
		glRotatef(myRotationAngle*180.0/PI, myRotationAxis[0], myRotationAxis[1], myRotationAxis[2]);
		
		
		
		//now rotate to get up aligned
		myRotationAxis = arVector3(0,0,-1);
		cout << myDir[0] << " " << myDir[1] << " " << myDir[2] << "\n";
		myRotationAngle = acos(myDir[1]);
		if(myDir[1] < .5 || myDir[1] > -.5){
			myRotationAngle = asin(myDir[0]);
		}
		glRotatef(myRotationAngle*180.0/PI, myRotationAxis[0], myRotationAxis[1], myRotationAxis[2]);
		*/
		
		glRotatef(90,-1,0,0);
		glMultMatrixf(myRotationMatrix.v);
		glRotatef(90,-1,0,0);
		
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
			string myString = contents[i];
			glPushMatrix();
			glScalef(.0004,.0004,.0004);  
			for(int j = 0; j < myString.length(); j++){
				char myChar = myString.at(j);
				glutStrokeCharacter(GLUT_STROKE_ROMAN, myChar);
			}
			glPopMatrix();
			glTranslatef(0,-.05,0);
		}
		glColor3f(1,1,1);
		glutSolidCube(.1);
	glPopMatrix();
}


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
		/*
		if(value > 1125) section = 16;
		else if (value > 1110) section = 14;
		else if (value > 1095) section = 13;
		else if (value > 1080) section = 12;
		else if (value > 1065) section = 11;
		else if (value > 1050) section = 10;
		else if (value > 1035) section = 9;
		else if (value > 1020) section = 8;
		else if (value > 1005) section = 7;
		else if (value > 990) section = 6;
		else if (value > 975) section = 5;
		else if (value > 960) section = 4;
		else if (value > 945) section = 3;
		else if (value > 930) section = 2;
		else if (value > 915) section = 1;
		*/
		/*
		if(value < 1001) section = 16;
		else if (value < 1012) section = 14;
		else if (value < 1023) section = 13;
		else if (value < 1034) section = 12;
		else if (value < 1045) section = 11;
		else if (value < 1056) section = 10;
		else if (value < 1067) section = 9;
		else if (value < 1078) section = 8;
		else if (value < 1089) section = 7;
		else if (value < 1100) section = 6;
		else if (value < 1111) section = 5;
		else if (value < 1122) section = 4;
		else if (value < 1133) section = 3;
		else if (value < 1144) section = 2;
		else if (value < 1155) section = 1;
		*/
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
		/*    for(j = 0.0; j < 2 * PI; j += PI / 180.0 * 22.5) {
		newx = cx + DOTRAD * sin(j);
		newy = cy + DOTRAD * cos(j);
		glVertex3f(cx,cy,cz);
		glVertex3f(newx, newy, cz);
		}    
		*/
		glPushMatrix();
		glTranslatef(cx,cy,cz);
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
		gluDisk(quadObj,0,radius,subdivisions,1);
		glPopMatrix();
	}
	else {
		float theta = atan2(cy,cx);
		/*
		for(j = 0.0; j < 2 * PI; j += PI / 180.0 * 22.5) {
		newx = cx + sin(theta) * DOTRAD * cos(j);
		newy = cy - cos(theta) * DOTRAD * cos(j);
		newz = cz + DOTRAD * sin(j);
		glVertex3f(cx,cy,cz);
		glVertex3f(newx, newy, newz);
		}
		*/
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
	
	//draws data displays
	cout << myDataDisplays.size();
	for(int i = 0; i < myDataDisplays.size(); i++){
		myDataDisplays[i].drawDataDisplay(framework);
	}
	
	
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
						printf(me,"%i",(int)currentDots.particleType[i]);
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
	}

	index = 0;


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
void drawDisplay(int index, bool highlighted, char * content[10],int numLines, int startOffSetX, int startOffSetY, float scale){
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

void doInterface(arMasterSlaveFramework& framework){\
	// framework.setEyeSpacing( 0.0f );
	/*
	if(doHUD){ 
		//if we're doing the HUD (which is now disabled and not accessible from in program), we'll do the matrix pushing here.
		glMultMatrixf(ar_getNavMatrix().v);
		glMultMatrixf(framework.getMatrix(0).v);
	}
	*/
	
	bool state = false;  //this is used to determine where the green box goes
	char * content[10];  //no more than 10 items per line.  Gets illegible if we try to get too much smaller
	int indexVal;
	if(doOptionsMenu){  //hardcoded menus.
		state = updateMenuIndexState(-2);  //each of these updateMenuIndexState lines is checking if this is currently the selected item.  If it is, this becomes true, and passes in to the drawDisplay function which adds a green box to it.  Honestly there's no reason I couldn't have just done this from within drawDisplay entirely, but it isn't computationally expensive anyhow.  TODO, make that smarter.
		content[0] = "Main";
		content[1] = "Menu";
		drawDisplay(-2,state, content ,2,0,50, 2);

		state = updateMenuIndexState(-1);
		content[0] = "Toggle";
		content[1] = "Outer";
		content[2] = "Detector";
		drawDisplay(-1,state, content ,3,-35,150, 1.5);

		state = updateMenuIndexState(0);
		content[0] = " Toggle";
		content[1] = "Cherenkov";
		content[2] = "Radiation";
		drawDisplay(0,state, content ,3,-140,150, 1.5);

		state = updateMenuIndexState(1);
		content[0] = "  Q/T";
		content[1] = " Toggle";
		drawDisplay(1,state, content ,2,-300,100, 2);

		state = updateMenuIndexState(2);
		content[0] = "Scale";
		content[1] = "Radius";
		content[2] = "By Q";
		drawDisplay(2,state, content ,3,-75,200, 1.8);
	}
	if(doCherenkovConeMenu){
		state = updateMenuIndexState(-2);
		if(cherenkovConeMenuIndex == 0){
			content[0] = "Main";
			content[1] = "Menu";
			drawDisplay(-2,state, content ,2,0,50, 2);
		}
		else{
			content[0] = "Back";
			drawDisplay(-2,state,content,1,0,-50,2);
		}

		int numHold = 3;  //this is 3.  If we don't have anything to write on a display, set it to 0.  We know all the other ones after that will also be 0
		//special case, if there are 4 (or less) particles, we don't need a "more" button
		if(currentDots.particleType.size() <= 4){
			for(int l = -1; l<=2;l++){
				state = updateMenuIndexState(l);
				indexVal = cherenkovConeMenuIndex*3+(l+1);
				if(indexVal >= currentDots.particleType.size()){
					numHold = 0;
				}
				content[0] = (char*)currentDots.particleName[indexVal].c_str();
				char dest[50];
				sprintf(dest, "%i MeV", (int)currentDots.energy[indexVal]);
				content[1] = dest;
				if(currentDots.doDisplay[indexVal]){
					content[2] = "On";
				}else{
					content[2] = "Off";
				}
				drawDisplay(l,state, content ,numHold,-100,100, 1.2);
			}
		}
		else{
			for(int l = -1; l<=1;l++){
				state = updateMenuIndexState(l);
				indexVal = cherenkovConeMenuIndex*3+(l+1);
				if(indexVal >= currentDots.particleType.size()){
					numHold = 0;
				}
				content[0] = (char*)currentDots.particleName[indexVal].c_str();
				char dest[50];
				sprintf(dest, "~%i MeV", (int)currentDots.energy[indexVal]);
				content[1] = dest;
				if(currentDots.doDisplay[indexVal]){
					content[2] = "On";
				}else{
					content[2] = "Off";
				}
				drawDisplay(l,state, content ,numHold,-100,100, 1.1);
			}
			/*
			state = updateMenuIndexState(0);
			drawDisplay(0,state, content ,0,0,0, 1);

			state = updateMenuIndexState(1);
			drawDisplay(1,state, content ,0,0,0, 1);
			*/

			state = updateMenuIndexState(2);
			content[0] = "Next";
			if(numHold == 3){
				drawDisplay(2,state, content ,1,0,0, 2);
			}else{
				drawDisplay(2,state,content,0,0,0,2);
			}
		}
	}
	if(doMainMenu){  //main menu
		state = updateMenuIndexState(-2);
		content[0] = " -";
		content[1] = "Event";
		content[2] = " -";
		drawDisplay(-2,state,content,3,0,200,2);

		state = updateMenuIndexState(-1);
		content[0] = "Options";
		drawDisplay(-1,state,content,1,-125,-75,2);

		state = updateMenuIndexState(0);
		content[0] = "Exit";
		content[1] = "Menu";
		drawDisplay(0,state, content ,2,0,100,2);

		state = updateMenuIndexState(1);
		content[0] = "Cherenkov";
		content[1] = " Cones";
		drawDisplay(1,state,content,2,-130,50, 1.5);

		state = updateMenuIndexState(2);
		content[0] = " +";
		content[1] = "Event";
		content[2] = " +";
		drawDisplay(2,state,content,3,0,200, 2);
	}
	glLineWidth(1.0);
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

	//now for individual displays, again hardcoded for the moment.  Sorry again.
	if(doMenu){
		if(doHUD){
			glPushMatrix();
			doInterface(framework);
			glPopMatrix();
		}else{
			glPushMatrix();
			glMultMatrixf( getCenterMatrix().v );
			doInterface(framework);
			glPopMatrix();
		}	
	}
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
	dataDisplay testDataDisplay;
	testDataDisplay.contents.push_back("Test string one");
	testDataDisplay.contents.push_back("Test string two");
	testDataDisplay.myPos = arVector3(0,0,0);
	testDataDisplay.myRotationMatrix = arMatrix4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	myDataDisplays.push_back(testDataDisplay);
	
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

		//handle autoplay end-stopping (For supernova mode)
		if(index == 0 || index == dotVectors.size()-1){
			autoPlay = 0;
		}

		//button mapping for reference:
		// 0 = yellow
		// 1 = red
		// 2 = green
		// 3 = blue
		// 4 = joystick press
		// 5 = back button (master)

		//do button presses
		if(fw.getOnButton(0)){  // on yellow button, step event back one if menus aren't up, step menu index back one if menus are up...replace stepping with autoplay if we're doing time compression
			if(doMenu){
				if(!(menuIndex < -1)){
					menuIndex--;
				} else {
					menuIndex = 2;
				}
			}
			else{
				doMenu = true;
				/*
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
				*/
			}
		}
		if(fw.getOnButton(1)){  //on red button, spawn another menu  // PROBLEM:  if more than one is grabbed, they overlap and are unfixable
			dataDisplay testDataDisplay;
			testDataDisplay.contents.push_back("Test string one");
			testDataDisplay.contents.push_back("Test string two");
			testDataDisplay.myPos = arVector3(0,0,0);
			testDataDisplay.myRotationMatrix = arMatrix4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
			myDataDisplays.push_back(testDataDisplay);
		}
		if(fw.getOnButton(2)){  // on green button, step event forward one, or if menus are up, step menuIndex forwar done  .. or, if time compressed, autoplay forward
			if(doMenu){
				if(!(menuIndex > 1)){
					menuIndex++;
				} else {
					menuIndex = -2;
				}
			}
			else{
				doMenu = true;
				/*
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
				*/
			}
			
		}
		if(fw.getOnButton(3)){ // on blue button, do nothing
		}
		if(fw.getOnButton(4)){  //on joystick button press (todo:  joystick compressed + left/right will autoplay)
		}
		if(!fw.getButton(5)){
			isGrabbingVertex = false;
		}
		if(fw.getButton(5) && isTouchingVertex){
			isGrabbingVertex = true;
			doMenu = false;
		}
		else
		if(fw.getOnButton(5)){  //this is the master control button.  It turns on the menu, and selects menu items
			/*if(!doMenu && !isTouchingVertex){
				doMenu = true;
			}else */
			if(doMenu && !isTouchingVertex){  //the menu is on, here write code to toggle menu items
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

		/*
		//update menu index if we're in HUD Mode, based on hand orientation from z axis
		if(doHUD){
			//we need to figure out the orientation from the rotation matrix
			for(int i =0; i < 4;i++){
				for(int j = 0; j < 4; j++){
					cout << fw.getMatrix(1).v[j+i*4];
					cout << " ";
				}
				cout <<"\n";
			}
			cout << "\n";
		}*/
		
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
			

		triggerDepressed = fw.getButton(5);

		//in load stage we'll go ahead and write the START TIMES and END TIMES of all events, and precalculate the EVENT LENGTH
		//which multiplied by TimeScaleFactor will be the waitTime for each frame

		/* //this will autoplay forward / back based on joystick left/right.  No longer active
		if((currentTime - timeHolder1) > currentDots.length*timeScaleFactor){
		//tests joystick state along axis 0 (left/right)
		if(fw.getAxis(0) > threshold){
		autoPlay = 0;
		if(index < dotVectors.size() - 1){
		index++;
		timeHolder1 = currentTime;
		}
		}
		if(fw.getAxis(0) < -1.0*threshold){
		autoPlay = 0;
		if(index > 0){
		index--;
		timeHolder1 = currentTime;
		}
		}
		}
		*/

		//more autoplay nonsense
		if(autoPlay == -1 && index >= dotVectors.size()-1){
			index = dotVectors.size()-2;
		}
		if(currentTime - timeHolder1 > currentDots.length*timeScaleFactor){
			if(autoPlay == -1){
				if(index > 0){
					index--;
					timeHolder1 = currentTime;
				}
			}
			else if(autoPlay == 1){
				if(index < dotVectors.size() - 1){
					index++;
					timeHolder1 = currentTime;
				}
			}
		}
		
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
		//everything happens automatically except for the computation based on modifiedCherenkovConeIndex
		if(modifiedCherenkovConeIndex != -1 && modifiedCherenkovConeIndex < currentDots.doDisplay.size()){
			dotVectors[index].doDisplay[modifiedCherenkovConeIndex] = !currentDots.doDisplay[modifiedCherenkovConeIndex];
		}
	}
	//handle vertex motion
	if(isGrabbingVertex){
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
		
		double finalX = -navX + handPosX + -2*handDirX;
		double finalY = -navY + handPosZ + -2*handDirZ;
		double finalZ = navZ + -handPosY + 2*handDirY;
			
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
			autoPlay = -1;
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
	// also setExitCallback(), setUserMessageCallback()
	// in demo/arMasterSlaveFramework.h

	/*				test code for generating clock times
	clock_t startt = clock();
	while(true){
	clock_t timed = clock();
	printf("%f",(timed-startt)/(double)CLOCKS_PER_SEC);
	printf("\n");
	}
	*/

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
	return framework.start() ? 0 : 1;
}

