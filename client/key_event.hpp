#ifndef KEY_EVENT_HPP
#define KEY_EVENT_HPP

#include <conio.h>

/* KeyEvents holds the information of what keys have been pressed and how many times each key was pressed*/

typedef struct KeyEvents{
	
	/*enumeration for the special keys that are not on the ascii table,
	 *but are given values that fits within 128 with the ascii characters
	 */
	enum SpecialKey{
		F1, F2, F3, F4, F5, F6, F7, F8, BKS, TAB,
		F9, F10, F11, ENT, F12, HOM, INS, DEL, PUP, PDN,
		END, KU, KL, KR, KD, ESC = 27
	};
	
	unsigned char keys_pressed_[128];
} KeyEvents;

/* returns an instance of a statically allocated KeyEvents */
KeyEvents *get_key_events();

/* returns the last character inputted by the user*/
char get_char_input();

#endif