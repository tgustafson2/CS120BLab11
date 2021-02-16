/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #4
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

enum N_States{NSM_Start,NSM_Wait,NSM_Increment,NSM_Decrement,NSM_Reset}N_State;
enum O_States{OSM_Start,OSM_Output}O_State;
//unsigned char num;


int N_Tick(int state){
	static unsigned short num;
	switch (state){
		case NSM_Start:
			num=0x4F00;
			state=NSM_Wait;
			break;
		case NSM_Wait:
			if(((~PINA)&0x03)==0x01){
				if(num<0x7FFF){
					num++;
				}
				state=NSM_Increment;
			}
			else if(((~PINA)&0x03)==0x02){
				if(num>0){
					num--;
				}
				state=NSM_Decrement;
			}
			else if(((~PINA)&0x03)==0x03){
				num=0;
				state=NSM_Reset;
			}
			break;
		case NSM_Increment:
			if(((~PINA)&0x03)==0){
				state=NSM_Wait;
			}
			else if(((~PINA)&0x03)==0x02){
                                if(num>0){
                                        num--;
                                }
                                state=NSM_Decrement;
                        }
			else if(((~PINA)&0x03)==0x03){
                                num=0;
                                state=NSM_Reset;
                        }
                        break;
		case NSM_Decrement:
			if(((~PINA)&0x03)==0){
                                state=NSM_Wait;
                        }
                        else if(((~PINA)&0x03)==0x01){
                                if(num<0x7FFF){
                                        num++;
                                }
                                state=NSM_Increment;
                        }
                        else if(((~PINA)&0x03)==0x03){
                                num=0;
                                state=NSM_Reset;
                        }
                        break;
		case NSM_Reset:
			state=NSM_Wait;
			break;
		default:
			num=0;
			state=NSM_Wait;
	}
	transmit_data(num);
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
    static task task1;
    task *tasks[]={&task1};
    const unsigned short numTasks=sizeof(tasks)/sizeof(task*);
    /* Insert your solution below */
    task1.state=NSM_Start;
    task1.period=1;
    task1.elapsedTime=task1.period;
    task1.TickFct=&N_Tick;
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
