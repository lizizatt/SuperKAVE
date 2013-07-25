//This file holds all constants and codes throughout the tdmenu system.
#ifndef TD_CODES_H
#define TD_CODES_H

//Menu drawing constants:
#define MENU_SPEED				0.02	//the speed at which menu animations run
#define PANEL_THICKNESS			0.02	//how thick each panel is (and how big the pellet is during open/close)
#define TEXT_COMPRESSION		1.5		//how text is "squished" on the x-axis
#define BTNGAP					0.01	//the gaps between buttons
#define MENU_OFFSET				2.0

//Wand keybindings:
#define TD_MENU_TOGGLE			4
#define TD_BUTTON_1				1
#define TD_BUTTON_2				0
#define TD_BUTTON_3				2
#define TD_BUTTON_4				3
#define TD_JOYSTICK_BUTTON		5

//Action codes:	//EDIT SECTION FOR MENU CUSTOMIZATION//
#define TD_A_SWITCHMENU2		2364

	//playback controls
#define TD_A_PLAYPAUSE			8743
#define TD_A_REWIND				2567
#define TD_A_FASTFWD			1798
#define TD_A_REVERSE			6029

	//special action codes, used in core program (don't touch)
#define TD_A_LISTBUTTON			5897

//Event codes:		
#define TD_BUTTON_IDLE			2378
#define TD_BUTTON_CURSOR		8534
#define TD_SLIDER_IDLE			5902
#define TD_SLIDER_CURSOR		6251
#define TD_SLIDER_DRAG			2369

//Change codes:
#define TD_PUSH					2590
#define TD_GRAB					5656
#define TD_UPDATE				3475
#define TD_SETVAL				2568
#define TD_SETTEXT				6738

#endif