#pragma once
#include <msp430.h>
#include <stdint.h>

//-----------------------------------------
//  Macros
//-----------------------------------------

#define SET_BIT(PORT, BIT) (PORT) |= (BIT)
#define CLEAR_BIT(PORT, BIT) (PORT) &= ~(BIT)
#define INVERT_BIT(PORT, BIT) (PORT) ^= (BIT)
#define IS_BIT_SET(PORT, BIT) (((PORT) & (BIT)) == 1)

//-----------------------------------------
//  Type definitions
//-----------------------------------------

typedef uint8_t bool;

//-----------------------------------------
//  Constants
//-----------------------------------------

#define true 1
#define false 0
