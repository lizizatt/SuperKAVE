#include "tdevents.h"
#include "tdmenucontroller.h"
#include "tdmenu.h"

void handleEvent(tdMenuController* ct, int menu, int panel, int object, int code, tdObject* ob)
{
	switch(code)
	{
	case TD_BUTTON_IDLE:
		break;
	case TD_BUTTON_CURSOR:
		if(ct->wand->getButton(TD_BUTTON_1))
		{
			ob->change(TD_PUSH);
			if(ct->wand->getOnButton(TD_BUTTON_1))
			{
				switch(menu)
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
	default:
		break;
	}
}