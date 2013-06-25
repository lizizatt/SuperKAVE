#include "arPrecompiled.h"
#define SZG_DO_NOT_EXPORT
#include "tdmenu.h"
#include "arGlut.h"


const float MENU_SPEED = 0.15 / 5;	//the speed at which menu animations run
const float PANEL_THICKNESS = 0.1;	//how thick each panel is (and how big the pellet is during open/close)
const float TEXT_COMPRESSION = 1.5;	//how text is "squished" on the x-axis

//color functions, alter to change color scheme
void colorPanel()	{glColor4f(0.4, 0.8, 1.0, 0.75);}
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

void tdDrawBox(float x, float y, float z, float w, float h, float d)
{
	glBegin(GL_QUAD_STRIP);
	glVertex3f(w/2+x,h/2+y,d/2+z);
	glVertex3f(w/2+x,-h/2+y,d/2+z);
	glVertex3f(w/2+x,h/2+y,-d/2+z);
	glVertex3f(w/2+x,-h/2+y,-d/2+z);
	glVertex3f(-w/2+x,h/2+y,-d/2+z);
	glVertex3f(-w/2+x,-h/2+y,-d/2+z);
	glVertex3f(-w/2+x,h/2+y,d/2+z);
	glVertex3f(-w/2+x,-h/2+y,d/2+z);
	glVertex3f(w/2+x,h/2+y,d/2+z);
	glVertex3f(w/2+x,-h/2+y,d/2+z);
	glEnd();
	glBegin(GL_QUADS);
	glVertex3f(-w/2+x,h/2+y,-d/2+z);
	glVertex3f(-w/2+x,h/2+y,d/2+z);
	glVertex3f(w/2+x,h/2+y,d/2+z);
	glVertex3f(w/2+x,h/2+y,-d/2+z);
	glVertex3f(-w/2+x,-h/2+y,d/2+z);
	glVertex3f(-w/2+x,-h/2+y,-d/2+z);
	glVertex3f(w/2+x,-h/2+y,-d/2+z);
	glVertex3f(w/2+x,-h/2+y,d/2+z);
	glEnd();
}

////////////////////////////////////////////////////////////////////////////////
//TDTEXTPANE METHODS
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//TDBUTTON METHODS
////////////////////////////////////////////////////////////////////////////////

tdButton::tdButton(float x, float y, float width, float height, float depth, int actioncode, string label, float textsize, bool leftjust)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->cdepth = depth;
	this->depth = depth;
	this->cursor = false;
	this->pushed = false;
	this->actioncode = actioncode;
	this->label = label;
	this->textsize = textsize;
	this->leftjust = leftjust;
}

void tdButton::draw()
{
	glPushMatrix();
	colorPanel();
	tdDrawBox(x, y, cdepth/2, width, height, cdepth);
	if(label != "")
	{
		glTranslatef(x-label.length()*textsize/(2.2728*TEXT_COMPRESSION),y-textsize/2,cdepth+0.001);
		glScalef(textsize/(119.05*TEXT_COMPRESSION),textsize/119.05,textsize/119.05);
		colorText();
		for(int i = 0; i < label.length(); i++)
			glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,label[i]);
		colorPanel();
	}
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
//TDSLIDER METHODS
////////////////////////////////////////////////////////////////////////////////

tdSlider::tdSlider(float x, float y, slidval* val, float length, float height, float depth, float width)
{
	this->x = x;
	this->y = y;
	this->pos = ar_translationMatrix(x,y,0);
	this->val = val;
	this->length = length;
	this->height = height;
	this->depth = depth;
	this->width = width;
	this->cpos = 0;
	this->dpos = 0;
	this->cursor = false;
	this->isGrab = false;
	this->wasGrab = false;
	this->actioncode = 0;
}

