#ifndef TDEVENTS_H_
#define TDEVENTS_H_

//Button keybindings:
#define TD_MENU_TOGGLE 0
#define TD_BUTTON_1 1
#define TD_BUTTON_2 2
#define TD_BUTTON_3 3
#define TD_BUTTON_4 4
#define TD_JOYSTICK_BUTTON 5

//Event code definitions:
#define TD_BUTTON_IDLE		2378
#define TD_BUTTON_CURSOR	8534

//Change code definitions:
#define TD_PUSH				2590
#define TD_GRAB				5656
#define TD_SETTEXT			6738

class tdMenuController;
class tdObject;

void handleEvent(tdMenuController* ct, int menu, int panel, int object, int code, tdObject* ob);

#endif