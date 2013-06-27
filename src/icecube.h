#ifndef OOPSKEL_H_
#define OOPSKEL_H_

#include "GL\glew.h"
#include "arMasterSlaveFramework.h"
#include "arInteractableThing.h"
#include "tdmenucontroller.h"

#include "icecubeGeometry.h"
#include "icecubeDataInput.h"

#include "arOBJ.h"

// Class definitions & imlpementations. We'll have just one one class, a 2-ft colored square that
// can be grabbed & dragged around. We'll also have an effector class for doing the grabbing
//
class RodEffectorIce : public arEffector {
  public:
    // set it to use matrix #1 (#0 is normally the user's head) and 3 buttons starting at 0 
    // (no axes, i.e. joystick-style controls; those are generally better-suited for navigation
    // than interaction)
    RodEffectorIce() : arEffector( 1, 3, 0, 0, 0, 0, 0 ) {

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

class IceCubeFramework : public arMasterSlaveFramework {
  public:
    IceCubeFramework();
    virtual bool onStart( arSZGClient& SZGClient );
    virtual void onWindowStartGL( arGUIWindowInfo* );
    virtual void onPreExchange( void );
    virtual void onPostExchange( void );
//    virtual void onWindowInit( void );
    virtual void onDraw( arGraphicsWindow& win, arViewport& vp );
//    virtual void onDisconnectDraw( void );
//    virtual void onPlay( void );
    virtual void onWindowEvent( arGUIWindowInfo* );
//    virtual void onCleanup( void );
//    virtual void onUserMessage( const string& messageBody );
//    virtual void onOverlay( void );
//    virtual void onKey( unsigned char key, int x, int y );
    virtual void onKey( arGUIKeyInfo* );
//    virtual void onMouse( arGUIMouseInfo* );

	float getScale(void) const { return fDownScale; }
	void setScale(float fScale) { fDownScale = fScale; }

  private:
	  
	void drawAxis(void);
	void drawEvents(void);
	void drawTimeline(void);
	void findExtremeEventTimes();

    // Our single object and effector
    RodEffectorIce _effector;
	GLint m_shaderProgram;
	
	IceCubeGeometry geometryData;
	DataInput event2Data; 
	
	arOBJRenderer m_skybox;

	int timeSpan;
	int expansionTime;
	int spin;
	
	float largestCharge;
	float smallestCharge;
	float chargeSpan;
	float fDownScale;

	tdMenuController ct;
	
	struct color {
		float red, green, blue;
	};

	vector<color> eventColors;
	
	static const unsigned int NUM_EVENT_FILES = 6;
	static const char * eventFiles[NUM_EVENT_FILES];
	static const std::string geometryFile;

	static const float nearClipDistance;
	static const float farClipDistance;
	static const float FEET_TO_LOCAL_UNITS;

    // Master-slave transfer variables
	int m_fileIndex;
	float speedAdjuster;
};

#endif