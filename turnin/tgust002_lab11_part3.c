/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #3
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

enum N_States{NSM_Start,NSM_Wait,NSM_Increment,NSM_Decrement,NSM_Reset,NSM_Off,NSM_Press}N_State;
enum Q_States{QSM_Start,QSM_Wait,QSM_Increment,QSM_Decrement,QSM_Reset,QSM_Off,QSM_Press}Q_State;
enum O_States{OSM_Start,OSM_C1,OSM_C2,OSM_Blink}O_State;
enum R_States{RSM_Start,RSM_B1,RSM_B2,RSM_B3,RSM_B4,RSM_B5}R_State;
enum T_States{TSM_Start,TSM_B1,TSM_B2}T_State;
unsigned char go;
unsigned char go2;

int T_Tick(int state){
	static unsigned char l3;
	switch(state){
		case TSM_Start:
			l3=0x01;
			state=TSM_B1;
			break;
		case TSM_B1:
			l3=l3<<1;
			if(l3==0x80){
				state=TSM_B2;
			}
			break;
		case TSM_B2:
			l3=0x01;
			state=TSM_B1;
			break;
		default:
			l3=0x01;
			state=TSM_B1;
	}
	if(go==0x03)transmit_data(l3,1);
	if(go2==0x03)transmit_data(l3,2);
	return state;
}

int R_Tick(int state){
	static unsigned char l2;
	switch(state){
		case RSM_Start:
			l2=0x81;
			state=RSM_B1;
			break;
		case RSM_B1:
			l2=0x42;
			state=RSM_B2;
			break;
		case RSM_B2:
			l2=0x24;
			state=RSM_B3;
			break;
		case RSM_B3:
			l2=0x18;
			state=RSM_B4;
			break;
		case RSM_B4:
			l2=0x00;
			state=RSM_B5;
			break;
		case RSM_B5:
			l2=0x81;
			state=RSM_B1;
			break;
		default:
			l2=0x81;
			state=RSM_B1;
	}
	if(go==0x02)transmit_data(l2,1);
	if(go2==0x02)transmit_data(l2,2);
	return state;
}

int O_Tick(int state){
	static unsigned char l1;
	switch(state){
		case OSM_Start:
			l1=0x55;
			state=OSM_C1;
			break;
		case OSM_C1:
			l1=0xAA;
			state=OSM_C2;
			break;
		case OSM_C2:
			l1=0xFF;
			state=OSM_Blink;
			break;
		case OSM_Blink:
			l1=0x55;
			state=OSM_C1;
			break;
		default:
			l1=0x55;
			state=OSM_C1;
	}
	if(go==0x01)transmit_data(l1,1);
	if(go2==0x01)transmit_data(l1,2);
	return state;
}

