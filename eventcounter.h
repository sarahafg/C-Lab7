/*--------------------------------------------------------
GEORGE MASON UNIVERSITY
  Header file definitions for eventcounter

Date:   Fall 2020
Author: Jens-Peter Kaps
--------------------------------------------------------*/
#ifndef eventcounter
#define eventcounter

#include "msp430.h"

#define BUTTON1 BIT1
#define SENSOR BIT0
#define REDPIN BIT0
#define REDON() P1OUT |= BIT0
#define REDOFF() P1OUT &= ~BIT0

#endif /* eventcounter */
