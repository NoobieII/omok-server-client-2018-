#include "key_event.hpp"

#include <conio.h>
#include <string.h>

/*
	Used to quickly convert special characters such as arrow keys to the enumerated
	values. Keys such as tab, backspace, esc, are in the enumeration to so integer
	literals don't have to be used to represent them.
*/

const unsigned char kConvertSpecialKey[] = 
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, KeyEvents::TAB,	//0
	0, 0, 0, KeyEvents::ENT, 0, 0, 0, 0, 0, 0,	//10
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//20
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//30
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//40
	0, 0, 0, 0, 0, 0, 0, 0, 0, KeyEvents::F1,	//50
	KeyEvents::F2, KeyEvents::F3, KeyEvents::F4, KeyEvents::F5, KeyEvents::F6, KeyEvents::F7, KeyEvents::F8, KeyEvents::F9, KeyEvents::F10, 0,	//60
	0, KeyEvents::HOM, KeyEvents::KU, KeyEvents::PUP, 0, KeyEvents::KL, 0, KeyEvents::KR, 0, KeyEvents::END,	//70
	KeyEvents::KD, KeyEvents::PDN, KeyEvents::INS, KeyEvents::DEL, 0, 0, 0, 0, 0, 0,	//80
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//90
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//100
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//110
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//120
	0, 0, 0, KeyEvents::F11, KeyEvents::F12, 0, 0, 0, 0, 0	//130
};

/*
	These values will be stored in the KeyHandler class ch_input_[]
*/
const unsigned char kControlValues[] = 
{
KeyEvents::F1, KeyEvents::F2, KeyEvents::F3, KeyEvents::F4, KeyEvents::F5, KeyEvents::F6, KeyEvents::F7, KeyEvents::F8, KeyEvents::BKS, KeyEvents::TAB,
KeyEvents::F9, KeyEvents::F10, KeyEvents::F11, KeyEvents::ENT, KeyEvents::F12, KeyEvents::HOM, KeyEvents::INS, KeyEvents::DEL, KeyEvents::PUP, KeyEvents::PDN,
KeyEvents::END, KeyEvents::KU, KeyEvents::KL, KeyEvents::KR, KeyEvents::KD, 0, 0, KeyEvents::ESC, 0, 0,
0, 0, ' ', '!', '\"', '#', '$', '%', '&', '\'',	//30
'(', ')', '*', '+', ',', '-', '.', '/', '0', '1',	//40
'2', '3', '4', '5', '6', '7', '8', '9', ':', ';',	//50
'<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E',	//60
'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',	//70
'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',	//80
'Z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c',	//90
'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',	//100
'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w',	//110
'x', 'y', 'z', '{', '|', '}', '~', 0, 0, 0	//120
};

static KeyEvents key_events, key_events_buf;

KeyEvents *get_key_events(){
	
	while(kbhit()){
		unsigned char ch = getch();
		
		if(ch == 0 || ch == 224){
			ch = getch();
			key_events.keys_pressed_[kConvertSpecialKey[ch] ]++;
		}
		else{
			key_events.keys_pressed_[kControlValues[ch] ]++;
		}
	}
	
	memcpy(&key_events_buf, &key_events, sizeof(KeyEvents));
	memset(&key_events, 0, sizeof(KeyEvents));
	
	return &key_events_buf;
}

char get_char_input(){
	unsigned char ch = getch();
	
	if(ch == 0 || ch == 224){
		ch = getch();
		return kConvertSpecialKey[ch];
	}
	else{
		return (char)(ch);
	}
}