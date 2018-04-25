/*
 * minigen.h
 *
 *  Created on: Nov 7, 2017
 *      Author: Viet
 */

#ifndef MINIGEN_H_
#define MINIGEN_H_

#include <stdint.h>
#include <stdio.h>

uint16_t configReg;
uint16_t _FSYNCPin;

typedef enum  {TRIANGLE, SINE, SQUARE, SQUARE_2} MODE;
typedef enum  {FREQ0, FREQ1} FREQREG;
typedef enum  {PHASE0, PHASE1} PHASEREG;
typedef enum  {FULL, COARSE, FINE} FREQADJUSTMODE ;

void configSpi();
void SpiWrite(uint16_t data);

void minigen_reset();
void minigen_setMode(MODE newMode);
void minigen_selectFreqReg (FREQREG reg);
void minigen_setFreqAdjustMode (FREQADJUSTMODE newMode);
void minigen_adjustFreq(FREQREG reg, FREQADJUSTMODE mode, uint32_t newFreq);
// void minigen_adjustFreq(FREQREG reg, FREQADJUSTMODE mode, uint16_t newFreq);
// void minigen_adjustFreq(FREQREG reg, uint32_t newFreq);
uint32_t minigen_freqCalc (float desiredFrequency);

#endif /* MINIGEN_H_ */
