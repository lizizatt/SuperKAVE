#ifndef TDMENU_H_
#define TDMENU_H_

#include "arInteractableThing.h"

<<<<<<< HEAD
//matrix inverse function adapted from MAYA implementation of GLU library
arMatrix4 invert(arMatrix4 m);

=======
>>>>>>> origin/master
//The basic menu object, base class for anything appearing on a menu panel
class tdObject
{
public:
};

//The basic menu panel, is simply a square pane displayed vertically
class tdPanel
{
public:
	tdPanel(arVector3 center = arVector3(0,0,0), float width = 0, float height = 0);
	virtual void tilt(arMatrix4 matrix);	//rotates the panel after it is positioned
	virtual void add(tdObject o);	//adds an object
	//TODO: implement tilt function
	virtual void draw();
	virtual void update(double time);
	virtual void open();
	virtual void close();
	virtual bool isActive();	//checks if panel is active at all (opening, closing, or open)
	virtual bool isOpen();	//checks if panel is in open state
<<<<<<< HEAD
	virtual arVector3 handlePointer(arVector3 start, arVector3 unit);
protected:
	arMatrix4 cmat;	//used to position the panel
=======
protected:
>>>>>>> origin/master
	arMatrix4 tmat;	//used to tilt the panel
	vector<tdObject> objects;	//the objects appearing on this menu panel
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
};

//The button icons displayed above the wand
class tdIcon : public tdWandPanel
{
public:
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
	virtual void addPanel(tdPanel p);
	virtual void addWandPanel(tdWandPanel p);
	virtual void draw(arMatrix4 menualign, arMatrix4 wandalign);
	virtual void update(double time);
	virtual void open();
	virtual void close();
	virtual bool isActive();
	virtual bool isOpen();
<<<<<<< HEAD
	virtual arVector3 handlePointer(arVector3 start, arVector3 unit);
=======
>>>>>>> origin/master
private:
	vector<tdPanel> panels;
	vector<tdWandPanel> wandPanels;
};

//A menu used for playback controls, uses special implementation
class tdPlaybackMenu : public tdMenu
{
public:
};

#endif