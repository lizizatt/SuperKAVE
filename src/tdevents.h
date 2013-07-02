#ifndef TDEVENTS_H_
#define TDEVENTS_H_

#include "tdcodes.h"

class tdMenuController;
class tdObject;

//Handles 
void tdHandleEvent(tdMenuController* ct, int menu, int panel, int object, int code, tdObject* ob);
void tdDoAction(tdMenuController* ct, int menu, int panel, int object, tdObject* ob);	//for scripted actions from input
void tdGetUpdate(tdMenuController* ct, int menu, int panel, int object, tdObject* ob);	//for stuff that changes without user input

#endif