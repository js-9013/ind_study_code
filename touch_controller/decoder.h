/*
 * decoder.h
 *
 *  Created on: Dec 21, 2017
 *      Author: Viet
 */

#ifndef DECODER_H_
#define DECODER_H_

#include "arm_math.h"
//#include "math_helper.h"


#define SAMPLE_LENGTH 1000//1024

#define CODE_LENGTH 128

uint8_t code_sent[CODE_LENGTH];


void generate_code_sent();

uint8_t decode_bits(uint16_t adc_buffer[]);//, uint8_t bit_sequence[]);

#endif /* DECODER_H_ */
