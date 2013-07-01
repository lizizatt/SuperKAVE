#ifndef TDMENU_H_
#define TDMENU_H_

#include "arInteractableThing.h"
#include "tdevents.h"

#define MENU_SPEED 0.03		//the speed at which menu animations run
#define PANEL_THICKNESS 0.02	//how thick each panel is (and how big the pellet is during open/close)
#define TEXT_COMPRESSION 1.5	//how text is "squished" on the x-axis
#define BTNGAP 0.01	//the gaps between buttons

struct slidval
{
	float start;
	float val;
	float end;
	slidval(float start = 0,float val = 0,float end = 10):start(start),val(val),end(end){}
};

struct listval
{
	vector<string> items;
	int selected;
	listval(vector<string> items = vector<string>()):items(items),selected(-1){}
};

//matrix inverse function adapted from MAYA implementation of GLU library
arMatrix4 invert(arMatrix4 m);

//cube drawing function, since glutSolidCube isn't behaving properly for me
void tdDrawBox(float x = 0, float y = 0, float z = 0, float w = 1, float h = 1, float d = 1);

//The basic menu object, base class for anything appearing on a menu panel. Just a template, never used by itself.
class tdObject
{
public:
	tdObject(){}
	virtual void draw(){}
	virtual void update(double time){}
	virtual arVector3 handlePointer(arVector3 endpt){return arVector3(0,0,9001);}	//checks if pointer is on object, if so, can "hijack" pointer location
	virtual void handleEvents(tdMenuController* ct, int menu, int panel, int object){}
	virtual void change(int code, float value = 0, string msg = ""){}	//called by handleEvents function, used to alter values, flags, etc
	virtual int getAction(){return this->actioncode;}
protected:
	int actioncode;	//objects that perform scripted actions (like buttons) are identified by codes
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
	tdButton(float x = 0, float y = 0, float width = 1, float height = 1, float depth = 1, int actioncode = 0, string label = "", float textsize = 1, bool leftjust = false);
	virtual void draw();
	virtual void update(double time);
	virtual arVector3 handlePointer(arVector3 endpt);
	virtual void handleEvents(tdMenuController* ct, int menu, int panel, int object);
	virtual void change(int code, float value = 0, string msg = "");
protected:
	float x;
	float y;
	float width;
	float height;
	float cdepth;
	float depth;
	string label;
	float textsize;
	bool cursor;
	bool pushed;
	bool leftjust;	//used for lists to set text to be left-justified.
};

//A slider, can be dragged to set a float value
class tdSlider : public tdObject
{
public:
	tdSlider(float x = 0, float y = 0, slidval* val = new slidval(), float length = 1, float height = 1, float depth = 1, float width = 1);
	virtual void draw();
	virtual void update(double time);
	virtual arVector3 handlePointer(arVector3 endpt);
	virtual void handleEvents(tdMenuController* ct, int menu, int panel, int object);
	virtual void change(int code, float value = 0, string msg = "");
protected:
	float x;
	float y;
	arMatrix4 pos;
	float length;
	float height;
	float depth;
	float width;
	slidval* val;
	float cpos;	//current position of slider
	float dpos;	//desired position of slider
	bool cursor;
	bool isGrab;
	bool wasGrab;
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
	float cwidth;	//current width and height excluding buffer
	float cheight;
	vector<tdObject*> objects;	//the objects appearing on this menu panel
	arVector3 center;	//the center of the panel (relative to menu center)
	float panew;	//the width and height of the "working pane"
	float paneh;
	int phase;		//used for animation, tells what phase of movement the menu is in
};

//A panel that follows the wand
class tdWandPanel
{
public:
protected:
};

//The button icons displayed above the wand
class tdIcon : public tdObject
{
public:
private:
};

//A list of text items
class tdListPanel : public tdPanel
{
public:
	tdListPanel(listval* list = new listval(), float width = 1, float btnheight = 1, float btndepth = 1, float textsize = 1, int actioncode = 0, bool leftjust = false);
	virtual void draw();
	virtual void open();
	virtual arVector3 handlePointer(arVector3 start, arVector3 unit);
protected:
	listval* list;
	float btnheight;
	float btndepth;
	float textsize;
	int actioncode;
	bool leftjust;
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