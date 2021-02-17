/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #5
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <timer.h>
#include <scheduler.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum N_States{NSM_Start,NSM_Wait,NSM_Left,NSM_Right,NSM_JumpL,NSM_JumpR}N_State;
enum A_States{ASM_Start,ASM_Left,ASM_Right}A_State;
enum O_States{OSM_Start,OSM_Output,OSM_Blink}O_State;
unsigned short num;
unsigned short ai;

int O_Tick(int state){
	unsigned short op;
	static unsigned short count;
	switch(state){
		case OSM_Start:
			count=0;
			op=num|ai;
			state=OSM_Output;
			break;
		case OSM_Output:
			if(num!=ai){
				op=num|ai;
			}
			else{
				op=0x7FFF;
				state=OSM_Blink;
			}
			break;
		case OSM_Blink:
			if(count%200>=100)op=0x7FFF;
			else op=0x0000;
			if(count>=2000){
				num=0x4000;
				ai=0x0001;
				state=OSM_Output;
				op=num|ai;
				count=0;
			}
			count++;
			break;
		default:
			op=num|ai;
			state=OSM_Output;
	}
	transmit_data(op);
	return state;
}

int A_Tick(int state){
	switch (state){
		case ASM_Start:
			ai=0x0001;
			state=ASM_Left;
			break;
		case ASM_Left:
			if(ai<0x4000)
			ai=ai<<1;
			else{
			ai=ai>>1;
			state=ASM_Right;
			}
			break;
		case ASM_Right:
			if(ai>0x0001){
				ai=ai>>1;
			}
			else{
				ai=ai<<1;
				state=ASM_Left;
			}
			break;
		default:
			ai=0x0001;
			state=ASM_Left;
	}
	return state;
}

int N_Tick(int state){
	switch (state){
		case NSM_Start:
			num=0x4000;
			state=NSM_Wait;
			break;
		case NSM_Wait:
			if(((~PINA)&0x07)==0x01){
				if(num>0x0001){
					num=num>>1;
				}
				state=NSM_Right;
			}
			else if(((~PINA)&0x07)==0x04){
				if(num<0x4000){
					num=num<<1;
				}
				state=NSM_Left;
			}
			else if(((~PINA)&0x07)==0x03){
				if(num>0x0010){
				num=num>>4;}
				else if(num>0x0008)num=num>>3;
				else if(num>0x0004)num=num>>2;
				else if(num>0x0002)num=num>>1;
				state=NSM_JumpR;
			}
			else if(((~PINA)&0x07)==0x06){
				if(num<0x0800)num=num<<4;
				else if(num<0x1000)num=num<<3;
				else if(num<0x2000)num=num<<2;
				else if(num<0x4000)num=num<<1;
				state=NSM_JumpL;
			}
			break;
		case NSM_Left:
			if(((~PINA)&0x07)==0x00){
				state=NSM_Wait;
			}
			else if(((~PINA)&0x07)==0x01){
                                if(num>0x0001){
                                        num=num>>1;
                                }
				state=NSM_Right;
                        }
			else if(((~PINA)&0x07)==0x03){
                                if(num>0x0080)num=num<<4;
                                else if(num>0x004)num=num<<3;
                                else if(num>0x0002)num=num<<2;
                                else if(num>0x0001)num=num<<1;
                                state=NSM_JumpR;
                        }
                        break;
		case NSM_Right:
			if(((~PINA)&0x07)==0x00){
                                state=NSM_Wait;
                        }
                        else if(((~PINA)&0x07)==0x04){
                                if(num<0x4000){
                                        num=num<<1;
                                }
				state=NSM_Left;
                        }
                        else if(((~PINA)&0x07)==0x06){
                                if(num<0x0800){
                                num=num>>4;}
                                else if(num<0x1000)num=num>>3;
                                else if(num<0x2000)num=num>>2;
                                else if(num<0x4000)num=num>>1;
                                state=NSM_JumpL;
                        }
                        break;
		case NSM_JumpR:
			if(((~PINA)&0x07)==0x00)
			state=NSM_Wait;
			break;
		case NSM_JumpL:
			if(((~PINA)&0x07)==0x00)
                        state=NSM_Wait;
                        break;
		default:
			num=0x4000;
			state=NSM_Wait;
	}
	//transmit_data(num);
	return state;
}

void transmit_data(unsigned short data){
	int i,j;
	for (i=0;i<16;i++){
		//Sets SRCLR to 1 allowing data to be set
		//Also clears SRCLK in preparation of sending data
		PORTC=0x48;
		//set SER=next bit of data to be sent.
		PORTC|=((data>>i)&0x01);
		//set SRCLK=1. Rising edge shifts next bit of data into the shift register
		PORTC|=0x02;
		PORTC|=0x10;
		//PORTC=0x48;
		//PORTC|=0x02;
		//PORTC|=0x10;
	}
	//set RCLK=1. Rising edge copies data from Shift register to "Storage" register
	PORTC=0x42;
	PORTC|=0x10;
	PORTC=0x00;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC=0xFF; PORTC=0x00;
    DDRA=0x00; PORTA=0xFF;
    static task task1,task2,task3;
    task *tasks[]={&task1,&task2,&task3};
    const unsigned short numTasks=sizeof(tasks)/sizeof(task*);
    /* Insert your solution below */
    task1.state=NSM_Start;
    task1.period=1;
    task1.elapsedTime=task1.period;
    task1.TickFct=&N_Tick;
    task2.state=ASM_Start;
    task2.period=500;
    task2.elapsedTime=task2.period;
    task2.TickFct=&A_Tick;
    task3.state=OSM_Start;
    task3.period=1;
    task3.elapsedTime=task3.period;
    task3.TickFct=&O_Tick;
    unsigned long int GCD=1;
    TimerSet(GCD);
    TimerOn();
    unsigned short i;
    while (1) {
	    for(i=0;i<numTasks;i++){
		    if(tasks[i]->elapsedTime>=tasks[i]->period){
			    tasks[i]->state=tasks[i]->TickFct(tasks[i]->state);
			    tasks[i]->elapsedTime=0;
		    }
		    tasks[i]->elapsedTime+=GCD;
	    }
	    while(!TimerFlag);
	    TimerFlag=0;

    }
    return 1;
}
