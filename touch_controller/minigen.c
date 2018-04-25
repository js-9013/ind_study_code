/*
 * minigen.c
 *
 *  Created on: Nov 7, 2017
 *      Author: Viet
 */


#include <minigen.h>
//#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
//#include <ti/drivers/GPIO.h>
//#include <ti/drivers/SPI.h>
#include <driverlib.h>
#include <GPIO.h>
#include <SPI.h>

#include <stdint.h>
#include <stdbool.h>

void minigen_reset(){
    uint32_t defaultFreq = minigen_freqCalc(100.0);
    minigen_adjustFreq(FREQ0, FULL, defaultFreq);
    minigen_adjustFreq(FREQ1, FULL, defaultFreq);
    //minigen_adjustPhaseShift(PHASE0, 0x0000);
    SpiWrite(0x0100);
    SpiWrite(0x0000);
}

// Set the mode of the part. The mode (trinagle, sine, or square) is set by
//  three bits in the status register: D5 (OPBITEN), D3 (DIV2), and D1 (MODE).
//  Here's a nice truth table for those settings:
//  D5 D1 D3
//  0  0  x   Sine wave output
//  0  1  x   Triangle wave output
//  1  0  0   Square wave @ 1/2 frequency
//  1  0  1   Square wave @ frequency
//  1  1  x   Not allowed
void minigen_setMode(MODE newMode){
    // We want to adjust the three bits in the config register that we're
    //  interested in without screwing up anything else. Unfortunately, this
    //  part is write-only, so we need to maintain a local shadow, adjust that,
    //  then write it.
    configReg &= ~0x002A; // Clear D5, D3, and D1.
    // This switch statement sets the appropriate bit in the config register.
    switch(newMode)
    {
        case TRIANGLE:
            configReg |= 0x0002;
        break;
        case SQUARE_2:
            configReg |=0x0020;
        break;
        case SQUARE:
            configReg |=0x0028;
        break;
        case SINE:
            configReg |=0x0000;
        break;
    }
    SpiWrite(configReg); // Now write our shadow copy to the part.
}

// The AD9837 has two frequency registers that can be independently adjusted.
//  This allows us to fiddle with the value in one without affecting the output
//  of the device. The register used for calculating the output is selected by
//  toggling bit 11 of the config register.
void minigen_selectFreqReg(FREQREG reg)
{
    // For register FREQ0, we want to clear bit 11.
    if (reg == FREQ0) configReg &= ~0x0800;
    // Otherwise, set bit 11.
    else              configReg |= 0x0800;
    SpiWrite(configReg);
}


// Similarly, there are two phase registers, selected by bit 10 of the config
//  register.
void  minigen_selectPhaseReg(PHASEREG reg)
{
  if (reg == PHASE0) configReg &= ~0x0400;
  else               configReg |= 0x0400;
  SpiWrite(configReg);
}


// The frequency registers are 28 bits in size (combining the lower 14 bits of
//  two 16 bit writes; the upper 2 bits are the register address to write).
//  Bits 13 and 12 of the config register select how these writes are handled:
//  13 12
//  0  0   Any write to a frequency register is treated as a write to the lower
//          14 bits; this allows for fast fine adjustment.
//  0  1   Writes are send to upper 14 bits, allowing for fast coarse adjust.
//  1  x   First write of a pair goes to LSBs, second to MSBs. Note that the
//          user must, in this case, be certain to write in pairs, to avoid
//          unexpected results!
void minigen_setFreqAdjustMode(FREQADJUSTMODE newMode)
{
    // Start by clearing the bits in question.
    configReg &= ~0x3000;
    // Now, adjust the bits to match the truth table above.
    switch(newMode)
    {
        case COARSE:  // D13:12 = 01
            configReg |= 0x1000;
            break;
        case FINE:    // D13:12 = 00
            break;
        case FULL:    // D13:12 = 1x (we use 10)
            configReg |= 0x2000;
            break;
    }
    SpiWrite(configReg);
}

// The phase shift value is 12 bits long; it gets routed to the proper phase
//  register based on the value of the 3 MSBs (4th MSB is ignored).
void minigen_adjustPhaseShift(PHASEREG reg, uint16_t newPhase)
{
    // First, let's blank the top four bits. Just because it's the right thing
    //  to do, you know?
    newPhase &= ~0xF000;
    // Now, we need to set the top three bits to properly route the data.
    //  D15:D13 = 110 for PHASE0...
    if (reg == PHASE0) newPhase |= 0xC000;
    // ... and D15:D13 = 111 for PHASE1.
    else               newPhase |= 0xE000;
    SpiWrite(newPhase);
}

// Okay, now we're going to handle frequency adjustments. This is a little
//  trickier than a phase adjust, because in addition to properly routing the
//  data, we need to know whether we're writing all 32 bits or just 16. I've
//  overloaded this function call for three cases: write with a mode change (if
//  one is needed), and write with the existing mode.

// Adjust the contents of the given register, and, if necessary, switch mode
//  to do so. This is probably the slowest method of updating a register.
void minigen_adjustFreq(FREQREG reg, FREQADJUSTMODE mode, uint32_t newFreq)
{
    minigen_setFreqAdjustMode(mode);

    // We need to split the 32-bit input into two 16-bit values, blank the top
    //  two bits of those values, and set the top two bits according to the
    //  value of reg.
    // Start by acquiring the low 16-bits...
    uint16_t temp = (uint16_t)newFreq;
    // ...and blanking the first two bits.
    temp &= ~0xC000;
    // Now, set the top two bits according to the reg parameter.
    if (reg==FREQ0) temp |= 0x4000;
    else            temp |= 0x8000;
    // Now, we can write temp out to the device.
    SpiWrite(temp);
    // Okay, that's the lower 14 bits. Now let's grab the upper 14.
    temp = (uint16_t)(newFreq>>14);
    // ...and now, we can just repeat the process.
    temp &= ~0xC000;
    // Now, set the top two bits according to the reg parameter.
    if (reg==FREQ0) temp |= 0x4000;
    else            temp |= 0x8000;
    // Now, we can write temp out to the device.
    SpiWrite(temp);
}

// Helper function, used to calculate the integer value to be written to a
//  freq register for a desired output frequency.
// The output frequency is fclk/2^28 * FREQREG. For us, fclk is 16MHz. We can
//  save processor time by specifying a constant for fclk/2^28- .0596. That is,
//  in Hz, the smallest step size for adjusting the output frequency.
uint32_t minigen_freqCalc(float desiredFrequency)
{
    return (uint32_t) (desiredFrequency/.0596);
}
