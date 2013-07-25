#include "arPrecompiled.h"
#define SZG_DO_NOT_EXPORT
#include "tdevents.h"
#include "tdmenucontroller.h"
#include "tdmenu.h"

void tdHandleEvent(tdMenuController* ct, int menu, int panel, int object, int code, tdObject* ob)
{
	switch(code)
	{
	case TD_BUTTON_IDLE:
		tdGetUpdate(ct,menu,panel,object,ob);
		break;
	case TD_BUTTON_CURSOR:	//button is pressed down if select button pushed, if first frame pushed performs an action
		if(ct->wand->getButton(TD_BUTTON_1))
		{
			ob->change(TD_PUSH);
			if(ct->wand->getOnButton(TD_BUTTON_1))
				tdDoAction(ct,menu,panel,object,ob);
		}
		tdGetUpdate(ct,menu,panel,object,ob);
		break;
	case TD_SLIDER_IDLE:	//sliders check and update values from controller table
		ob->change(TD_UPDATE);
		break;
	case TD_SLIDER_CURSOR:	//sliders initiate grab if button pressed
		if(ct->wand->getButton(TD_BUTTON_1))
			ob->change(TD_GRAB);
		else
			ob->change(TD_UPDATE);
		break;
	case TD_SLIDER_DRAG:	//sliders continue grab (if button pressed) and push values to controller table
		if(ct->wand->getButton(TD_BUTTON_1))
			ob->change(TD_GRAB);
		break;
	default:
		break;
	}
}

//performs scripted actions
void tdDoAction(tdMenuController* ct, int menu, int panel, int object, tdObject* ob)
{
	int c = ob->getAction();
	switch(c)
	{
	case TD_A_SWITCHMENU2:
		ct->swap(2);
		break;

		//Playback controls:
	case TD_A_PLAYPAUSE:
		if(ct->vars.playstatus == 0)
			ct->vars.playstatus = 1;
		else
			ct->vars.playstatus = 0;
		break;
	case TD_A_REWIND:
		if(ct->vars.playstatus == 0)
			ct->vars.time->val -= 1000;
		else if(ct->vars.playstatus == 2)
			ct->vars.playstatus = 1;
		else
			ct->vars.playstatus = 2;
		break;
	case TD_A_FASTFWD:
		if(ct->vars.playstatus == 0)
			ct->vars.time->val += 1000;
		else if(ct->vars.playstatus == 3)
			ct->vars.playstatus = 1;
		else
			ct->vars.playstatus = 3;
		break;
	case TD_A_REVERSE:
		ct->vars.playreverse = !ct->vars.playreverse;
		break;

		//Special codes:
	case TD_A_LISTBUTTON:
		ct->getMenu(menu)->getPanel(panel)->respond(ct,menu,panel,object);
		break;
	default:
		break;
	}
}

void tdGetUpdate(tdMenuController* ct, int menu, int panel, int object, tdObject* ob)
{
	int c = ob->getAction();
	switch(c)
	{
	case TD_A_PLAYPAUSE:
		if(ct->vars.playstatus == 0)
			ob->change(TD_SETTEXT,0,"Play");
		else
			ob->change(TD_SETTEXT,0,"Pause");
		break;
	case TD_A_REWIND:
		if(ct->vars.playstatus == 0)
			ob->change(TD_SETTEXT,0,"Back");
		else if(ct->vars.playstatus == 2)
			ob->change(TD_SETTEXT,0,"Stop");
		else
			ob->change(TD_SETTEXT,0,"Rewind");
		break;
	case TD_A_FASTFWD:
		if(ct->vars.playstatus == 0)
			ob->change(TD_SETTEXT,0,"Skip");
		else if(ct->vars.playstatus == 3)
			ob->change(TD_SETTEXT,0,"Stop");
		else
			ob->change(TD_SETTEXT,0,"FastFwd");
		break;



	default:
		break;
	}
}