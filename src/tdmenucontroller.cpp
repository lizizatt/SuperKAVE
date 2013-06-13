#include "arPrecompiled.h"
#define SZG_DO_NOT_EXPORT
#include "tdmenucontroller.h"

const float MENU_OFFSET = 10.0;	//the distance from the wand base to the menu center

tdMenuController::tdMenuController(arEffector wand, vector<tdMenu> menus)
{
	//TODO
}

void tdMenuController::draw()
{
	//TODO
}

void tdMenuController::update(double time)
{
	//TODO
}

void tdMenuController::sync()
{
	arMatrix4 base = this->wand.getBaseMatrix();
	base = base * ar_translationMatrix(0,0,-MENU_OFFSET);			//moves menu set distance away from wand
	arVector3 v = ar_extractEulerAngles(base, AR_YXZ);				//order of transforms acquired as yaw-pitch-roll
	arMatrix4 roll = ar_rotationMatrix(arVector3(0,0,1),-v.v[2]);	//removes roll from menu
	arMatrix4 pitch = ar_rotationMatrix(arVector3(1,0,0),-v.v[1]);	//removes tilt from menu
	this->align = base * roll * pitch;
}