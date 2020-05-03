#include <Arduino.h>
#include <ctype.h>

#define CLOCK_PIN 3 // green
#define DATA_PIN  2  // white

#define KEY_CAPSLOCK		0x58
#define KEY_SPESHEL_KEY		0xE0 // 224
#define KEY_END_PRESS		0xF0 // 240
#define FLAG_SPESHEL_KEY	0x100

#define LSHIFT				0x12
#define RSHIFT				0x59
#define LEFT_KEY			0x16B
#define RIGHT_KEY			0x174
#define UP_KEY				0x175
#define DOWN_KEY			0x172
#define SELES_KEY			0x14A


uint16_t keyboard_keybuffer     = 0;
uint8_t keyboard_last_keybuffer = 0;
uint8_t keyboard_keybits        = 0;

bool capslockON  = 0;
bool shift_ON    = 0;
bool speshel_key = 0;
bool end_press   = 0;

char shiftkey[] = {0,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
					   0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
					  'Q', '!',  0 ,  0 ,  0 , 'Z', 'S', 'A', 'W', '@',
					   0 ,  0 , 'C', 'X', 'D', 'E', '$', '#',  0 ,  0 ,
					  ' ', 'V', 'F', 'T', 'R', '%',  0 ,  0 , 'N', 'B',
					  'H', 'G', 'Y', '^',  0 ,  0 ,  0 , 'M', 'J', 'U',
					  '&', '*',  0 ,  0 , '<', 'K', 'I', 'O', ')', '(',
					   0 ,  0 , '>', '?', 'L', ':', 'P', '_',  0 ,  0 ,
					   0 , '"',  0 , '{', '+',  0 ,  0 ,  0 ,  0 ,'\n',
					  '}',  0 , '|',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
					   0 ,'\b',  0 ,  0 , '1',  0 , '4', '7',  0 ,  0 ,
					   0 , '0',  0 , '2', '5', '6', '8',  0 ,  0 ,  0 ,
					  '+', '3', '-', '*', '9',  0 ,  0 ,  0 ,  0 ,  0 ,
					   0 };

char charkey [] = {0,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
					   0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
					  'q', '1',  0 ,  0 ,  0 , 'z', 's', 'a', 'w', '2',
					   0 ,  0 , 'c', 'x', 'd', 'e', '4', '3',  0 ,  0 ,
					  ' ', 'v', 'f', 't', 'r', '5',  0 ,  0 , 'n', 'b',
					  'h', 'g', 'y', '6',  0 ,  0 ,  0 , 'm', 'j', 'u',
					  '7', '8',  0 ,  0 , ',', 'k', 'i', 'o', '0', '9',
					   0 ,  0 , '.', '/', 'l', ';', 'p', '-',  0 ,  0 ,
					   0 ,  0 ,  0 , '[', '=',  0 ,  0 ,  0 ,  0 ,'\n',
					  ']',  0 ,'\\',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
					   0 ,'\b',  0 ,  0 , '1',  0 , '4', '7',  0 ,  0 ,
					   0 , '0',  0 , '2', '5', '6', '8',  0 ,  0 ,  0 ,
					  '+', '3', '-', '*', '9',  0 ,  0 ,  0 ,  0 ,  0 ,
					   0 };

bool Key_Available()
{
	return keyboard_keybits == 11 && end_press == 0 && speshel_key == 0;
}

void Interrupt_Value(void)
{
	int value = digitalRead(DATA_PIN);

	if (keyboard_keybits == 11)
	{
		keyboard_last_keybuffer = keyboard_keybuffer;
		keyboard_keybits = 0;
		keyboard_keybuffer = 0;
	}

	if (keyboard_keybits > 0 && keyboard_keybits < 9)
		keyboard_keybuffer |= (value << (keyboard_keybits - 1));

	if(keyboard_keybits == 10)
	{
	
		if(keyboard_keybuffer == KEY_CAPSLOCK && keyboard_last_keybuffer != KEY_END_PRESS)
			capslockON = !capslockON;

		if(keyboard_last_keybuffer != KEY_END_PRESS)
			end_press = 0;
		if(keyboard_keybuffer == KEY_END_PRESS)
			end_press = 1;

		if(speshel_key)
			keyboard_keybuffer |= FLAG_SPESHEL_KEY;
		if(keyboard_keybuffer == KEY_SPESHEL_KEY)
			speshel_key = 1;
		else
			speshel_key = 0;

		if(keyboard_keybuffer == LSHIFT || keyboard_keybuffer == RSHIFT)
		{
			if(end_press == 1)
				shift_ON = 0;
			else
				shift_ON = 1;
		}
	}

	keyboard_keybits++;
	
}

uint16_t Read_Key(void)
{
	uint16_t buffer = 0;
	
	while(!Key_Available());
	
	keyboard_last_keybuffer = 0;
	keyboard_keybits = 0;
	buffer = keyboard_keybuffer;
	keyboard_keybuffer = 0;
	return buffer;
}

char Key_To_Char(uint16_t key)
{
	if(key == 0x15A) key = 0x5A; // enter 2

	if(key > FLAG_SPESHEL_KEY)
		return 1;

	if((capslockON && shift_ON && charkey[key] >= 'a' && charkey[key] <= 'z') || (capslockON == 0 && shift_ON == 0))
		return charkey[key];

	if(shift_ON)
		return shiftkey[key];
	if(capslockON)
		return toupper(charkey[key]);
}

char Read_Char()
{
	return Key_To_Char(Read_Key());
}

void Keyboard_Init(void)
{
	attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), Interrupt_Value, FALLING);
	pinMode(CLOCK_PIN, INPUT);
  	digitalWrite(CLOCK_PIN, HIGH);
  	pinMode(DATA_PIN, INPUT);
  	digitalWrite(DATA_PIN, HIGH);

}
