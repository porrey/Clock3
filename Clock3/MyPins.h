#ifndef MY_PINS_H
#define MY_PINS_H

// ***
// *** Buttons
// ***
#define MODE_BUTTON 2
#define SETUP_BUTTON 3

// ***
// *** Defines the register and port for the 74LS138N 3 to 8 decoders. The three
// *** input pins are on PD4 (A), PD5 (B) and PD6 (C). A, B and C are all tied
// *** together on on three decoder chips.
// ***
#define DECODER_REGISTER DDRD
#define DECODER_PORT PORTD
#define DECODER_A PORTD4
#define DECODER_B PORTD5
#define DECODER_C PORTD6

// ***
// *** The rows are connected to PORT B in order (row 1 on PB0, row 2
// *** PB1 and so on).
// ***
#define ROW_REGISTER DDRB
#define ROW_PORT PORTB
#define ROW_1 PORTB0
#define ROW_2 PORTB1
#define ROW_3 PORTB2
#define ROW_4 PORTB3
#define ROW_5 PORTB4
#define ROW_6 PORTB5
#define ROW_7 PORTB6

// ***
// *** The enable pins of the 74LS138N decoder chips are tied to
// *** pins 2 and 3 of Port C.
// *** CS_S1 = 1, CS_S2 = 0 (enables columns 0 to 7)
// *** CS_S1 = 0, CS_S2 = 1 (enables columns 8 to 15)
// *** CS_S1 = 0, CS_S2 = 0 (enables columns 16 to 19)
// ***
#define CHIP_SELECT_REGISTER DDRC
#define CHIP_SELECT_PORT PORTC
#define CS_S1 PORTC2
#define CS_S2 PORTC3
#define DECODERS_OFF (CHIP_SELECT_PORT & B11110011) | B00001100;
#define DECODER_1 (CHIP_SELECT_PORT & B11110011) | B00000100;
#define DECODER_2 (CHIP_SELECT_PORT & B11110011) | B00001000;
#define DECODER_3 (CHIP_SELECT_PORT & B11110011) | B00000000; 

#endif
