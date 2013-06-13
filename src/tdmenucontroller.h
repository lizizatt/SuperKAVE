#ifndef TDMENUCONTROLLER_H_
#define TDMENUCONTROLLER_H_

#include "arInteractableThing.h"
#include "tdmenu.h"
#include <vector>

class tdMenuController
{
public:
	tdMenuController(arEffector wand, vector<tdMenu> menus);
	void draw();	//draws the currently-displayed menu
	void update(double time);
	void sync();	//updates the menus to be displayed wherever the wand is currently pointing
private:
	arEffector wand;	//used to track control device
	vector<tdMenu> menus;
	tdMenu activeMenu;	//the menu that is active
	arMatrix4 align;	//the matrix used to align the menus to the wand's previous position)
};

#endif