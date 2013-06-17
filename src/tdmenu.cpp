#include "arPrecompiled.h"
#define SZG_DO_NOT_EXPORT
#include "tdmenu.h"
#include "arGlut.h"

const float MENU_SPEED = 0.15;	//the speed at which menu animations run
const float PANEL_THICKNESS = 0.1;	//how thick each panel is (and how big the pellet is during open/close)

//color functions, alter to change color scheme
void colorPanel()	{glColor4f(0.4, 0.8, 1.0, 0.6);}
void colorText()	{glColor4f(1.0, 1.0, 1.0, 1.0);}

////////////////////////////////////////////////////////////////////////////////
//TDOBJECT METHODS
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//TDPANEL METHODS
////////////////////////////////////////////////////////////////////////////////

tdPanel::tdPanel(arVector3 center, float width, float height)
{
	this->center = center;
	this->panew = width;
	this->paneh = height;
	this->phase = 0;
	this->cwidth = 0;
	this->cheight = 0;
	this->tmat = ar_identityMatrix();
}

void tdPanel::tilt(arMatrix4 matrix)
{
	this->tmat = matrix;
}

void tdPanel::add(tdObject o)
{
	//TODO
}

void tdPanel::draw()
{
	if(phase != 0)	//only display panel if it's actually active
	{
		//glEnable(GL_CULL_FACE); dunno if want this, guess I'll see
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();	//move to panel position
		glMultMatrixf(ar_translationMatrix(center).v);
		glMultMatrixf(tmat.v);
		glPushMatrix();	//draw the panel
		glMultMatrixf(ar_scaleMatrix(PANEL_THICKNESS + cwidth, PANEL_THICKNESS + cheight, PANEL_THICKNESS).v);
		glMultMatrixf(ar_translationMatrix(0,0,-0.5).v);
		colorPanel();
		glutSolidCube(1);
		glPopMatrix();
		if(cwidth > 0 && cheight > 0)
		{
			//TODO: draw other stuff (transform as needed first)
		}
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
	}
}

void tdPanel::update(double time)
{
	switch(phase)	//motion of menu determined by phase
	{
	case 0:	//menu not displayed
		cwidth = 0;
		cheight = 0;
		break;
	case 1:	//menu bar horizontally expanding
		cwidth += MENU_SPEED * time;
		cheight = 0;
		if(cwidth >= panew)
		{
			cwidth = panew;
			phase = 2;
		}
		break;
	case 2:	//menu vertically expanding
		cwidth = panew;
		cheight += MENU_SPEED * time;
		if(cheight >= paneh)
		{
			cheight = paneh;
			phase = 3;
		}
		break;
	case 3:	//menu fully open
		cwidth = panew;
		cheight = paneh;
		break;
	case 4:	//menu vertically compressing
		cwidth = panew;
		cheight -= MENU_SPEED * time;
		if(cheight <= 0)
		{
			cheight = 0;
			phase = 5;
		}
		break;
	case 5:	//menu bar horizontally compressing
		cwidth -= MENU_SPEED * time;
		cheight = 0;
		if(cwidth <= 0)
		{
			cwidth = 0;
			phase = 0;
		}
		break;
	default:
		phase = 0;
		break;
	}
}

void tdPanel::open()
{
	if(phase == 0 || phase == 5) phase = 1;
	if(phase == 4) phase = 2;
}

void tdPanel::close()
{
	if(phase == 1) phase = 5;
	if(phase > 1 && phase < 5) phase = 4;
}

bool tdPanel::isActive()
{
	return (phase != 0);
}

bool tdPanel::isOpen()
{
	return (phase == 3);
}

////////////////////////////////////////////////////////////////////////////////
//TDWANDPANEL METHODS
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//TDICON METHODS
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//TDLISTPANEL METHODS
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//TDMENU METHODS
////////////////////////////////////////////////////////////////////////////////

tdMenu::tdMenu()
{
	this->panels = vector<tdPanel>();
	this->wandPanels = vector<tdWandPanel>();
}

void tdMenu::addPanel(tdPanel p)
{
	panels.push_back(p);
}

void tdMenu::addWandPanel(tdWandPanel p)
{
	//TODO
}

void tdMenu::draw(arMatrix4 menualign, arMatrix4 wandalign)
{
	glPushMatrix();
	glMultMatrixf(menualign.v);
	for(int i = 0; i < panels.size(); i++)
	{
		panels[i].draw();
	}
	glPopMatrix();
	glPushMatrix();	//align to wand and draw
	glMultMatrixf(wandalign.v);
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//wandPanels[i].draw();
	}
	glPopMatrix();
}

void tdMenu::update(double time)
{
	for(int i = 0; i < panels.size(); i++)
	{
		panels[i].update(time);
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//wandPanels[i].update(time);
	}
}

void tdMenu::open()
{
	for(int i = 0; i < panels.size(); i++)
	{
		panels[i].open();
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//wandPanels[i].open();
	}
}

void tdMenu::close()
{
	for(int i = 0; i < panels.size(); i++)
	{
		panels[i].close();
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//wandPanels[i].close();
	}
}

bool tdMenu::isActive()
{
	for(int i = 0; i < panels.size(); i++)
	{
		if(panels[i].isActive()) return true;
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//if(wandPanels[i].isActive()) return true;
	}
	return false;
}

bool tdMenu::isOpen()
{
	for(int i = 0; i < panels.size(); i++)
	{
		if(!panels[i].isOpen()) return false;
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//if(!wandPanels[i].isOpen()) return false;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
//TDPLAYBACKMENU METHODS
////////////////////////////////////////////////////////////////////////////////

