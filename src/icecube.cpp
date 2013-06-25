//********************************************************
// Syzygy is licensed under the BSD license v2
// see the file SZG_CREDITS for details
//********************************************************

// precompiled header include MUST appear as the first non-comment line
#include "arPrecompiled.h"
// MUST come before other szg includes. See arCallingConventions.h for details.
#define SZG_DO_NOT_EXPORT

#include "icecube.h"

#include "arInteractionUtilities.h"

#include <stdio.h>
#include <stdlib.h>

#include "arGlut.h"

#include "icecubeGeometryInput.h"
#include "icecubeDataInput.h"

#include "glslUtils.h"

// OOPified skeleton.cpp. Subclasses arMasterSlaveFramework and overrides its
// on...() methods (as opposed to installing callback functions).

// Unit conversions.  Tracker (and cube screen descriptions) use feet.
// Atlantis, for example, uses 1/2-millimeters, so the appropriate conversion
// factor is 12*2.54*20.
const float FEET_TO_LOCAL_UNITS = 1.; //Feet to meters

// Near & far clipping planes.
const float nearClipDistance = .1*FEET_TO_LOCAL_UNITS;
const float farClipDistance = 100.*FEET_TO_LOCAL_UNITS;
  

GeometryInput geometryData;
DataInput event2Data;
//GLUquadricObj * qObj; 

float fDownScale = 100.f;

//int startTime = 999999;
//int timeCounter = 0;
float speedAdjuster = 0.0f;
//int endTime = 0;
int timeSpan = 0;
int expansionTime = 0;
bool playForward = true;
float largestCharge = 0;
float smallestCharge = 999999;
float chargeSpan = 0;
struct color{
float red, green, blue;
};

tdMenuController ct = tdMenuController();

vector<color> eventColors;








/////////////////////////////////////Sphere drawing algorithm////////////////////////////////////////////////


#define X .525731112119133606 
#define Z .850650808352039932

