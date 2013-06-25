#include "tdevents.h"
#include "tdmenucontroller.h"
#include "tdmenu.h"

void handleEvent(tdMenuController* ct, int menu, int panel, int object, int code, tdObject* ob)
{
	switch(code)
	{
	case TD_BUTTON_IDLE:
		break;
	case TD_BUTTON_CURSOR:	//button is pressed down if select button pushed, if first frame pushed performs an action
		if(ct->wand->getButton(TD_BUTTON_1))
		{
			ob->change(TD_PUSH);
			if(ct->wand->getOnButton(TD_BUTTON_1))
				doAction(ct,menu,panel,object,ob);
		}
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
void doAction(tdMenuController* ct, int menu, int panel, int object, tdObject* ob)
{
	int c = ob->getAction();
	switch(c)
	{
	case TD_A_SWITCHMENU2:
		ct->swap(2);
		break;
	case TD_A_BACKSKIP:
		ct->vars.time->val -= 100;
		break;
	case TD_A_PLAYPAUSE:

		/*
		if(ct->vars.pause)
		{ct->vars.pause = false; ob->change(TD_SETTEXT,0,"Pause");}
		else
		{ct->vars.pause = true; ob->change(TD_SETTEXT,0,"Play");}
		*/
		break;
	case TD_A_FWDSKIP:
		ct->vars.time->val += 100;
		break;
	default:
		break;
	}
}