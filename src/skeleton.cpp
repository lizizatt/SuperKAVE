//********************************************************
// Syzygy is licensed under the BSD license v2
// see the file SZG_CREDITS for details
//********************************************************

// precompiled header include MUST appear as the first non-comment line
#include "arPrecompiled.h"
// MUST come before other szg includes. See arCallingConventions.h for details.
#define SZG_DO_NOT_EXPORT
#include <stdio.h>
#include <stdlib.h>
#include "arMasterSlaveFramework.h"
#include "arInteractableThing.h"
#include "arInteractionUtilities.h"
#include "arGlut.h"
//#include "arMath.h"
//#include <string.h>
//#include <math.h>
//#include <vector>
//#include <iostream>
//#include <fstream>


// The class containing all the relevant information about the dots to be displayed.
class dot {
public:
    float cx, cy, cz, charge, time;
    arVector3 angle;
    dot(float x, float y, float z, arVector3 theta, float q, float t) {
        cx = x;
        cy = y;
        cz = z;
        angle = theta;
        charge = q;
        time = t;
    };
    dot(){};
};

//Global variables:
float ax, az;
GLUquadricObj * quadObj;        //cylinder quadratic object.
static float RADIUS = 17.0;     //Constants for sizing.
static float HEIGHT = 40.0;
static float PI = 3.141592;
static float DOTRAD = 0.4;
vector<dot> dots;               //Vector to hold unknown number of dot objects (the dots on the cylinder.)
float viewer_distance=100.0;
float viewer_angle=0.0;
bool l_button=false;
bool r_button=false;
arVector3 l_position;
dot tempDot;                    //dot held between loading new events. This is a one-dot buffer
bool stored = false;            //Telling us whether a dot is stored in tempDot
bool colorByCharge = true;      //whether to color by charge or time
ifstream dataFile;              //data input
//string bufferLine;  

//absolute value function to deal with issue in dotColor regarding multiple definitions of abs() in libraries
double abs(double in){
	if(in > 0.0){
		return in;
	}
	return in * -1.0;
}


//returns color of dots based on whether charge or time are the metrics for coloring. 
arVector3 dotColor(dot d) {
    float red, green, blue;
    float numDivs = 16;
    float section = 0;
    if(colorByCharge) {
        float value = d.charge;
        if(value > 26.7) section = 16;
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
        
        red = max(0.0, 255.0 * (section - numDivs / 2.0) / (numDivs / 2.0));
        green = max(0.0, 255.0 * (numDivs / 2 - abs((numDivs / 2.0 - section))) / (numDivs / 2.0));
        blue = max(0.0, 255.0 * (numDivs / 2.0 - section) / (numDivs / 2.0));
    } else {
        float value = d.charge;
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
        
        red = max(0.0, 255.0 * (section - numDivs / 2.0) / (numDivs / 2.0));
        green = max(0.0, 255.0 * (numDivs / 2 - abs((numDivs / 2.0 - section))) / (numDivs / 2.0));
        blue = max(0.0, 255.0 * (numDivs / 2.0 - section) / (numDivs / 2.0));
    }    
    return arVector3(red,green,blue);
}

//logic for drawing a circle on the edge of the cylinder. 
void drawCircle(dot d) {
    bool top = (d.angle[0]==0 && d.angle[1]==0) ? true : false;
	float cx = d.cx;
	float cy = d.cy;
    float cz = d.cz;
	float j, newx, newy, newz;
    arVector3 color = dotColor(d);
    glColor3f(color[0], color[1], color[2]);
	glBegin(GL_LINES);
    if(top)
        for(j = 0.0; j < 2 * PI; j += PI / 180.0) {
            newx = cx + DOTRAD * sin(j);
            newy = cy + DOTRAD * cos(j);
            glVertex3f(cx,cy,cz);
            glVertex3f(newx, newy, cz);
        }    
    else {
        float theta = atan2(cy,cx);
        for(j = 0.0; j < 2 * PI; j += PI / 180.0) {
            newx = cx + sin(theta) * DOTRAD * cos(j);
            newy = cy - cos(theta) * DOTRAD * cos(j);
            newz = cz + DOTRAD * sin(j);
            glVertex3f(cx,cy,cz);
            glVertex3f(newx, newy, newz);
        }
    }
	glEnd();
}

//loop through vector of dots and draws them all. 
void drawDots(void) {
    for(std::vector<dot>::size_type i = 0; i != dots.size(); i++) {
        drawCircle(dots[i]);
    }
}

void drawScene(void)
{
  //  glClearColor(0.0, 0.0, 0.0, 0.0);
  //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
  //  setup_camera();  //see how program runs without this for the moment
    
    glPushMatrix();
    gluQuadricDrawStyle(quadObj, GLU_LINE);
    glColor3f(1.0, 1.0, 1.0);
    glTranslatef(0,20,0);
    glRotatef(ax, 1.0, 0.0, 0.0);
    glRotatef(az, 0.0, 0.0, 1.0);
    gluCylinder(quadObj, RADIUS, RADIUS, HEIGHT, 50, 30);
    drawDots();
    
    glPopMatrix();
    
 //   glutSwapBuffers(); //this is not necessary in syzygy.  This crashes things in syzygy.  -jaz
}