void tdSlider::draw()
{
	colorPanel();
	glPushMatrix();
	glMultMatrixf(pos.v);
	glBegin(GL_QUAD_STRIP);
	for(int i = 100; i >= 0; i--)
	{
		glVertex3f(-length/2-width/2,depth*cos(i*M_PI/100)/1.5,depth*sin(i*M_PI/100)/1.5);
		glVertex3f(length/2+width/2,depth*cos(i*M_PI/100)/1.5,depth*sin(i*M_PI/100)/1.5);
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(-length/2-depth/2-width/2,0,0);
	for(int i = 100; i >= 0; i--)
		glVertex3f(-length/2-width/2,depth*cos(i*M_PI/100)/1.5,depth*sin(i*M_PI/100)/1.5);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(length/2+depth/2+width/2,0,0);
	for(int i = 0; i < 101; i++)
		glVertex3f(length/2+width/2,depth*cos(i*M_PI/100)/1.5,depth*sin(i*M_PI/100)/1.5);
	glEnd();
	tdDrawBox(cpos-length/2,0,depth/2,width,height,depth);

	glPopMatrix();
}

void tdSlider::update(double time)
{
	if(dpos < 0) dpos = 0;
	if(dpos > length) dpos = length;
	if(cpos < dpos)
	{
		cpos += MENU_SPEED * time;
		if(cpos > dpos)
			cpos = dpos;
	}
	else if(cpos > dpos)
	{
		cpos -= MENU_SPEED * time;
		if(cpos < dpos)
			cpos = dpos;
	}

	cursor = false;
	wasGrab = isGrab;
	isGrab = false;
}

arVector3 tdSlider::handlePointer(arVector3 endpt)
{
	if(wasGrab)
	{
		dpos = endpt.v[0] - x + length/2;
		return arVector3(x - length/2 + cpos, y, depth);
	}
	if(endpt.v[0] >= x - length/2 + cpos - width/2 && endpt.v[0] <= x - length/2 + cpos + width/2 && endpt.v[1] >= y - height/2 && endpt.v[1] <= y + height/2)
	{
		cursor = true;
		return arVector3(endpt.v[0],endpt.v[1],endpt.v[2]+depth);
	}
	return arVector3(0,0,9001);
}

void tdSlider::handleEvents(tdMenuController* ct, int menu, int panel, int object)
{
	if(wasGrab)
		handleEvent(ct,menu,panel,object,TD_SLIDER_DRAG,this);
	else if(cursor)
		handleEvent(ct,menu,panel,object,TD_SLIDER_CURSOR,this);
	else
		handleEvent(ct,menu,panel,object,TD_SLIDER_IDLE,this);
}

void tdSlider::change(int code, float value, string msg)
{
	switch(code)
	{
	case TD_UPDATE:
		cout << dpos;
		dpos = (val->val - val->start) * length / (val->end - val->start);
		break;
	case TD_GRAB:
		isGrab = true;
		val->val = (cpos * (val->end - val->start) / length) + val->start;
		break;
	case TD_SETVAL:
		dpos = value;
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
		glDisable(GL_LIGHTING);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();	//move to panel position
		glMultMatrixf(cmat.v);
		glMultMatrixf(tmat.v);
		//draw the panel
		//glPushMatrix();	
		//glScalef(PANEL_THICKNESS + cwidth, PANEL_THICKNESS + cheight, PANEL_THICKNESS);
		//glTranslatef(0,0,-0.5);
		colorPanel();
		//glutSolidCube(1);
		tdDrawBox(0, 0, -PANEL_THICKNESS/2, PANEL_THICKNESS + cwidth, PANEL_THICKNESS + cheight, PANEL_THICKNESS);
		//glPopMatrix();
		if(cwidth > 0 && cheight > 0)
		{
			//glMultMatrixf(ar_translationMatrix(-cwidth/2, -cheight/2,0).v);	//would set origin to bottom-left corner
			glScalef(cwidth/panew, cheight/paneh, 1);	//proportions drawing matrix to panel's current state of compression
			for(int i = 0; i < objects.size(); i++)
			{
				objects[i]->draw();
			}
		}
		colorText();
		glScalef(1/104.76,1/104.76,1/104.76);
		glTranslatef(0,0,1);
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,"WASSUP"[0]);
		//glTranslatef(1.0476,0,0);
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,"WASSUP"[1]);
		//glTranslatef(1.0476,0,0);
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,"WASSUP"[2]);
		//glTranslatef(1.0476,0,0);
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,"WASSUP"[3]);
		//glTranslatef(1.0476,0,0);
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,"WASSUP"[4]);
		//glTranslatef(1.0476,0,0);
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN,"WASSUP"[5]);
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
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
	arVector3 endpt = arVector3(0,0,9001);
	for(int i = 0; i < panels.size(); i++)
	{
		if(panels[i]->isOpen())
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
		if(panels[i]->isOpen())
			panels[i]->handleEvents(ct, menu, i);
}

////////////////////////////////////////////////////////////////////////////////
//TDPLAYBACKMENU METHODS
////////////////////////////////////////////////////////////////////////////////

