#include "arPrecompiled.h"
#define SZG_DO_NOT_EXPORT
#include "tdmenu.h"
#include "arGlut.h"

const float MENU_SPEED = 0.15 / 5;	//the speed at which menu animations run
const float PANEL_THICKNESS = 0.1;	//how thick each panel is (and how big the pellet is during open/close)

//color functions, alter to change color scheme
void colorPanel()	{glColor4f(0.4, 0.8, 1.0, 0.6);}
void colorText()	{glColor4f(1.0, 1.0, 1.0, 1.0);}

//Code adapted from MESA implementation of GLU library
arMatrix4 invert(arMatrix4 m)
{
    float inv[16], invOut[16], det;
    int i;
    inv[0] = m.v[5]  * m.v[10] * m.v[15] - 
             m.v[5]  * m.v[11] * m.v[14] - 
             m.v[9]  * m.v[6]  * m.v[15] + 
             m.v[9]  * m.v[7]  * m.v[14] +
             m.v[13] * m.v[6]  * m.v[11] - 
             m.v[13] * m.v[7]  * m.v[10];

    inv[4] = -m.v[4]  * m.v[10] * m.v[15] + 
              m.v[4]  * m.v[11] * m.v[14] + 
              m.v[8]  * m.v[6]  * m.v[15] - 
              m.v[8]  * m.v[7]  * m.v[14] - 
              m.v[12] * m.v[6]  * m.v[11] + 
              m.v[12] * m.v[7]  * m.v[10];

    inv[8] = m.v[4]  * m.v[9]  * m.v[15] - 
             m.v[4]  * m.v[11] * m.v[13] - 
             m.v[8]  * m.v[5]  * m.v[15] + 
             m.v[8]  * m.v[7]  * m.v[13] + 
             m.v[12] * m.v[5]  * m.v[11] - 
             m.v[12] * m.v[7]  * m.v[9];

    inv[12] = -m.v[4]  * m.v[9]  * m.v[14] + 
               m.v[4]  * m.v[10] * m.v[13] +
               m.v[8]  * m.v[5]  * m.v[14] - 
               m.v[8]  * m.v[6]  * m.v[13] - 
               m.v[12] * m.v[5]  * m.v[10] + 
               m.v[12] * m.v[6]  * m.v[9];

    inv[1] = -m.v[1]  * m.v[10] * m.v[15] + 
              m.v[1]  * m.v[11] * m.v[14] + 
              m.v[9]  * m.v[2]  * m.v[15] - 
              m.v[9]  * m.v[3]  * m.v[14] - 
              m.v[13] * m.v[2]  * m.v[11] + 
              m.v[13] * m.v[3]  * m.v[10];

    inv[5] = m.v[0]  * m.v[10] * m.v[15] - 
             m.v[0]  * m.v[11] * m.v[14] - 
             m.v[8]  * m.v[2]  * m.v[15] + 
             m.v[8]  * m.v[3]  * m.v[14] + 
             m.v[12] * m.v[2]  * m.v[11] - 
             m.v[12] * m.v[3]  * m.v[10];

    inv[9] = -m.v[0]  * m.v[9]  * m.v[15] + 
              m.v[0]  * m.v[11] * m.v[13] + 
              m.v[8]  * m.v[1]  * m.v[15] - 
              m.v[8]  * m.v[3]  * m.v[13] - 
              m.v[12] * m.v[1]  * m.v[11] + 
              m.v[12] * m.v[3]  * m.v[9];

    inv[13] = m.v[0]  * m.v[9]  * m.v[14] - 
              m.v[0]  * m.v[10] * m.v[13] - 
              m.v[8]  * m.v[1]  * m.v[14] + 
              m.v[8]  * m.v[2]  * m.v[13] + 
              m.v[12] * m.v[1]  * m.v[10] - 
              m.v[12] * m.v[2]  * m.v[9];

    inv[2] = m.v[1]  * m.v[6] * m.v[15] - 
             m.v[1]  * m.v[7] * m.v[14] - 
             m.v[5]  * m.v[2] * m.v[15] + 
             m.v[5]  * m.v[3] * m.v[14] + 
             m.v[13] * m.v[2] * m.v[7] - 
             m.v[13] * m.v[3] * m.v[6];

    inv[6] = -m.v[0]  * m.v[6] * m.v[15] + 
              m.v[0]  * m.v[7] * m.v[14] + 
              m.v[4]  * m.v[2] * m.v[15] - 
              m.v[4]  * m.v[3] * m.v[14] - 
              m.v[12] * m.v[2] * m.v[7] + 
              m.v[12] * m.v[3] * m.v[6];

    inv[10] = m.v[0]  * m.v[5] * m.v[15] - 
              m.v[0]  * m.v[7] * m.v[13] - 
              m.v[4]  * m.v[1] * m.v[15] + 
              m.v[4]  * m.v[3] * m.v[13] + 
              m.v[12] * m.v[1] * m.v[7] - 
              m.v[12] * m.v[3] * m.v[5];

    inv[14] = -m.v[0]  * m.v[5] * m.v[14] + 
               m.v[0]  * m.v[6] * m.v[13] + 
               m.v[4]  * m.v[1] * m.v[14] - 
               m.v[4]  * m.v[2] * m.v[13] - 
               m.v[12] * m.v[1] * m.v[6] + 
               m.v[12] * m.v[2] * m.v[5];

    inv[3] = -m.v[1] * m.v[6] * m.v[11] + 
              m.v[1] * m.v[7] * m.v[10] + 
              m.v[5] * m.v[2] * m.v[11] - 
              m.v[5] * m.v[3] * m.v[10] - 
              m.v[9] * m.v[2] * m.v[7] + 
              m.v[9] * m.v[3] * m.v[6];

    inv[7] = m.v[0] * m.v[6] * m.v[11] - 
             m.v[0] * m.v[7] * m.v[10] - 
             m.v[4] * m.v[2] * m.v[11] + 
             m.v[4] * m.v[3] * m.v[10] + 
             m.v[8] * m.v[2] * m.v[7] - 
             m.v[8] * m.v[3] * m.v[6];

    inv[11] = -m.v[0] * m.v[5] * m.v[11] + 
               m.v[0] * m.v[7] * m.v[9] + 
               m.v[4] * m.v[1] * m.v[11] - 
               m.v[4] * m.v[3] * m.v[9] - 
               m.v[8] * m.v[1] * m.v[7] + 
               m.v[8] * m.v[3] * m.v[5];

    inv[15] = m.v[0] * m.v[5] * m.v[10] - 
              m.v[0] * m.v[6] * m.v[9] - 
              m.v[4] * m.v[1] * m.v[10] + 
              m.v[4] * m.v[2] * m.v[9] + 
              m.v[8] * m.v[1] * m.v[6] - 
              m.v[8] * m.v[2] * m.v[5];

    det = m.v[0] * inv[0] + m.v[1] * inv[4] + m.v[2] * inv[8] + m.v[3] * inv[12];
    if (det == 0)
        return ar_identityMatrix();
    det = 1.0 / det;
    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;
    return arMatrix4(invOut);
}