int N_Tick(int state){
	static unsigned char num;
	switch (state){
		case NSM_Start:
			num=0x01;
			state=NSM_Wait;
			break;
		case NSM_Wait:
			if(((~PINA)&0x03)==0x01){
				if(num<0x03){
					num++;
				}
				state=NSM_Increment;
			}
			else if(((~PINA)&0x03)==0x02){
				if(num>0x01){
					num--;
				}
				state=NSM_Decrement;
			}
			else if(((~PINA)&0x03)==0x03){
				num=0x00;
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
                                if(num<0x03){
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
			if((~PINA&0x03)==0x00){
				transmit_data(0x00,1);
				num=0x00;
				state=NSM_Off;}
			break;
		case NSM_Off:
			if((~PINA&0x03)==0x03){
				num=0x01;
				state=NSM_Press;
			}
			break;
		case NSM_Press:
			if((~PINA&0x03)==0x00){
				state=NSM_Wait;
			}
			break;
		default:
			num=0;
			state=NSM_Wait;
	}
	go=num;
	return state;
}

int Q_Tick(int state){
        static unsigned char num2;
        switch (state){
                case QSM_Start:
                        num2=0x01;
                        state=QSM_Wait;
                        break;
                case QSM_Wait:
                        if(((~PINA>>2)&0x03)==0x01){
                                if(num2<0x03){
                                        num2++;
                                }
                                state=QSM_Increment;
                        }
                        else if(((~PINA>>2)&0x03)==0x02){
                                if(num2>0x01){
                                        num2--;
                                }
                                state=QSM_Decrement;
                        }
                        else if((~PINA>>2&0x03)==0x03){
                                num2=0x00;
                                state=QSM_Reset;
                        }
                        break;
                case QSM_Increment:
                        if(((~PINA)>>2&0x03)==0){
                                state=QSM_Wait;
                        }
                        else if(((~PINA)>>2&0x03)==0x02){
                                if(num2>0){
                                        num2--;
                                }
                                state=QSM_Decrement;
                        }
                        else if(((~PINA)>>2&0x03)==0x03){
                                num2=0;
                                state=QSM_Reset;
                        }
                        break;
                case QSM_Decrement:
                        if(((~PINA)>>2&0x03)==0){
                                state=QSM_Wait;
                        }
                        else if(((~PINA)>>2&0x03)==0x01){
                                if(num2<0x03){
                                        num2++;
                                }
                                state=QSM_Increment;
                        }
                        else if(((~PINA)>>2&0x03)==0x03){
                                num2=0;
                                state=QSM_Reset;
                        }
                        break;
                case QSM_Reset:
                        if((~PINA>>2&0x03)==0x00){
                                transmit_data(0x00,2);
                                num2=0x00;
                                state=QSM_Off;}
                        break;
                case QSM_Off:
                        if((~PINA>>2&0x03)==0x03){
                                num2=0x01;
                                state=QSM_Press;
                        }
                        break;
                case QSM_Press:
                        if((~PINA>>2&0x03)==0x00){
                                state=QSM_Wait;
                        }
                        break;
                default:
                        num2=0;
                        state=NSM_Wait;
        }
        go2=num2;
        return state;
}


void transmit_data(unsigned char data, unsigned char s){
	int i;
	if (s==0x01){
	for (i=0;i<8;i++){
		//Sets SRCLR to 1 allowing data to be set
		//Also clears SRCLk in preparation of sending data
		PORTC=0x08;
		//set SET=nex bit of data to be sent
		PORTC|=((data>>i)&0x01);
		//set SRCLK=1. Rising edge shifts next bit of data into the shift register
		PORTC|=0x02;
	}
	//set RCLK=1. Rising edge copies data from "Shift" register to "Storage register
	PORTC|=0x04;
	//clears all lines in perpartion of a new transmission
	PORTC|=0x00;}
	else if(s==0x02){
	for(i=0;i<8;i++){
                //Sets SRCLR to 1 allowing data to be set
                //Also clears SRCLk in preparation of sending data
                PORTC=0x20;
                //set SET=nex bit of data to be sent
                PORTC|=((data>>i)&0x01);
                //set SRCLK=1. Rising edge shifts next bit of data into the shift register
                PORTC|=0x02;
        }
        //set RCLK=1. Rising edge copies data from "Shift" register to "Storage" register
        PORTC|=0x10;
        //clears all lines in perpartion of a new transmission
        PORTC|=0x00;}

}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC=0xFF; PORTC=0x00;
    DDRA=0x00; PORTA=0xFF;
    static task task1,task2,task3,task4,task5;
    task *tasks[]={&task1,&task2,&task3,&task4,&task5};
    const unsigned short numTasks=sizeof(tasks)/sizeof(task*);
    /* Insert your solution below */
    task1.state=NSM_Start;
    task1.period=1;
    task1.elapsedTime=task1.period;
    task1.TickFct=&N_Tick;
    task2.state=OSM_Start;
    task2.period=400;
    task2.elapsedTime=task2.period;
    task2.TickFct=&O_Tick;
    task3.state=RSM_Start;
    task3.period=200;
    task3.elapsedTime=task3.period;
    task3.TickFct=&R_Tick;
    task4.state=TSM_Start;
    task4.period=200;
    task4.elapsedTime=task4.period;
    task4.TickFct=&T_Tick;
    task5.state=QSM_Start;
    task5.period=1;
    task5.elapsedTime=task5.period;
    task5.TickFct=&Q_Tick;
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
