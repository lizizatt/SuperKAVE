#ifndef TDMENUCONTROLLER_H_
#define TDMENUCONTROLLER_H_

#include "arInteractableThing.h"
#include "tdmenu.h"
#include <vector>
#include "arMasterSlaveFramework.h"


//This class controls the menu system and its behavior. To alter the menu configuration, change the MenuController constructor and handleEvents functions
class tdMenuController
{
public:
	struct valuetable	//holds all values maintained by menu system
	{
		slidval* time;
		int playstatus;	//feedback to/from playback controls
		bool playreverse;	//true if playing event backwards
	}vars;
	tdMenuController(){}	//default constructor doesn't do anything, need to give it a wand
	tdMenuController(arEffector* wand);	//instantiates the menu controller AND sets up a hardcoded menu.
	void draw();	//draws the currently-displayed menu
	void update(double time);
	void sync();	//updates the menus to be displayed wherever the wand is currently pointing
	void swap(int next);	//switches which menu is active
	arVector3 handlePointer();	//gets endpoint of laser pointer and handles effects of current pointer placement in menu
	void handleEvents(string ext);	//checks for events within the menu and controller (and outside) and updates accordingly
	arEffector * wand;	//used to track control device
private:
	double lastTime;	//used to calculate time change since last update
	vector<tdMenu*> menus;	//holds all menus
	int activeMenu;	//the menu that is active
	int nextMenu;	//the menu to be activated next (holds value while previous menu is closing)
	arMatrix4 align;	//the matrix used to align the menus to the wand's previous position)
};

#endif