////////////////////////////////////////////////////////////////////////////////
//TDTEXTPANE METHODS
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//TDBUTTON METHODS
////////////////////////////////////////////////////////////////////////////////

tdButton::tdButton(float x, float y, float width, float height, float depth)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->cdepth = depth;
	this->depth = depth;
	this->pos = ar_translationMatrix(x, y, 0);
	this->bump = ar_translationMatrix(0, 0, 0.5);
	this->cursor = false;
	this->pushed = false;
}

void tdButton::draw()
{
	glPushMatrix();
	glMultMatrixf(pos.v);
	glMultMatrixf(ar_scaleMatrix(width, height, cdepth).v);
	glMultMatrixf(bump.v);
	glutSolidCube(1);
	glPopMatrix();
}

void tdButton::update(double time)
{
	if(pushed)
	{
		cdepth -= MENU_SPEED * time;
		if(cdepth < depth / 2)
			cdepth = depth / 2;
	}
	else
	{
		cdepth += MENU_SPEED * time;
		if(cdepth > depth)
			cdepth = depth;
	}
	pushed = false;
	cursor = false;
}

arVector3 tdButton::handlePointer(arVector3 endpt)
{
	if(endpt.v[0] >= x - width/2 && endpt.v[0] <= x + width/2 && endpt.v[1] >= y - height/2 && endpt.v[1] <= y + height/2)
	{
		cursor = true;
		return arVector3(endpt.v[0],endpt.v[1],endpt.v[2]+cdepth);
	}
	return arVector3(0,0,9001);
}

void tdButton::handleEvents(tdMenuController* ct, int menu, int panel, int object)
{
	if(cursor)
		handleEvent(ct,menu,panel,object,TD_BUTTON_CURSOR,this);
	else
		handleEvent(ct,menu,panel,object,TD_BUTTON_IDLE,this);
}

void tdButton::change(int code, float value, string msg)
{
	switch(code)
	{
	case TD_PUSH:
		pushed = true;
		break;
	default:
		break;
	}
}

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
	this->cmat = ar_translationMatrix(center);
	this->tmat = ar_identityMatrix();
	this->objects = vector<tdObject*>();
}

void tdPanel::tilt(arMatrix4 matrix)
{
	this->tmat = matrix;
}

void tdPanel::add(tdObject* o)
{
	this->objects.push_back(o);
}

