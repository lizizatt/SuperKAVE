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
			{
				switch(menu)	//actions for all buttons
				{
				case 0:
					break;
				case 1:
					switch(panel)
					{
					case 0:
						switch(object)
						{
						case 0:
							ct->swap(2);
							break;
						default:
							break;
						}
						break;
					case 1:
						break;
					case 2:
						break;
					}
					break;
				case 2:
					break;
				default:
					break;
				}
			}
		}
		break;
	case TD_SLIDER_IDLE:	//sliders check and update values from controller table
		//TODO: table updates for all sliders
		break;
	case TD_SLIDER_CURSOR:	//sliders initiate grab if button pressed
		if(ct->wand->getButton(TD_BUTTON_1))
			ob->change(TD_GRAB);
		break;
	case TD_SLIDER_DRAG:	//sliders continue grab (if button pressed) and push values to controller table
		if(ct->wand->getButton(TD_BUTTON_1))
			ob->change(TD_GRAB);
		//TODO: table push for all sliders
		break;
	default:
		break;
	}
}