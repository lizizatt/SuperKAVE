#ifndef TDMENU_H_
#define TDMENU_H_

#include "arInteractableThing.h"
#include "tdevents.h"

//matrix inverse function adapted from MAYA implementation of GLU library
arMatrix4 invert(arMatrix4 m);

//The basic menu object, base class for anything appearing on a menu panel. Just a template, never used by itself.
class tdObject
{
public:
	tdObject(){}
	virtual void draw(){}
	virtual void update(double time){}
	virtual bool isActive(){return false;}
	virtual arVector3 handlePointer(arVector3 endpt){return arVector3(0,0,9001);}	//checks if pointer is on object, if so, can "hijack" pointer location
	virtual void handleEvents(tdMenuController* ct, int menu, int panel, int object){}
	virtual void change(int code, float value = 0, string msg = ""){}	//called by handleEvents function, used to alter values, flags, etc
protected:
};

//A text pane, displays text
class tdTextPane : public tdObject
{
public:
protected:
};

//A button. You press it and it does stuff
class tdButton : public tdObject
{
public:
	tdButton(float x = 0, float y = 0, float width = 1, float height = 1, float depth = 1);
	virtual void draw();
	virtual void update(double time);
	virtual arVector3 handlePointer(arVector3 endpt);
	virtual void handleEvents(tdMenuController* ct, int menu, int panel, int object);
	virtual void change(int code, float value = 0, string msg = "");
protected:
	float x;
	float y;
	arMatrix4 pos;
	arMatrix4 bump;
	float width;
	float height;
	float cdepth;
	float depth;
	bool cursor;
	bool pushed;
};

//A slider, can be dragged to set a float value
class tdSlider : public tdObject
{
public:
protected:
};

//The basic menu panel, is simply a square pane displayed vertically
class tdPanel
{
public:
	tdPanel(arVector3 center = arVector3(0,0,0), float width = 0, float height = 0);
	virtual void tilt(arMatrix4 matrix);	//rotates the panel after it is positioned
	virtual void add(tdObject* o);	//adds an object
	virtual void draw();
	virtual void update(double time);
	virtual void open();
	virtual void close();
	virtual bool isActive();	//checks if panel is active at all (opening, closing, or open)
	virtual bool isOpen();	//checks if panel is in open state
	virtual arVector3 handlePointer(arVector3 start, arVector3 unit);
	virtual void handleEvents(tdMenuController* ct, int menu, int panel);
protected:
	arMatrix4 cmat;	//used to position the panel
	arMatrix4 tmat;	//used to tilt the panel
	vector<tdObject*> objects;	//the objects appearing on this menu panel
	arVector3 center;	//the center of the panel (relative to menu center)
	float panew;	//the width and height of the "working pane"
	float paneh;
	int phase;		//used for animation, tells what phase of movement the menu is in
private:
	float cwidth;	//current width and height excluding buffer
	float cheight;
};

//A panel that follows the wand
class tdWandPanel
{
public:
protected:
};

//The button icons displayed above the wand
class tdIcon : public tdWandPanel
{
public:
private:
};

//A list of text items
class tdListPanel : public tdPanel
{
public:
};

//A full menu 'state' that contains panels, wand panels, and other things.
class tdMenu
{
public:
	tdMenu();
	virtual void addPanel(tdPanel* p);
	virtual void addWandPanel(tdWandPanel* p);
	virtual void draw(arMatrix4 menualign, arMatrix4 wandalign);
	virtual void update(double time);
	virtual void open();
	virtual void close();
	virtual bool isActive();
	virtual bool isOpen();
	virtual arVector3 handlePointer(arVector3 start, arVector3 unit);
	virtual void handleEvents(tdMenuController* ct, int menu);
private:
	vector<tdPanel*> panels;
	vector<tdWandPanel*> wandPanels;
};

//A menu used for playback controls, uses special implementation
class tdPlaybackMenu : public tdMenu
{
public:
};

#endif