void tdPanel::draw()
{
	if(phase != 0)	//only display panel if it's actually active
	{
		glEnable(GL_CULL_FACE); //dunno if want this, guess I'll see
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();	//move to panel position
		glMultMatrixf(cmat.v);
		glMultMatrixf(tmat.v);
		glPushMatrix();	//draw the panel
		glMultMatrixf(ar_scaleMatrix(PANEL_THICKNESS + cwidth, PANEL_THICKNESS + cheight, PANEL_THICKNESS).v);
		glMultMatrixf(ar_translationMatrix(0,0,-0.5).v);
		colorPanel();
		glutSolidCube(1);
		glPopMatrix();
		if(cwidth > 0 && cheight > 0)
		{
			//glMultMatrixf(ar_translationMatrix(-cwidth/2, -cheight/2,0).v);	//would set origin to bottom-left corner
			glMultMatrixf(ar_scaleMatrix(cwidth/panew, cheight/paneh, 1).v);
			for(int i = 0; i < objects.size(); i++)
			{
				objects[i]->draw();
			}
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
	for(int i = 0; i < objects.size(); i++)
		objects[i]->update(time);
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

arVector3 tdPanel::handlePointer(arVector3 start, arVector3 unit)
{
	arVector3 nstart = start;
	nstart = invert(cmat) * nstart;
	nstart = invert(tmat) * nstart;
	arVector3 nunit = unit;
	nunit = invert(cmat) * nunit;
	nunit = invert(tmat) * nunit;
	arVector3 endpt = arVector3();
	arVector3 dir = nunit - nstart;
	if(dir.v[2] < 0 && nstart.v[2] > 0)	//check if pointer is in front of menu panel and not pointing backwards/sideways
	{
		float mag = nstart.v[2] / -dir.v[2];
		endpt = arVector3(nstart.v[0]+dir.v[0]*mag, nstart.v[1]+dir.v[1]*mag, nstart.v[2]+dir.v[2]*mag);
		if(abs(endpt.v[0]) < (PANEL_THICKNESS + cwidth) / 2 && abs(endpt.v[1]) < (PANEL_THICKNESS + cheight) / 2)
		{
			arVector3 newend;
			for(int i = 0; i < objects.size(); i++)
			{
				newend = objects[i]->handlePointer(endpt);
				if(newend.v[2] != 9001)
					endpt = newend;
			}
			endpt = tmat * endpt;
			endpt = cmat * endpt;
			return endpt;
		}
	}
	return arVector3(0,0,9001);
}

void tdPanel::handleEvents(tdMenuController* ct, int menu, int panel)
{
	for(int i = 0; i < objects.size(); i++)
		objects[i]->handleEvents(ct, menu, panel, i);
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
	this->panels = vector<tdPanel*>();
	this->wandPanels = vector<tdWandPanel*>();
}

void tdMenu::addPanel(tdPanel* p)
{
	panels.push_back(p);
}

void tdMenu::addWandPanel(tdWandPanel* p)
{
	//TODO
}

void tdMenu::draw(arMatrix4 menualign, arMatrix4 wandalign)
{
	glPushMatrix();
	glMultMatrixf(menualign.v);
	for(int i = 0; i < panels.size(); i++)
	{
		panels[i]->draw();
	}
	glPopMatrix();
	glPushMatrix();	//align to wand and draw
	glMultMatrixf(wandalign.v);
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//wandPanels[i]->draw();
	}
	glPopMatrix();
}

void tdMenu::update(double time)
{
	for(int i = 0; i < panels.size(); i++)
	{
		panels[i]->update(time);
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//wandPanels[i]->update(time);
	}
}

void tdMenu::open()
{
	for(int i = 0; i < panels.size(); i++)
	{
		panels[i]->open();
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//wandPanels[i]->open();
	}
}

void tdMenu::close()
{
	for(int i = 0; i < panels.size(); i++)
	{
		panels[i]->close();
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//wandPanels[i]->close();
	}
}

bool tdMenu::isActive()
{
	for(int i = 0; i < panels.size(); i++)
	{
		if(panels[i]->isActive()) return true;
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//if(wandPanels[i]->isActive()) return true;
	}
	return false;
}

bool tdMenu::isOpen()
{
	for(int i = 0; i < panels.size(); i++)
	{
		if(!panels[i]->isOpen()) return false;
	}
	for(int i = 0; i < wandPanels.size(); i++)
	{
		//if(!wandPanels[i]->isOpen()) return false;
	}
	return false;
}

arVector3 tdMenu::handlePointer(arVector3 start, arVector3 unit)
{
	arVector3 nstart = start;
	arVector3 nunit = unit;
	arVector3 endpt = arVector3();
	for(int i = 0; i < panels.size(); i++)
	{
		endpt = panels[i]->handlePointer(nstart, nunit);
		if(endpt != arVector3(0,0,9001))
		{
			return endpt;
		}
	}
	return arVector3(0,0,9001);
}

void tdMenu::handleEvents(tdMenuController* ct, int menu)
{
	for(int i = 0; i < panels.size(); i++)
		panels[i]->handleEvents(ct, menu, i);
}

////////////////////////////////////////////////////////////////////////////////
//TDPLAYBACKMENU METHODS
////////////////////////////////////////////////////////////////////////////////