static GLfloat vdata[12][3] = {    
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
    {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
    {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};
static GLuint tindices[20][3] = { 
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
    {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
    {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
    {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

void normalize(GLfloat *a) {
    GLfloat d=sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
    a[0]/=d; a[1]/=d; a[2]/=d;
}

void drawtri(GLfloat *a, GLfloat *b, GLfloat *c, int div, float r) {
    if (div<=0) {
        glNormal3fv(a); glVertex3f(a[0]*r, a[1]*r, a[2]*r);
        glNormal3fv(b); glVertex3f(b[0]*r, b[1]*r, b[2]*r);
        glNormal3fv(c); glVertex3f(c[0]*r, c[1]*r, c[2]*r);
    } else {
        GLfloat ab[3], ac[3], bc[3];
        for (int i=0;i<3;i++) {
            ab[i]=(a[i]+b[i])/2;
            ac[i]=(a[i]+c[i])/2;
            bc[i]=(b[i]+c[i])/2;
        }
        normalize(ab); normalize(ac); normalize(bc);
        drawtri(a, ab, ac, div-1, r);
        //drawtri(b, bc, ab, div-1, r);
        drawtri(c, ac, bc, div-1, r);
        //drawtri(ab, bc, ac, div-1, r);  //<--Comment this line and sphere looks really cool!
    }  
}

void drawsphere(int ndiv, float radius=1.0) {
    glBegin(GL_TRIANGLES);
    for (int i=0;i<20;i++)
        drawtri(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], ndiv, radius);
    glEnd();
}


//////////////////////////////End sphere drawing algorithm//////////////////////////////











void findExtremeEventTimes(){
	//Find extreme times
	largestCharge = 0;
	smallestCharge = 999999;
	chargeSpan = 0;
	for(int i=0; i < event2Data.icecubeData.xCoord.size(); i++){
		if(event2Data.icecubeData.time[i] > ct.vars.time->end){
			ct.vars.time->end = (int)(event2Data.icecubeData.time[i]);
		}
		else if(event2Data.icecubeData.time[i] < ct.vars.time->start){
			ct.vars.time->start = (int)(event2Data.icecubeData.time[i]);
		}
		if(event2Data.icecubeData.charge[i] > largestCharge){
			largestCharge = event2Data.icecubeData.charge[i];
		}
		else if(event2Data.icecubeData.charge[i] < smallestCharge){
			smallestCharge = event2Data.icecubeData.charge[i];
		}
	}
	chargeSpan = largestCharge - smallestCharge;
	timeSpan = ct.vars.time->end - ct.vars.time->start;
	expansionTime = timeSpan/(event2Data.icecubeData.xCoord.size());
	ct.vars.time->end += expansionTime;
	if(timeSpan < 0.1){timeSpan = 1;}
	ct.vars.time->val = ct.vars.time->start;
	
	color red;
	for(int i=0; i < event2Data.icecubeData.xCoord.size(); i++){
		red.red=0.5f;red.green=0.0f;red.blue=1.0f;
		
		if(event2Data.icecubeData.time[i] <= ct.vars.time->start+timeSpan/4){
			red.red=1.0f;red.green=4*(event2Data.icecubeData.time[i]-ct.vars.time->start)/timeSpan;red.blue=0.0f;
		}
		if(event2Data.icecubeData.time[i] > ct.vars.time->start+timeSpan/4){
			red.red=1-(event2Data.icecubeData.time[i]-(ct.vars.time->start + timeSpan/4))/(timeSpan/4);red.green=1.0f;red.blue=0.0f;
		}
		if(event2Data.icecubeData.time[i] > ct.vars.time->start+timeSpan/2){
			float y = (event2Data.icecubeData.time[i]-(ct.vars.time->start + timeSpan/2))/(timeSpan/4);;
			red.red=0.0f;red.green=1.0f-y;red.blue=y;
		}
		if(event2Data.icecubeData.time[i] > ct.vars.time->start+0.75*timeSpan){
			red.red=(event2Data.icecubeData.time[i]-(ct.vars.time->start + 0.75*timeSpan))/(timeSpan/3);red.green=0;red.blue=1.0f;
		}
		eventColors.push_back(red);

	}
}

  
void ColoredSquareIce::draw( arMasterSlaveFramework* fw ) {

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);

	//glScalef(3.281f, 3.281f, 3.281f);

	arMatrix4 userPosition = ar_getNavMatrix();  //userPosition[0] is the rotation of the user
	//cout << "userPosition (x,y,z) = (" << userPosition[12] << ", " << userPosition[14] << ", " << userPosition[13] << ")" << endl; //12 (initial x direction), 13 (vertical), 14 (initial z direction) to get user's position
  //fw->getMatrix(0); //12 (some 2D movement), 13 (vertical), 14 to get user's position
	//fw->getMatrix(0);
	//cout << "uP[0] = " << uP[0] << endl;

  glPushMatrix();
   
    glMultMatrixf(getMatrix().v );
    // set one of two colors depending on if this object has been selected for interaction
    if (getHighlight()) {
      glColor3f( 0,1,0 );
    } else {
     glColor3f( 1,1,0 );
    }
    // draw a 2-ft upright square
    glBegin( GL_QUADS );	
      glVertex3f( -1,-1,0 );
      glVertex3f( -1,1,0 );
      glVertex3f( 1,1,0 );
      glVertex3f( 1,-1,0 );
    glEnd();
	glPopMatrix();
	glPushMatrix();
	
	glTranslatef(0,20,0);
	glRotatef(90, 1.0, 0.0, 0.0);  //rotate to set cylinder up/down

	glScalef(3.281f, 3.281f, 3.281f);

	glBegin( GL_LINES );	
		glColor3f(1.0f, 0.0f, 0.0f);
      glVertex3f( 0,0,0 );
      glVertex3f( 0,0,100 );
	  glColor3f(0.0f, 0.0f, 1.0f);
      glVertex3f( 0,0,0 );
      glVertex3f( 100,0,0 );
	  glColor3f(1.0f, 1.0f, 0.0f);
      glVertex3f( 0,0,0 );
      glVertex3f( 0,100,0 );
    glEnd();

	
	//drawsphere(1, 1.0f);

	float scaleDownEventSphere = fDownScale/10;

	int numDrawn = 0;
	
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable (GL_DEPTH_BUFFER);
	//glDisable (GL_DEPTH_TEST);
	float chargeRadiusFactor = 1.f;

	float sphereX, sphereY, sphereZ;
	float diffX, diffY, diffZ;

	//DisplaySphere(5, false, 0);
	//Draws the event data
	for(int i=0; i < event2Data.icecubeData.xCoord.size(); i++){
		if(ct.vars.time->val > event2Data.icecubeData.time[i]){
			//Transparency changing with amount of time event has been drawn
			//glColor4f(eventColors.at(i).red, eventColors.at(i).green, eventColors.at(i).blue, event2Data.icecubeData.time[i]/ct.vars.time->val);
			chargeRadiusFactor = log(143*(event2Data.icecubeData.charge[i] - smallestCharge)/chargeSpan + 7);
			glColor3f(eventColors.at(i).red, eventColors.at(i).green, eventColors.at(i).blue);   //, sin((ct.vars.time->val - event2Data.icecubeData.time[i])/50) + 1.24);    //a = 0.1 + (largestCharge - event2Data.icecubeData.charge[i])*(largestCharge - event2Data.icecubeData.charge[i])/(chargeSpan*chargeSpan));
			sphereX=event2Data.icecubeData.xCoord[i]/fDownScale;sphereY=event2Data.icecubeData.yCoord[i]/fDownScale;sphereZ=-event2Data.icecubeData.zCoord[i]/fDownScale;
			diffX = sphereX - userPosition[12]/3.281;diffY = sphereY - userPosition[14]/3.281;diffZ = 5 - sphereZ - userPosition[13]/3.281;
			//cout << "diffZ = " << diffZ << endl;
			if(diffX*diffX + diffY*diffY < 25 && diffZ*diffZ < 25){
				//cout << "diffZ = " << diffZ << endl;
				glTranslatef(sphereX, sphereY, sphereZ);
				//cout << "sphereX = " << event2Data.icecubeData.xCoord[i]/fDownScale << "; sphereY = " << event2Data.icecubeData.yCoord[i]/fDownScale << "; sphereZ = " << -event2Data.icecubeData.zCoord[i]/fDownScale << endl;
			
				//Expansion animation of event data
				//glRotatef(0.1*(ct.vars.time->val - event2Data.icecubeData.time[i]), 0, 0, 1);
				if(ct.vars.time->val-event2Data.icecubeData.time[i] < expansionTime){drawsphere(1, ((ct.vars.time->val-event2Data.icecubeData.time[i])/expansionTime)*(chargeRadiusFactor*0.25f/scaleDownEventSphere));}
				//if(ct.vars.time->val-event2Data.icecubeData.time[i] < expansionTime){drawsphere(1, ((ct.vars.time->val-event2Data.icecubeData.time[i])/expansionTime)*(chargeRadiusFactor*0.25f/scaleDownEventSphere));}
				else{drawsphere(1, chargeRadiusFactor*0.25f/scaleDownEventSphere);}	
				//glRotatef(-0.1*(ct.vars.time->val - event2Data.icecubeData.time[i]), 0, 0, 1);
				glTranslatef(-sphereX, -sphereY, -sphereZ);
			}
		}
	}

	glEnable (GL_DEPTH_BUFFER);
	glEnable (GL_DEPTH_TEST);
	glDisable (GL_BLEND);
	//glEnable (GL_DEPTH_BUFFER);
	//Time increments for forward and backward animation
	if(playForward){
		if(ct.vars.time->val < ct.vars.time->end){
			ct.vars.time->val+=10 - speedAdjuster;
		}
	}
	else{
		if(ct.vars.time->val > ct.vars.time->start){
			ct.vars.time->val-=10 - speedAdjuster;
		}
	}
	//cout << "time = " << ct.vars.time->val << endl;


	//gluQuadricDrawStyle(qObj, GLU_FILL);  //sets draw style of the cylinder to wireframe (GLU_LINE)
	glColor3f(1.0f, 0.25f, 0.25f);

	//inner cyliner:
	//gluCylinder(quadObj, RADIUS, RADIUS, HEIGHT, 6, 30); //inner cylinder wireframe  //now hexagonal
	//gluDisk(quadObj, 0.0, RADIUS, 6, 30);

	//glTranslatef(0,0,40);
	//gluDisk(quadObj, 0.0, RADIUS, 6, 30);
	//glTranslatef(0,0,-40);

	//float fDownScale = 10.f;//10.f;
	float scaleDownSphere = fDownScale/5;
	//fDownScale = 10.f;

	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(0.0f, 0.0f, -1920.f/fDownScale);
	glScalef(1.0f, 1.0f, 0.025f);
	glutSolidCube(2000.0f/fDownScale);
	glScalef(1.0f, 1.0f, 40.0f);
	glTranslatef(0.0f, 0.0f, 1920.f/fDownScale);

	//Draws the Icecube detector grid
	for(unsigned int i=0; i < geometryData.icecubeGeometry.xCoord.size(); i++){
		glColor3f(0.25f, 0.25f, 0.25f);
		sphereX=geometryData.icecubeGeometry.xCoord[i]/fDownScale;sphereY=geometryData.icecubeGeometry.yCoord[i]/fDownScale;sphereZ=-geometryData.icecubeGeometry.zCoord[i]/fDownScale;
		diffX = sphereX - userPosition[12]/3.281;diffY = sphereY - userPosition[14]/3.281;diffZ = 5 -sphereZ - userPosition[13]/3.281;
		if(diffX*diffX + diffY*diffY< 16 && diffZ*diffZ < 16){
			
			if(geometryData.icecubeGeometry.strings[i] > 78){glColor3f(1.0f, 1.0f, 1.0f);}
			if(geometryData.icecubeGeometry.modules[i] > 60){glColor3f(1.0f, 0.0f, 0.0f);
			glTranslatef(sphereX, sphereY, sphereZ);
			glutSolidSphere(0.55f/scaleDownSphere, 4, 4);
			glTranslatef(-sphereX, -sphereY, -sphereZ);
			}
			else{
			glTranslatef(sphereX, sphereY, sphereZ);
			glutSolidSphere(0.25f/scaleDownSphere, 4, 4);
			glTranslatef(-sphereX, -sphereY, -sphereZ);
			glBegin(GL_LINES);
			if(i < geometryData.icecubeGeometry.xCoord.size()-1 && geometryData.icecubeGeometry.strings[i] == geometryData.icecubeGeometry.strings[i+1]){
				glColor3f(0.75f, 0.75f, 0.75f);
				if(geometryData.icecubeGeometry.modules[i] == 60){
					glVertex3f(geometryData.icecubeGeometry.xCoord[i-59]/fDownScale, geometryData.icecubeGeometry.yCoord[i-59]/fDownScale, -geometryData.icecubeGeometry.zCoord[i-59]/fDownScale);
					glVertex3f(geometryData.icecubeGeometry.xCoord[i+1]/fDownScale, geometryData.icecubeGeometry.yCoord[i+1]/fDownScale, -geometryData.icecubeGeometry.zCoord[i+1]/fDownScale);
				}
				else{
					glVertex3f(geometryData.icecubeGeometry.xCoord[i]/fDownScale, geometryData.icecubeGeometry.yCoord[i]/fDownScale, -geometryData.icecubeGeometry.zCoord[i]/fDownScale);
					glVertex3f(geometryData.icecubeGeometry.xCoord[i+1]/fDownScale, geometryData.icecubeGeometry.yCoord[i+1]/fDownScale, -geometryData.icecubeGeometry.zCoord[i+1]/fDownScale);
				}
			}
			glEnd();
		
			}
		}
		else{
			glBegin(GL_LINES);
			if(i < geometryData.icecubeGeometry.xCoord.size()-1 && geometryData.icecubeGeometry.strings[i] == geometryData.icecubeGeometry.strings[i+1]){
				glColor3f(0.75f, 0.75f, 0.75f);
				if(geometryData.icecubeGeometry.modules[i] == 60){
					glVertex3f(geometryData.icecubeGeometry.xCoord[i-59]/fDownScale, geometryData.icecubeGeometry.yCoord[i-59]/fDownScale, -geometryData.icecubeGeometry.zCoord[i-59]/fDownScale);
					glVertex3f(geometryData.icecubeGeometry.xCoord[i+1]/fDownScale, geometryData.icecubeGeometry.yCoord[i+1]/fDownScale, -geometryData.icecubeGeometry.zCoord[i+1]/fDownScale);
				}
				else{
					glVertex3f(geometryData.icecubeGeometry.xCoord[i]/fDownScale, geometryData.icecubeGeometry.yCoord[i]/fDownScale, -geometryData.icecubeGeometry.zCoord[i]/fDownScale);
					glVertex3f(geometryData.icecubeGeometry.xCoord[i+1]/fDownScale, geometryData.icecubeGeometry.yCoord[i+1]/fDownScale, -geometryData.icecubeGeometry.zCoord[i+1]/fDownScale);
				}
			}
			glEnd();
		}
	}
  glPopMatrix();


 
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	 
	glLoadIdentity();
	gluOrtho2D(-5.0f, 5.0f, -5.0f, 5.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef(0.0, 4.8, -1.0);  //x=horizontal across screen; y=vertical across screen; z=does nothing important
	glColor3f(0.55f, 0.55f, 0.95f);
	glScalef(8.0, 0.1, 1.0);
	glutSolidCube(1.0f);
	glScalef(0.125, 10.0, 1.0);
	glTranslatef(0.0, -4.8, 1.0); 

	glTranslatef(4.0, 4.8, -0.5);
	glColor3f(0.15f, 0.15f, 0.55f);
	glRotatef(45.0f, 0, 0, 1);
	glutSolidCube(0.2f);
	glRotatef(-45.0f, 0, 0, 1);

	glTranslatef(-8.0, 0.0, 0.0);
	glRotatef(45.0f, 0, 0, 1);
	glutSolidCube(0.2f);
	glRotatef(-45.0f, 0, 0, 1);

	if(ct.vars.time->val > ct.vars.time->end){ct.vars.time->val = ct.vars.time->end;}
	if(ct.vars.time->val < ct.vars.time->start){ct.vars.time->val = ct.vars.time->start;}

	glTranslatef(8*float((float(ct.vars.time->val)-float(ct.vars.time->start))/float(timeSpan+expansionTime)) , 0.0, 0.5);
	glColor3f(0.55f, 0.15f, 0.15f);
	glRotatef(45.0f, 0, 0, 1);
	glutSolidCube(0.2f);
	glRotatef(-45.0f, 0, 0, 1);


  glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
}

void RodEffectorIce::draw() const {
  glPushMatrix();
  glMultMatrixf(getCenterMatrix().v);
    // draw grey rectangular solid 2"x2"x5'
    glScalef( 2./12, 2./12., 5. );
    glColor3f( .5,.5,.5 );
    glutSolidCube(1.);
    // superimpose slightly larger black wireframe (makes it easier to see shape)
    glColor3f(0,0,0);
    glutWireCube(1.03);
  glPopMatrix();
}

// End of classes

IceCubeFramework::IceCubeFramework() :
  arMasterSlaveFramework(),
  _squareHighlightedTransfer(0), m_shaderProgram(-1) {
	  arMasterSlaveFramework().setUnitConversion(FEET_TO_LOCAL_UNITS);
	  arTexture().readJPEG("", "", "", -1, true);
}


// onStart callback method (called in arMasterSlaveFramework::start()
//
// Note: DO NOT do OpenGL initialization here; this is now called
// __before__ window creation. Do it in the onWindowStartGL()
//
bool IceCubeFramework::onStart( arSZGClient& /*cli*/ ) {
  // Register shared memory.
  //  framework.addTransferField( char* name, void* address, arDataType type, int numElements ); e.g.
  addTransferField("squareHighlighted", &_squareHighlightedTransfer, AR_INT, 1);
  addTransferField("squareMatrix", _squareMatrixTransfer.v, AR_FLOAT, 16);

  // (re)initialize menu controller
  ct = tdMenuController(&_effector);
  //ENDJEDIT

  // Setup navigation, so we can drive around with the joystick
  //
  // Tilting the joystick by more than 20% along axis 1 (the vertical on ours) will cause
  // translation along Z (forwards/backwards). This is actually the default behavior, so this
  // line isn't necessary.
  setNavTransCondition( 'z', AR_EVENT_AXIS, 1, 0.2 );      

  // Tilting joystick left or right will rotate left/right around vertical axis (default is left/right
  // translation)
  setNavRotCondition( 'y', AR_EVENT_AXIS, 0, 0.2 );      

  // Set translation & rotation speeds to 5 ft/sec & 30 deg/sec (defaults)
  setNavTransSpeed( 5. * 100/fDownScale );    ////////Changes velocity of user
  setNavRotSpeed( 30. );
  
  // set square's initial position
  _square.setMatrix( ar_translationMatrix(0,5,-16) );

  

  /*const arMatrix4 ident;
  dsTransform( "sound scale", getNavNodeName(), ar_scaleMatrix(1) );
  int whaleSoundTransformID = dsTransform( "whale sound matrix", "sound scale", ident );
  int dolphinSoundTransformID = dsTransform( "dolphin sound matrix", "sound scale", ident );
  (void)dsLoop("whale song", "whale sound matrix", "..\\..\\src\\neutrinos\\data\\icecube\\sounds\\whale.mp3",
    1, 1.0, arVector3(0,0,0));*/
  //(void)dsLoop("dolphin song", "dolphin sound matrix", "dolphin.mp3",
  //  1, 0.05, arVector3(0,0,0));

  return true;
}

// Method to initialize each window (because now a Syzygy app can
// have more than one).
void IceCubeFramework::onWindowStartGL( arGUIWindowInfo* ) {

  GLenum err = glewInit();

  if(err != GLEW_OK)
  {
	  fprintf(stderr, "Error initializing GLEW!\n");
	  exit(1);
  }

  fprintf(stderr, "Glew Initialization Complete!\n");
  fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

  //std::string vshader = std::string("..\\..\\src\\neutrinos\\data\\icecube\\shader\\simple.vert");
  //std::string fshader = std::string("..\\..\\src\\neutrinos\\data\\icecube\\shader\\red.frag");
  std::string vshader = std::string("..\\..\\src\\neutrinos\\data\\icecube\\shader\\sphere_billboard2.vert");
  //std::string fshader = std::string("..\\..\\src\\neutrinos\\data\\icecube\\shader\\sphere_bwprint.frag");
  //std::string fshader = std::string("..\\..\\src\\neutrinos\\data\\icecube\\shader\\sphere_cheerio.frag");
  std::string fshader = std::string("..\\..\\src\\neutrinos\\data\\icecube\\shader\\sphere_glshovel2.frag");
  //std::string vshader = std::string(commonVShader());
  //std::string fshader = coinFShader(PLASTICY,PHONG,false);
  m_shaderProgram = loadProgramFiles(vshader,fshader,true);

  // OpenGL initialization
  glClearColor(0,0,0,0);

 // qObj = gluNewQuadric();

  ct.vars.time->start = 999999;ct.vars.time->end = 0;		
			eventColors.clear();
			event2Data.~DataInput();
  geometryData.getText();
  event2Data.getText("..\\..\\src\\neutrinos\\data\\icecube\\eventData\\e2.txt");
  findExtremeEventTimes();

  GLfloat fogDensity = 0.1f; 
	GLfloat fogColor[4] = {0.0, 0.0, 0.0, 1.0}; 
	
	GLfloat mat_specular[] = { 0.2, 0.2, 0.2, 0.2 };
	GLfloat mat_shininess[] = { 100.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

	glEnable (GL_FOG); //enable the fog

	glFogi (GL_FOG_MODE, GL_EXP2); 

	glFogfv (GL_FOG_COLOR, fogColor); 

	glFogf (GL_FOG_DENSITY, fogDensity);

	glHint (GL_FOG_HINT, GL_NICEST);

	GLfloat light_position[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat light_position2[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = {1.0,1.0,1.0,1.0};
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position);

	glShadeModel (GL_SMOOTH);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, light_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position2);

	glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT0);
	
	
}


// Method called before data is transferred from master to slaves. Only called
// on the master. This is where anything having to do with
// processing user input or random variables should happen.
void IceCubeFramework::onPreExchange() {
  // Do stuff on master before data is transmitted to slaves.

  // handle joystick-based navigation (drive around). The resulting
  // navigation matrix is automagically transferred to the slaves.
  navUpdate();

  // update the input state (placement matrix & button states) of our effector.
  _effector.updateState( getInputState() );

  // Handle any interaction with the square (see interaction/arInteractionUtilities.h).
  // Any grabbing/dragging happens in here.
  ar_pollingInteraction( _effector, (arInteractable*)&_square );

  // Pack data destined for slaves into appropriate variables
  // (bools transfer as ints).
  _squareHighlightedTransfer = (int)_square.getHighlight();
  _squareMatrixTransfer = _square.getMatrix();

  //JEDIT
  
  ct.handleEvents("");
  ct.update(getTime());
  //ENDJEDIT
}

// Method called after transfer of data from master to slaves. Mostly used to
// synchronize slaves with master based on transferred data.
void IceCubeFramework::onPostExchange() {
  // Do stuff after slaves got data and are again in sync with the master.
  if (!getMaster()) {
    
    // Update effector's input state. On the slaves we only need the matrix
    // to be updated, for rendering purposes.
    _effector.updateState( getInputState() );

    // Unpack our transfer variables.
    _square.setHighlight( (bool)(_squareHighlightedTransfer==0) );
    _square.setMatrix( _squareMatrixTransfer.v );
  }
}

void IceCubeFramework::onDraw( arGraphicsWindow& /*win*/, arViewport& /*vp*/ ) {
  // Load the navigation matrix.
  loadNavMatrix();
  // Draw stuff.
  //glUseProgram(m_shaderProgram);
  _square.draw();
  _effector.draw();
  //glColor3f(1.0f, 0.0f, 0.0f);
  //glutSolidSphere(5.0f, 8, 8);
  //glUseProgram(0);
  ct.draw();
}

// Catch key events.
void IceCubeFramework::onKey( arGUIKeyInfo* keyInfo ) {
  cout << "Key ascii value = " << keyInfo->getKey() << endl;
  cout << "Key ctrl  value = " << keyInfo->getCtrl() << endl;
  cout << "Key alt   value = " << keyInfo->getAlt() << endl;
  string stateString;
  arGUIState state = keyInfo->getState();
  if (state == AR_KEY_DOWN) {
    stateString = "DOWN";
  } else if (state == AR_KEY_UP) {
    stateString = "UP"; 
	if(keyInfo->getKey() == 112){  //p (play forwards)
			ct.vars.time->val = ct.vars.time->start;
			playForward = true;
		}
		if(keyInfo->getKey() == 111){  //o (play backwards)
			ct.vars.time->val = ct.vars.time->end;
			playForward = false;
		}
		if(keyInfo->getKey() == 108){  //l (speed up playing of event)
			if(speedAdjuster == -90 || speedAdjuster == 9){
				speedAdjuster = 0.0f;
			}
			else{speedAdjuster = -90;}
		}
		if(keyInfo->getKey() == 107){  //k (slow down)
			if(speedAdjuster == 9 || speedAdjuster == -90){
				speedAdjuster = 0.0f;
			}
			else{speedAdjuster = 9.0f;}
		}
		if(keyInfo->getKey() == 113){  //q
			ct.vars.time->start = 999999;ct.vars.time->end = 0;		
			eventColors.clear();
			event2Data.~DataInput();
			event2Data.getText("..\\..\\src\\neutrinos\\data\\icecube\\eventData\\e2.txt");
			findExtremeEventTimes();
			playForward = true;
		}
		if(keyInfo->getKey() == 119){  //w
			ct.vars.time->start = 999999;ct.vars.time->end = 0;
			eventColors.clear();
			event2Data.~DataInput();
			event2Data.getText("..\\..\\src\\neutrinos\\data\\icecube\\eventData\\e3.txt");
			findExtremeEventTimes();
			playForward = true;
		}
		if(keyInfo->getKey() == 101){  //e
			ct.vars.time->start = 999999;ct.vars.time->end = 0;
			eventColors.clear();
			event2Data.~DataInput();
			event2Data.getText("..\\..\\src\\neutrinos\\data\\icecube\\eventData\\e5.txt");
			findExtremeEventTimes();
			playForward = true;
		}
		if(keyInfo->getKey() == 114){  //r
			ct.vars.time->start = 999999;ct.vars.time->end = 0;
			eventColors.clear();
			event2Data.~DataInput();
			event2Data.getText("..\\..\\src\\neutrinos\\data\\icecube\\eventData\\e6.txt");
			findExtremeEventTimes();
			playForward = true;
		}
		if(keyInfo->getKey() == 116){  //t
			ct.vars.time->start = 999999;ct.vars.time->end = 0;
			eventColors.clear();
			event2Data.~DataInput();
			event2Data.getText("..\\..\\src\\neutrinos\\data\\icecube\\eventData\\e7.txt");
			findExtremeEventTimes();
			playForward = true;
		}
		

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
void IceCubeFramework::onWindowEvent( arGUIWindowInfo* winInfo ) {
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
    getWindowManager()->setWindowViewport( windowID, 0, 0, width, height );
  }
}









