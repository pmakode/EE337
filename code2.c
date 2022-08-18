//Header files
#include <at89c5131.h>
#include <stdio.h>  
#include <math.h>
#include <lcd.h>
// IE defintion
sfr IE  = 0xA8;

//Global variables
unsigned char addr = 0x80;
unsigned int waiting_count = 0;
unsigned int current_count = 0;
unsigned int overflow_count_for_distance = 0;
unsigned int pre_ocfd = 0;
float speed_auto = 0;
float fare_for_travel = 21;
float fare_for_waiting = 0;
float total_fare = 0;
float distance_travelled = 0;
code unsigned char mystr1[] = "Mumbai Auto";
code unsigned char mystr2[] = "For Hire";
code unsigned char mystr3[] = "Waiting";
code unsigned char mystr5[] = "360 Km/h";
code unsigned char mystr6[] = "720 Km/h" ;
code unsigned char mystr7[] = "1800 Km/h";
code unsigned char mystr8[] = "3600 Km/h";
unsigned int valu_disp_ind = 0;
//unsigned char mystr4[8];
char mystr4[6]={0,0,0,0,0,'\0'};
void port_init(void);
//Extern functions from assembly files
extern void timer_init(void);
extern void init(void);
extern void timer0_int(void);
//extern void timer1_int(void);
//Timer 0 ISR
void Timer0_Interrupt(void);
//Timer 1 ISR
void Timer1_Interrupt(void);
//Port initialization
void port_init(void)
{	P2=0x00;
	EN=0;
	RS=0;
	RW=0;
}


void Timer0_Interrupt(void) interrupt 1 //timer 0 ISR
{	TR0 = 0;							//stop timer0
	current_count = waiting_count;		//store previous overflow value
	waiting_count = waiting_count+1;	//update current overflow value
	TL0 = 0xB0;							//load timer 0 with 15536, so that it count up 50k times, 
	TH0 = 0x3C;							//and 40 such counts would give 1 sec
	TR0 = 1;							//start timer0
}

void Timer1_Interrupt(void) interrupt 3 //timer 1 ISR similar to timer 0 isr
{	TR1 = 0;
	pre_ocfd = overflow_count_for_distance;
	overflow_count_for_distance = overflow_count_for_distance+1;
	TL1 = 0xB0;
	TH1 = 0x3C;
	TR1 = 1;
}

void actual_fare(void){				
	unsigned int rounded_count = waiting_count/200;		//waiting charges added only after 5 seconds, therefore /200 gives essentially the number of '5 seconds' 
	unsigned int rounded_count_for_distance = overflow_count_for_distance/40;  // travelling charge added every one second
	float rounded_value;		
	float temp_val = 0;
	fare_for_waiting = 1.42*rounded_count;	//waiting charge
	distance_travelled=distance_travelled+speed_auto*1;
	if(distance_travelled>1.5)		//check if distace travelled>1.5 kms
	{
		temp_val = distance_travelled-1.5;
	}
	else{
		temp_val = 0;
	}

	if(P1_3==0){fare_for_travel = 21 + 14.2*temp_val;}
	if(P1_3==1){fare_for_travel = fare_for_travel;}//fare for travelling
	
	total_fare = fare_for_waiting + fare_for_travel;	//total fare for trip
	rounded_value = floor(total_fare);		//rounding off the total fare for the trip
	if((total_fare - rounded_value)>0.49){
		total_fare=rounded_value+1;
	}
	else{
		total_fare=rounded_value;
	}
	//iske baad lcd pr total fare display hoga
	int_to_string(total_fare,mystr4);	//converting total fare to string
	//sprintf(mystr4,"%d",(unsigned int) (total_fare));
	lcd_cmd(0x01);		//clear lcd
	msdelay(2);		//delay 2ms
	lcd_cmd(0x84);	//place cursor on lcd
	msdelay(2);		//delay 2ms
	lcd_write_string(mystr4);	//write fare on lcd
	lcd_cmd(0xC5);			// place cursor to write 'waiting' or 'speed of auto'
	if(valu_disp_ind == 1)	//write appropriate string according to the value of valu_disp_ind
		{lcd_write_string(mystr3);}
	else if(valu_disp_ind == 2)
		{lcd_write_string(mystr5);}
	else if(valu_disp_ind == 3)
		{lcd_write_string(mystr6);}
	else if(valu_disp_ind == 4)
		{lcd_write_string(mystr7);}
	else 
		{lcd_write_string(mystr8);}
	//actual fare ends
}



//Main function
int main()
{	P1=0x0F;			//initialize port 1
	//P1=0xF0;		
	TMOD = 0x11;        //mode 1 of both timers
	TL0 = 0xB0;
	TH0 = 0x3C;
	lcd_init();

	while(1){

	while(P1_0==0)         //auto is available, since trip has not started, resetting all variables
	{
		waiting_count = 0;					//count of timer0 overflow
		current_count = 0;					// count for previous value of timer0 overflow
		overflow_count_for_distance = 0;	//count for timer1 overflow
		pre_ocfd = 0;						//count for previous value of timer1 overflow
		speed_auto = 0;						//speed of auto, 0 since it is stationary
		fare_for_travel = 21;				//minimum fare for travel if auto is hired
		fare_for_waiting = 0;				//fare for waiting
		total_fare = 0;						//total fare for the trip
		distance_travelled = 0;				//distance of the trip
		lcd_cmd(0x01);
		lcd_cmd(0x82);						//displaying mumbai auto, for hire
		lcd_write_string(mystr1);
		lcd_cmd(0xC4);
		lcd_write_string(mystr2);
		
	}

	while(P1_0==1){             	  //auto currently hired
		if(P1_3==1)      			 //auto is waiting
		{	valu_disp_ind = 1;
			IE = 0x82;
			TR0 = 1;
			while(P1_3==1 && P1_0==1){	 // till auto is waiting and it is hired, waiting charges increase
				if(((waiting_count/200) - (current_count/200))==1)
				{actual_fare();}
			}
			TR0 = 0; // auto no longer waiting, therefore stop timer 0
		}          

		//auto is travelling
		if(P1_3==0){	
		TL1 = 0xB0;		//mode 1 of both timers by tmod in line 127
		TH1 = 0x3C;
		IE = 0x88;
		TR1 = 1;

		while(P1_3==0 && P1_0==1) //till auto is NOT waiting and it is hired, travelling charges increase
		{
			if(((overflow_count_for_distance/40)-(pre_ocfd/40))==1) // if time elapsed is one sec
			{   
				if(P1_2==0 && P1_1==0){	//find value of speed
					valu_disp_ind = 2;
					speed_auto = 0.1;}
				if(P1_2==0 && P1_1==1){	
					valu_disp_ind = 3;
					speed_auto  = 0.2;}					
				if(P1_2==1 && P1_1==0){	
					valu_disp_ind = 4;
					speed_auto  = 0.5;}
				if(P1_2==1 && P1_1==1){	
					valu_disp_ind = 5;
					speed_auto  = 1;}
					
					actual_fare();	// caculate and display fare	
				}
				
				}
				TR1 = 0;		//since not travelling, therefore stop timer 1
			}
								
			speed_auto = 0;	//auto not travelling, therefore speed = 0
								
	}
} // loopback to while(1)

return 0;
}	