/*  logic to load next event. First clears the vector containing all the dots, 
    then loads new dots. The key here is that we store the first dot of the 
    next event, which then signals the end of that event's data. 
*/
void loadNextEvent(void) {
    int hit;
    arVector3 theta;
    float x,y,z,q,t;
    
    dots.clear();

    bool debug = false;
    if(dataFile.is_open()) {
        while (dataFile.good()) {
            if(stored) dots.push_back(tempDot);
            dataFile >> hit >> x >> y >> z >> q >> t;
            if(dataFile.fail()) break;
            x = x / 100.0;
            y = y / 100.0;
            z = z * 20.0 / 1810.0 + 20.0;
            theta = (z==0 || z == 40) ? arVector3(0.0,0.0,1.0): arVector3(x,y,0.0);
            tempDot = dot(x,y,z,theta,q,t);
            if(hit != 1 || dots.size() == 0) dots.push_back(tempDot);
            else {stored = true; break; }
        }
        if(dataFile.fail() || !dataFile.good()) dataFile.close();
    } else {
		printf("file was not open!");
		exit(0);
	}
		//  drawScene();
}

//reads in file and does initialization steps for glut_cylinder
void readInFile(){
	dataFile.open("C:/users/owner/desktop/glut_cylinder/temp");
    if(!dataFile.is_open()) {
        cout << "Unable to open file";
        exit(0);
    }
	quadObj = gluNewQuadric();
}

// Unit conversions.  Tracker (and cube screen descriptions) use feet.
// Atlantis, for example, uses 1/2-millimeters, so the appropriate conversion
// factor is 12*2.54*20.
const float FEET_TO_LOCAL_UNITS = 1.;

// Near & far clipping planes.
const float nearClipDistance = .1*FEET_TO_LOCAL_UNITS;
const float farClipDistance = 100.*FEET_TO_LOCAL_UNITS;
  
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
    // Draw it. Maybe share some data from master to slaves.
    // The matrix can be gotten from updateState() on the slaves' postExchange().
    void draw() const;
  private:
};
void RodEffector::draw() const {
  glPushMatrix();
    glMultMatrixf( getCenterMatrix().v );
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

// Global variables - our single object and effector
RodEffector theEffector;

// Master-slave transfer variables



// start callback (called in arMasterSlaveFramework::start()
//
// Note: DO NOT do OpenGL initialization here; this is now called
// __before__ window creation. Do it in the WindowStartGL callback.
//
bool start( arMasterSlaveFramework& framework, arSZGClient& /*cli*/ ) {
  // Register shared memory.
  //  framework.addTransferField( char* name, void* address, arDataType type, int numElements ); e.g.
//  framework.addTransferField("squareHighlighted", &squareHighlightedTransfer, AR_INT, 1);


  // Setup navigation, so we can drive around with the joystick
  //
  // Tilting the joystick by more than 20% along axis 1 (the vertical on ours) will cause
  // translation along Z (forwards/backwards). This is actually the default behavior, so this
  // line isn't necessary.
  framework.setNavTransCondition( 'z', AR_EVENT_AXIS, 1, 0.2 );      

  // Tilting joystick left or right will rotate left/right around vertical axis (default is left/right
  // translation)
  framework.setNavRotCondition( 'y', AR_EVENT_AXIS, 0, 0.2 );      

  // Set translation & rotation speeds to 5 ft/sec & 30 deg/sec (defaults)
  framework.setNavTransSpeed( 5. );
  framework.setNavRotSpeed( 30. );

  return true;
}

// Callback to initialize each window (because now a Syzygy app can
// have more than one).
void windowStartGL( arMasterSlaveFramework&, arGUIWindowInfo* ) {
  // OpenGL initialization
  glClearColor(0,0,0,0);

  //opens the file, does basic initialization, loads first event
  readInFile();
  loadNextEvent();
}

// Callback called before data is transferred from master to slaves. Only called
// on the master. This is where anything having to do with
// processing user input or random variables should happen.
void preExchange( arMasterSlaveFramework& fw ) {
//	printf("starting preexchange \n");
  // Do stuff on master before data is transmitted to slaves.

  // handle joystick-based navigation (drive around). The resulting
  // navigation matrix is automagically transferred to the slaves.
  fw.navUpdate();

  // update the input state (placement matrix & button states) of our effector.
  theEffector.updateState( fw.getInputState() );

  // Pack data destined for slaves into appropriate variables
  // (bools transfer as ints).

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
}

void display( arMasterSlaveFramework& fw ) {
//	printf("started display \n");
  // Load the navigation matrix.
  fw.loadNavMatrix();
  // Draw stuff.
  theEffector.draw();

  //draws cylinder
  drawScene();
//  printf("got to end of display \n");
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
  } else if (state == AR_KEY_UP) {
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
  // also setExitCallback(), setUserMessageCallback()
  // in demo/arMasterSlaveFramework.h


  if (!framework.init(argc, argv)) {
    return 1;
  }

  // Never returns unless something goes wrong
  return framework.start() ? 0 : 1;
}
