#include "arPrecompiled.h"
#define SZG_DO_NOT_EXPORT
#include "tdmenucontroller.h"

const float MENU_OFFSET = 2.0;	//the distance from the wand base to the menu center

tdMenuController::tdMenuController(arEffector* wand)
{
	this->wand = wand;
	this->menus = vector<tdMenu*>();
	this->lastTime = 0;

	//Initialize vars values
	this->vars.time = new slidval(0,999999,0);
	this->vars.playstatus = 0;
	this->vars.playreverse = false;
	this->vars.zoom = new slidval(1,1,1000);
	this->vars.drawCables = true;
	/////////////////////////////////////////////////
	//MENU LAYOUT SECTION: CHANGE TO CUSTOMIZE MENU//
	/////////////////////////////////////////////////

	tdMenu* menu;
	tdPanel* panel;
	tdWandPanel* wpanel;
	tdObject* object;
	slidval* sv = new slidval();

	menu = new tdMenu();	//base state, no panels
	menus.push_back(menu);

	menu = new tdMenu();	//one big panel and two smaller ones
	panel = new tdPanel(arVector3(0,0,0),2,3);
	object = new tdButton(-0.5, 0.75, 0.5-BTNGAP, 0.25-BTNGAP, 0.05, TD_A_REWIND, "Rewind", 0.1);
	panel->add(object);
	object = new tdButton(0, 0.75, 0.5-BTNGAP, 0.25-BTNGAP, 0.05, TD_A_PLAYPAUSE, "Play", 0.1);
	panel->add(object);
	object = new tdButton(0, 0.5, 0.5-BTNGAP, 0.25-BTNGAP, 0.05, TD_A_REVERSE, "Reverse", 0.1);
	panel->add(object);
	object = new tdButton(0.5, 0.75, 0.5-BTNGAP, 0.25-BTNGAP, 0.05, TD_A_FASTFWD, "FastFwd", 0.1);
	panel->add(object);
	object = new tdSlider(0, 1, vars.time, 1.5, 0.25, 0.05, 0.25);
	panel->add(object);
	menu->addPanel(panel);
	panel = new tdPanel(arVector3(-2,1,0.5),1,2);
	panel->tilt(ar_rotationMatrix(arVector3(0,1,0),M_PI/6));
	menu->addPanel(panel);
	panel = new tdPanel(arVector3(2,1,0.5),1,2);
	panel->tilt(ar_rotationMatrix(arVector3(0,1,0),-M_PI/6));
	object = new tdButton(0, 0.75, 0.5, 0.25, 0.05, TD_A_SWITCHMENU2, "menu 2", 0.1);
	panel->add(object);
	menu->addPanel(panel);
	menus.push_back(menu);

	menu = new tdMenu();	//menu with a bunch of panels side by side
	/*panel = new tdPanel(arVector3(-20*sin(0.523),1,20-20*cos(0.523)),1,2);
	panel->tilt(ar_rotationMatrix(arVector3(0,1,0),0.523));
	menu->addPanel(panel);
	panel = new tdPanel(arVector3(-20*sin(0.262),1,20-20*cos(0.262)),2,4);
	panel->tilt(ar_rotationMatrix(arVector3(0,1,0),0.262));
	menu->addPanel(panel);
	panel = new tdPanel(arVector3(0,1,0),3,5);
	menu->addPanel(panel);
	panel = new tdPanel(arVector3(20*sin(0.262),1,20-20*cos(0.262)),4,6);
	panel->tilt(ar_rotationMatrix(arVector3(0,1,0),-0.262));
	menu->addPanel(panel);
	panel = new tdPanel(arVector3(20*sin(0.523),1,20-20*cos(0.523)),5,8);
	panel->tilt(ar_rotationMatrix(arVector3(0,1,0),-0.523));
	menu->addPanel(panel);
	*/
	string blah[] = {"item1", "item2", "item3"};
	panel = new tdListPanel(new listval(vector<string>(blah,blah+sizeof(blah)/sizeof(string))), 3, 1, 0.2, 0.25, 0, false);
	menu->addPanel(panel);
	wpanel = new tdWandPanel();
	object = new tdButton(0,0,0.5,1,0.1);
	wpanel->add(object);
	menu->addWandPanel(wpanel);
	menus.push_back(menu);

	activeMenu = 0;
	nextMenu = 0;
	menus[0]->open();

	//////////////////////////////
	//END OF MENU LAYOUT SECTION//
	//////////////////////////////
}

void tdMenuController::draw()
{
	//draw menu
	menus[activeMenu]->draw(align,wand->getBaseMatrix());
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
	menus[activeMenu]->draw(align,wand->getBaseMatrix());
}

void tdMenuController::update(double time)
{
	menus[activeMenu]->update(time - lastTime);
	lastTime = time;
	if(activeMenu != nextMenu)	//checks if menus are switching
	{
		menus[activeMenu]->close();
		if(!menus[activeMenu]->isActive())	//checks if previous menu is already closed
		{
			activeMenu = nextMenu;
			menus[activeMenu]->open();
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

void tdMenuController::swap(int next)
{
	nextMenu = next;
}

arVector3 tdMenuController::handlePointer()
{
	arVector3 start = arVector3(0,0,0);	//origin of wand
	arVector3 unit = arVector3(0,0,-1);	//unit vector in wand direction
	start = wand->getBaseMatrix() * start;
	start = invert(align) * start;
	unit = wand->getBaseMatrix() * unit;
	unit = invert(align) * unit;
	arVector3 end = menus[activeMenu]->handlePointer(start, unit);
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
	//TODO: PARSING FOR EXT

	//////////////////////////////////////////////////////////
	//SECTION CONTROLLING EFFECTS OF BUTTONS WITHOUT CONTEXT//
	//////////////////////////////////////////////////////////

	switch(activeMenu)
	{
	case 0:	//menu closed
		if(wand->getOnButton(TD_MENU_TOGGLE))
		{
			this->sync();
			nextMenu = 1;
		}
		break;
	case 1:	//3-panel menu
		if(wand->getOnButton(TD_MENU_TOGGLE))
		{
			nextMenu = 0;
		}
		else if(wand->getOnButton(TD_BUTTON_2))
		{
			nextMenu = 2;
		}
		break;
	case 2:	//side-by-side panels
		if(wand->getOnButton(TD_MENU_TOGGLE))
		{
			nextMenu = 0;
		}
		else if(wand->getOnButton(TD_BUTTON_2))
		{
			nextMenu = 1;
		}
		break;
	default:
		break;
	}

	////////////////////////////////
	//END OF BUTTON ACTION SECTION//
	////////////////////////////////

	menus[activeMenu]->handleEvents(this, activeMenu);
}