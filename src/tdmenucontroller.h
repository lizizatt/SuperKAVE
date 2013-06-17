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
	tdMenuController(){}	//default constructor doesn't do anything, need to give it a wand
	tdMenuController(arEffector * wand);	//instantiates the menu controller AND sets up a hardcoded menu.
	void draw();	//draws the currently-displayed menu
	void update(double time);
	void sync();	//updates the menus to be displayed wherever the wand is currently pointing
	void handleEvents(string ext);	//checks for events within the menu and controller (and outside) and updates accordingly
private:
	double lastTime;	//used to calculate time change since last update
	arEffector * wand;	//used to track control device
	vector<tdMenu> menus;	//holds all menus
	int activeMenu;	//the menu that is active
	int nextMenu;	//the menu to be activated next (holds value while previous menu is closing)
	arMatrix4 align;	//the matrix used to align the menus to the wand's previous position)
};

#endif