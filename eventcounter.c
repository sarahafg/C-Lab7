/*--------------------------------------------------------
GEORGE MASON UNIVERSITY
ECE 447 - LaunchPad LCD and Port Input Example
  Using port ISR to detect event

  Counting high to low transitions on Port 2.0 and
  displaying them on the LCD.
  Pressing Launchpad switch S1 on Port 1.1 resets
  the counter to 0.

Date:   Fall 2020
Author: Jens-Peter Kaps
--------------------------------------------------------*/

#include <msp430.h>
#include "eventcounter.h"
#include "lcd.h"

float speed = 0;
float mph = 0;
unsigned int count = 0;
unsigned int first_second = 0;
unsigned int third_fourth = 0;
unsigned int ovrflw_cnt = 0;
const int c = 14;
unsigned char total_avgs = 0;
unsigned int kmph = 0;
unsigned char flag = 0;

int main(void) {
    volatile unsigned int i;
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    P1DIR |= REDPIN;           // output: red LED
    P1DIR &= ~BUTTON1;        // input
    P2DIR &= ~SENSOR;         // input
    P1OUT |= BUTTON1;          // give pullup/down resistor a '1' so it pulls up
    P1REN |= BUTTON1;          // enable pullup/down
    P2SEL0 &= ~(BIT0);          // pin 2 connected to TB0CCR6
    P2SEL1 |= (BIT0);
    PM5CTL0 &= ~LOCKLPM5;        // Unlock ports from power manager, causes interrupts!
    P1IES |= BUTTON1;          // Interrupt on falling edge
    P1IFG &= ~BUTTON1;         // clear interrupt flag
    P1IE |= BUTTON1;          // enable interrupt
    TB0CTL = MC_2 | ID_0 | TBSSEL_1 | TBIE | TBCLR;
    TB0CCTL6 = CM_3 | CCIS_1 | SCS | CAP | CCIE;
    lcd_init();                  // initialize the LCD
    __enable_interrupt();
    lcd_clear();
    count = 0;
    displayNum(0,4);
    while(1)                      // continuous loop
    {
        __low_power_mode_3();
        displayNum(speed,4);    // display counter value
    }
    return 0;
}

// Interrupt Service Routine for Port 1
#pragma vector=PORT1_VECTOR       // associate funct. w/ interrupt vector
__interrupt void Port_1(void)     // name of ISR
{
    if(P1IFG & BUTTON1){             // button 1 caused interrupt
        count = 0;
        P1IFG &= ~BUTTON1;           // clear interrupt flag of BUTTON1
        if(flag == 0) {
            flag = 1;
            kmph = speed;
            mph = 1.61 * speed;
            speed = mph;
            __low_power_mode_off_on_exit();
        } else {
            flag = 0;
            speed = kmph;
            __low_power_mode_off_on_exit();
        }
    }
}

#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void)
{
  switch(__even_in_range(TB0IV,TB0IV_TBIFG))
  {
    case TB0IV_NONE:   break;               // No interrupt
    case TB0IV_TBCCR1: break;               // CCR1 not used
    case TB0IV_TBCCR2: break;               // CCR2 not used
    case TB0IV_TBCCR3: break;               // CCR3 not used
    case TB0IV_TBCCR4: break;               // CCR4 not used
    case TB0IV_TBCCR5: break;               // CCR5 not used
    case TB0IV_TBCCR6:

        if (TB0CCTL6 & CCI) {
            if (total_avgs >= 4) {
                speed = 0;
            }
            if (TB0CCR6 > first_second) {
                if (ovrflw_cnt <= 1) {  // one or no overflow
                    third_fourth = TB0CCR6 - first_second;
                }
            }
            if (TB0CCR6 < first_second) {
                if (ovrflw_cnt > 1) { // more than one overflow
                    third_fourth = (65536)*(ovrflw_cnt-1) + (TB0CCR6-first_second);
                }
            }
            if (TB0CCR6 > first_second) {
                if (ovrflw_cnt > 1) { // more than one overflow
                    third_fourth = ovrflw_cnt*(65536) + (TB0CCR6-first_second);
                }
            }
            total_avgs++;
            speed = third_fourth/32768.0;
            speed = c/speed;
            speed = speed/18;
            speed = speed*100;
            speed = speed/4;
            __low_power_mode_off_on_exit();
        } else {
            ovrflw_cnt=0;
            first_second = TB0CCR6;
            __low_power_mode_off_on_exit();
        }
        break;
    case TB0IV_TBIFG:                       // overflow
        P1OUT ^= REDPIN;
      break;
    default: break;
  }
}
