#include "arPrecompiled.h"
#define SZG_DO_NOT_EXPORT
#include "tdmenucontroller.h"

const float MENU_OFFSET = 15.0;	//the distance from the wand base to the menu center

tdMenuController::tdMenuController(arEffector * wand)
{
	this->wand = wand;
	this->menus = vector<tdMenu>();
	/////////////////////////////////////////////////
	//MENU LAYOUT SECTION: CHANGE TO CUSTOMIZE MENU//
	/////////////////////////////////////////////////

	tdMenu menu0 = tdMenu();	//base state, no panels
	menus.push_back(menu0);

	tdMenu menu1 = tdMenu();	//one big panel and two smaller ones
	tdPanel m1main = tdPanel(arVector3(0,5,0),10,15);
	menu1.addPanel(m1main);
	tdPanel m1left = tdPanel(arVector3(-10,10,2),5,5);
	menu1.addPanel(m1left);
	tdPanel m1right = tdPanel(arVector3(10,10,2),5,5);
	menu1.addPanel(m1right);
	menus.push_back(menu1);

	tdMenu menu2 = tdMenu();	//menu with a bunch of panels side by side
	tdPanel m21 = tdPanel(arVector3(-20*sin(0.523),5,20-20*cos(0.523)),3,5);
	m21.tilt(ar_rotationMatrix(arVector3(0,1,0),0.523));
	menu2.addPanel(m21);
	tdPanel m22 = tdPanel(arVector3(-20*sin(0.262),5,20-20*cos(0.262)),3,5);
	m22.tilt(ar_rotationMatrix(arVector3(0,1,0),0.262));
	menu2.addPanel(m22);
	tdPanel m23 = tdPanel(arVector3(0,5,0),3,5);
	menu2.addPanel(m23);
	tdPanel m24 = tdPanel(arVector3(20*sin(0.262),5,20-20*cos(0.262)),3,5);
	m24.tilt(ar_rotationMatrix(arVector3(0,1,0),-0.262));
	menu2.addPanel(m24);
	tdPanel m25 = tdPanel(arVector3(20*sin(0.523),5,20-20*cos(0.523)),3,5);
	m25.tilt(ar_rotationMatrix(arVector3(0,1,0),-0.523));
	menu2.addPanel(m25);



	menus.push_back(menu2);

	activeMenu = 0;
	nextMenu = 0;
	menus[0].open();

	//////////////////////////////
	//END OF MENU LAYOUT SECTION//
	//////////////////////////////
}

void tdMenuController::draw()
{
	//draw menu
	menus[activeMenu].draw(align,wand->getBaseMatrix());
	//draw pointer
	arVector3 ptr = handlePointer();
	if(ptr != arVector3(0,0,0))
	{
		glPushMatrix();
		glMultMatrixf(wand->getBaseMatrix().v);
		glColor4f(1,1,1,1);
		glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(ptr.v[0],ptr.v[1],ptr.v[2]);
		glEnd();
		glPopMatrix();
	}
}

void tdMenuController::update(double time)
{
	menus[activeMenu].update(time - lastTime);
	lastTime = time;
	if(activeMenu != nextMenu)	//checks if menus are switching
	{
		menus[activeMenu].close();
		if(!menus[activeMenu].isActive())	//checks if previous menu is already closed
		{
			activeMenu = nextMenu;
			menus[activeMenu].open();
		}
	}
}

void tdMenuController::sync()
{
	arMatrix4 base = this->wand->getBaseMatrix();
	base = base * ar_translationMatrix(0,0,-MENU_OFFSET);			//moves menu set distance away from wand
	arVector3 v = ar_extractEulerAngles(base, AR_YXZ);				//order of transforms acquired as yaw-pitch-roll
	arMatrix4 roll = ar_rotationMatrix(arVector3(0,0,1),-v.v[2]);	//removes roll from menu
	arMatrix4 pitch = ar_rotationMatrix(arVector3(1,0,0),-v.v[1]);	//removes tilt from menu
	align = base * roll * pitch;
}

arVector3 tdMenuController::handlePointer()
{
	arVector3 start = arVector3(0,0,0);	//origin of wand
	arVector3 unit = arVector3(0,0,-1);	//unit vector in wand direction
	start = wand->getBaseMatrix() * start;
	start = invert(align) * start;
	unit = wand->getBaseMatrix() * unit;
	unit = invert(align) * unit;
	arVector3 end = menus[activeMenu].handlePointer(start, unit);
	if(end != arVector3(0,0,9001))
	{
		end = align * end;
		end = invert(wand->getBaseMatrix()) * end;
		return end;
	}
	return arVector3(0,0,0);
}

void tdMenuController::handleEvents(string ext)
{
	//TODO: PARSING

	///////////////////////////////////////////////////
	//MENU BEHAVIOR SECTION: CHANGE TO CUSTOMIZE MENU//
	///////////////////////////////////////////////////

	switch(activeMenu)
	{
	case 0:	//menu closed
		if(wand->getOnButton(0))
		{
			this->sync();
			nextMenu = 1;
		}
		break;
	case 1:	//3-panel menu
		if(wand->getOnButton(0))
		{
			nextMenu = 0;
			menus[1].close();
		}
		else if(wand->getOnButton(1))
		{
			nextMenu = 2;
		}
		break;
	case 2:	//side-by-side panels
		if(wand->getOnButton(0))
		{
			nextMenu = 0;
		}
		else if(wand->getOnButton(1))
		{
			nextMenu = 1;
		}
		break;
	default:
		break;
	}

	////////////////////////////////
	//END OF MENU BEHAVIOR SECTION//
	////////////////////////////////
}