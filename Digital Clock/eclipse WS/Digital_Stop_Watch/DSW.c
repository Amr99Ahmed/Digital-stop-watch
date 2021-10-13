/*
 * DSW.c
 *
 *  Created on: Oct 13, 2021
 *      Author: Amr Ahmed Abdelghany
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* global variables to count time */
unsigned char seconds;
unsigned char minutes;
unsigned char hours;

/*
 * Timer 1
 * FOC1A & FOC1B set to 1 because non-PWM
 * initiate TCNT1
 * Set value in ICR1 to compare
 * WGM13 & WGM12 set to 1 for CTC mode
 * CS10 & CS11 set to 1 for prescaler 64
 * Fcpu = 1 Mhz
 * for 1 sec need to interrupt at 15625
 * in TIMSK -> OCIE1A set to enable Timer1 compare match interrupt
 * enable Global interrupt
 * timer1 is counting & waiting for flag to interrupt it in 1 sec
 */

void Timer1_CTC_init(void)
{
	/* function definition */
	TCCR1A = (1<<FOC1A) | (1<<FOC1B);
	TCCR1B |= (1<<WGM13) | (1<<WGM12) | (1<<CS10) | (CS11);
	TIMSK |= (1<<OCIE1A);
	TCNT1 = 0;
	ICR1 = 15624;
}

/*
 * Timer0
 * CTC mode operation
 * OCR0 = 78 for 5 msec count
 * Fcpu = 1 Mhz
 * TCNT0 = 0
 * FOC0 set to 1 for non-PWM
 * WGM01 at TCCR0 set to 1 for CTC
 * CS00 & CS01 both are ones for 64 prescaler
 * at TIMSK -> OCIE0 set to 1 for compare match interrupt enable
 */

void Timer0_CTC_init(void)
{
	/* function definition */
	TCCR0 = (1<<FOC0) | (1<<WGM01) | (1<<CS00) | (1<<CS01);
	TIMSK = (1<<OCIE0);
	TCNT0 = 0;
	OCR0 = 78;
}

void INT0_init(void)
{
	/*function definition*/
	GICR |= (1<<6);
	MCUCR |= (1<<ISC01);
	DDRD &=~(1<<2);
	PORTD |= (1<<2);
}

void INT1_init(void)
{
	/*function definition*/
	GICR |= (1<<7);
	MCUCR |= (1<<ISC11);
	DDRD &=~(1<<3);
	PORTD |= (1<<3);
}

void INT2_init(void)
{
	/*function definition*/
	GICR |= (1<<5);
	MCUCSR |= (1<<ISC2);
	DDRB &=~(1<<2);
	PORTB |= (1<<2);
}

int main(void)
{
	/*initialization code*/
	/*setting PA0 to PA5 & PC0 to PC3 o\p pins*/
	DDRA |= 0x3F;
	DDRC |= 0x1F;
	/*initiate the pins*/
	PORTC &=0xF0;
	PORTC |=(1<<PC4);
	PORTA &=0xC0;
	INT0_init();
	INT1_init();
	INT2_init();
	Timer0_CTC_init();
	Timer1_CTC_init();
	/*global interrupt enable*/
	SREG |= (1<<7);
	/*starting incrementing timer0 for seconds , minutes , hours*/
	TCCR1B |= (1<<CS10) | (1<<CS11);
	/*start with variables ((((((((((((edit here)))))))))))*/
	seconds = 0;
	minutes = 0;
	hours = 0;
	/*------------------------------------------------*/
	/*super loop*/
	while(1)
	{
		/*Application code*/
	}
}

//--------------------------------------------------
/*interrupt service routines*/
//--------------------------------------------------

ISR(INT2_vect)
{
	/*interrupt service routine of external INT0*/
	/* continue the counting */
	TCCR1B |= (1<<CS10) | (1<<CS11);
}
ISR(INT1_vect)
{
	/*interrupt service routine of external INT0*/
	/*stopping the counting */
	TCCR1B &= ~((1<<CS10) | (1<<CS11));
}
ISR(INT0_vect)
{
	/*interrupt service routine of external INT0*/
	/*initiate the clock */
	seconds = 0;
	minutes = 0;
	hours   = 0;
}

ISR(TIMER0_COMP_vect)
{
	/*interrupt service routine for timer0 interrupt*/
	/*each 5 msec Timer0 display current time variables for 3 milliseconds*/
	for(int i =0 ; i<6 ; i++)
	{
		/*enabling the displaying 7-segment*/
		PORTA = (PORTA &0xC0) | (1<<i);
		if(i == 0) PORTC = ((seconds % 10)&0x0F) | (PORTC &0xF0);
		if(i == 1) PORTC = ((seconds / 10)&0x0F) | (PORTC &0xF0);
		if(i == 2) PORTC = ((minutes % 10)&0x0F) | (PORTC &0xF0);
		if(i == 3) PORTC = ((minutes / 10)&0x0F) | (PORTC &0xF0);
		if(i == 4) PORTC = ((hours % 10)&0x0F) | (PORTC &0xF0);
		if(i == 5) PORTC = ((hours / 10)&0x0F) | (PORTC &0xF0);
		/*delay to be able to see the display*/
		_delay_ms(3);
	}
}

ISR(TIMER1_COMPA_vect)
{
	/*interrupt service routine of TIMER1 interrupt*/
	/*each 1 Second TIMER1 update time Variables*/
	if(seconds == 59)
	{
		seconds = 0;
		if(minutes == 59)
		{
			minutes = 0;
			if(hours == 23)
			{
				hours = 0;
			}
			else
			{
				hours++;
			}
		}
		else
		{
				minutes++;
		}
	}
	else
	{
		seconds++;
	}
}
