//Header files
#include <at89c5131.h>
#include <stdio.h>	//for sprintf functions

//Bit definitions
sbit RS=P0^0;
sbit RW=0x81;	//Also can use P0^1 or 0x80^1
sbit EN=P0^2;

//Global variables
unsigned char addr = 0x80;

//LCD functions
void lcd_init(void);
void lcd_cmd(unsigned int i);
void lcd_char(unsigned char ch);
void lcd_write_string(unsigned char *s);
void port_init(void);

//Extern functions from assembly files
extern void timer_init(void);
extern void init(void);
extern void timer0_int(void);

//Timer 0 ISR
void Timer0_Interrupt(void) interrupt 1
{
			timer0_int();
}


//Delay function for time*1ms
void msdelay(unsigned int time)
{
	int i,j;
	for(i=0;i<time;i++)
	{
		for(j=0;j<318;j++);
	}
}
//LCD utility functions
void lcd_cmd(unsigned int i)
{
	RS=0;
	RW=0;
	EN=1;
	P2=i;
	msdelay(10);
	EN=0;
}
void lcd_char(unsigned char ch)
{
	RS=1;
	RW=0;
	EN=1;
	P2=ch;
	msdelay(10);
	EN=0;
}
void lcd_write_string(unsigned char *s)
{
	while(*s!='\0')		//Can use while(*s)
	{
		lcd_char(*s++);
	}
}
void lcd_init(void) using 3
{
	lcd_cmd(0x38);
	msdelay(4);
	lcd_cmd(0x06);
	msdelay(4);
	lcd_cmd(0x0C);
	msdelay(4);
	lcd_cmd(0x01);
	msdelay(4);
}

//Port initialization
void port_init(void)
{
	P2=0x00;
	EN=0;
	RS=0;
	RW=0;
}

//Main function
int main(void)
{
	unsigned char str[]="IITB";
	unsigned char str2[16];
	unsigned int loop_count = 0;
	port_init();
	lcd_init();
	init();
	timer_init();
	lcd_cmd(0x80);
	lcd_write_string(str);
	while(1)
	{
		lcd_cmd(0x80);
		sprintf(str2,"Count:%d",loop_count);
		lcd_write_string(str2);
		loop_count+=5;
		msdelay(250);
//Shift display to the right		
//		lcd_cmd(0x1C);
//		msdelay(250);
	}
}
	