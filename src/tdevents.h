#ifndef TDEVENTS_H_
#define TDEVENTS_H_

//Wand keybindings:
#define TD_MENU_TOGGLE 0
#define TD_BUTTON_1 1
#define TD_BUTTON_2 2
#define TD_BUTTON_3 3
#define TD_BUTTON_4 4
#define TD_JOYSTICK_BUTTON 5

//Action codes:	//EDIT SECTION FOR MENU CUSTOMIZATION//
#define TD_A_SWITCHMENU2	2364
#define TD_A_BACKSKIP		3720
#define TD_A_PLAYPAUSE		8743
#define TD_A_FWDSKIP		7369

//Event codes:
#define TD_BUTTON_IDLE		2378
#define TD_BUTTON_CURSOR	8534
#define TD_SLIDER_IDLE		5902
#define TD_SLIDER_CURSOR	6251
#define TD_SLIDER_DRAG		2369

//Change codes:
#define TD_PUSH				2590
#define TD_GRAB				5656
#define TD_UPDATE			3475
#define TD_SETVAL			2568
#define TD_SETTEXT			6738

class tdMenuController;
class tdObject;

void handleEvent(tdMenuController* ct, int menu, int panel, int object, int code, tdObject* ob);
void doAction(tdMenuController* ct, int menu, int panel, int object, tdObject* ob);

#endif