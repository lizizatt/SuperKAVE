#ifndef TDMENU_H_
#define TDMENU_H_

#include "arInteractableThing.h"

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
	virtual void draw();
	virtual void update(double time);
	virtual void open();
	virtual void close();
	virtual bool isActive();
	virtual bool isOpen();
protected:
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
	tdMenu(vector<tdPanel> panels = vector<tdPanel>(), vector<tdWandPanel> wandPanels = vector<tdWandPanel>());
	virtual void draw();
	virtual void update(double time);
	virtual void open();
	virtual void close();
	virtual bool isActive();
	virtual bool isOpen();
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