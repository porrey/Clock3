#ifndef BITWISE_H
#define BITWISE_H

#define BV(bit) (1 << (bit))
#define setBit(byte, bit) (byte |= BV(bit))
#define clearBit(byte, bit) (byte &= ~BV(bit))
#define toggleBit(byte, bit) (byte ^= BV(bit))

#endif
