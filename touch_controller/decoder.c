/*
 * decoder.c
 *
 *  Created on: Dec 21, 2017
 *      Author: Viet
 */

#include <decoder.h>

#define BLOCK_SIZE 1000 //32
#define NUM_TAPS 4


/* -------------------------------------------------------------------
 * Declare State buffer of size (numTaps + blockSize - 1)
 * ------------------------------------------------------------------- */
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
/* ----------------------------------------------------------------------
** FIR Coefficients buffer generated using fir1() MATLAB function.
** fir1(3, 3/10)
** because we need low-order filter, cut off frequency = 3KHz, sampling frequency = 20KHz -> Nyquist frequency = 10KHz
** ------------------------------------------------------------------- */
//const float32_t firCoeffs32[NUM_TAPS] = {
//  -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
//  -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
//  +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
//  +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f
//};
const float32_t firCoeffs32[NUM_TAPS] = {0.0350326977f, 0.4649673022f, 0.4649673022f, 0.0350326977f};

uint32_t blockSize = BLOCK_SIZE;
float32_t result[SAMPLE_LENGTH+8-1] = {0};
float32_t convolveSeq[8] = {0, 100, 0, -100, 0, 100, 0, -100};
uint16_t testArr[SAMPLE_LENGTH+8-1] = {0};
uint16_t resultArr[SAMPLE_LENGTH+8-1] = {0};
int bitLength = 1;
extern uint8_t intermediate_bit_sequence[SAMPLE_LENGTH];
float32_t threshold = 200.0f;

void generate_code_sent(){
    int i = 0;
    uint8_t bit = 0;
    for (i = 0; i < CODE_LENGTH; i++){
        bit = 1 - bit;
        code_sent[i] = bit;
    }
}

uint8_t decode_bits(uint16_t adc_buffer[]){//, uint8_t* intermediate_bit_sequence){
    float32_t f32_adc_buffer[SAMPLE_LENGTH];

    uint8_t might_have_code = 0;
    int i = 0;
    for (i = 0; i < SAMPLE_LENGTH; i++)
        f32_adc_buffer[i] = (float32_t) adc_buffer[i];

    // signal = signal - mean(signal)
    float32_t mean_adc;
    arm_mean_f32(f32_adc_buffer, SAMPLE_LENGTH, &mean_adc);
    float32_t f32_adc_buffer_offset[SAMPLE_LENGTH];
    arm_offset_f32(f32_adc_buffer, -mean_adc, f32_adc_buffer_offset, SAMPLE_LENGTH);

    // sig_sq = 2 * signal .* signal
    float32_t sig_sq[SAMPLE_LENGTH];
    arm_mult_f32(f32_adc_buffer_offset, f32_adc_buffer_offset, sig_sq, SAMPLE_LENGTH);
    arm_scale_f32(sig_sq, 2, sig_sq, SAMPLE_LENGTH);

    // FIR filtering. From this example: http://www.keil.com/pack/doc/CMSIS/DSP/html/arm_fir_example_f32_8c-example.html#_a12
    // env_sq = fir1(sig_sq);
    float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
    float32_t env_sq[SAMPLE_LENGTH];
    arm_fir_instance_f32 S;

    arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], SAMPLE_LENGTH);
    arm_fir_f32(&S, &sig_sq[0], &env_sq[0], SAMPLE_LENGTH);

    // env = sqrt(env_sq);
    float32_t env[SAMPLE_LENGTH];
    for (i = 0; i < SAMPLE_LENGTH; i++){
        arm_sqrt_f32(env_sq[i], &env[i]);
    }

    // now we have `env` as the envelope of the signal.
    // do thresholding

    for (i = 0; i < SAMPLE_LENGTH; i++){
        intermediate_bit_sequence[i] = env[i] > threshold? 1 : 0;
        if (intermediate_bit_sequence[i] == 1){
            might_have_code = 1;
        }
    }
    return might_have_code;
}
