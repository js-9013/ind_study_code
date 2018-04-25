#include "msp.h"
#include <driverlib.h>
#include <grlib.h>
#include <stdio.h>
#include <arm_math.h>
#include "arm_const_structs.h"
#include "decoder.h"
#include "minigen_controller.h"

uint32_t doBitReverse = 1;
volatile arm_status status;

#define SMCLK_FREQUENCY     4800000
#define SAMPLE_FREQUENCY    20000
#define NUMBER_DMA_BUFFERS 6
#define PREAMBLE_LENGTH 16

/* DMA Control Table */
#ifdef ewarm
#pragma data_alignment=256
#else
#pragma DATA_ALIGN(controlTable, 256)
#endif
uint8_t controlTable[256];

/* FFT data/processing buffers*/
uint16_t data_array1[SAMPLE_LENGTH];
uint16_t data_array2[SAMPLE_LENGTH];
uint8_t intermediate_bit_sequence[SAMPLE_LENGTH];
uint8_t merged_bit_sequence[NUMBER_DMA_BUFFERS * SAMPLE_LENGTH];
uint8_t code_sent_2[CODE_LENGTH] = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0};
uint8_t preamble[PREAMBLE_LENGTH] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
uint8_t received_code[CODE_LENGTH];

volatile int switch_data = 0;
extern void UART_transmitData(uint32_t, uint_fast8_t);

/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_1,
        (SMCLK_FREQUENCY/SAMPLE_FREQUENCY),
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_SET_RESET,
        (SMCLK_FREQUENCY/SAMPLE_FREQUENCY)/2
};

const eUSCI_UART_Config uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_ACLK,          // ACLK Clock Source
    3,                                     // BRDIV = 3
    0,                                       // UCxBRF = 2
    146,                                       // UCxBRS = 146
    EUSCI_A_UART_NO_PARITY,                  // No Parity
    EUSCI_A_UART_LSB_FIRST,                  // MSB First
    EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
    EUSCI_A_UART_MODE,                       // UART mode
    EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION  // Low frequency setting
};

volatile uint8_t code_found = 0;

// temporary variables
uint32_t idx; // index of the merged_bit_sequence
uint32_t symbol_sequence_idx;
uint32_t last_idx; // index of the first sample of current same-value sequence
uint32_t i;
int idx2;
uint32_t nbits;
unsigned int timerCount = 0;
volatile uint32_t curValue;
volatile uint32_t startValue;

void transmitCode(char code, uint32_t time) {
    UART_transmitData(EUSCI_A2_BASE, 0xFF);
    UART_transmitData(EUSCI_A2_BASE, time & 0xFF);
    UART_transmitData(EUSCI_A2_BASE, time >> 8 & 0xFF);
    UART_transmitData(EUSCI_A2_BASE, time >> 16 & 0xFF);
    UART_transmitData(EUSCI_A2_BASE, time >> 24 & 0xFF);
    UART_transmitData(EUSCI_A2_BASE, code);
}

void main(void)
{
    /* Halting WDT and disabling master interrupts */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

    /* Set to Vcore1 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set to use DCDC */
    MAP_PCM_setPowerState(PCM_AM_DCDC_VCORE1);

    /* Set wait state */
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
//    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
//    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
//    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
//    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
//    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_setExternalClockSourceFrequency(32768, 4800000);
    MAP_CS_startHFXT(false);
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1); //use DCO CLK for SMCLK - UART, no divider
    MAP_CS_initClockSignal(CS_ACLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12); //set DCO clock to 12 MHz

    /* Terminating all remaining pins to minimize power consumption. This is
        done by register accesses for simplicity and to minimize branching API
        calls */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PB, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PC, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PD, PIN_ALL16);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_PE, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PB, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PC, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PD, PIN_ALL16);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_PE, PIN_ALL16);

    /* Configure pins P3.2 and P3.3 in UART mode.
     * Doesn't matter if they are set to inputs or outputs
     */

    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
               GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT, TIMER32_FREE_RUN_MODE);

    /* Configuring UART Module */
    UART_initModule(EUSCI_A2_BASE, &uartConfig);

    /* Enable UART module */
    UART_enableModule(EUSCI_A2_BASE);

    /* Configuring Timer_A to generate PWM  */
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    /* Initializing ADC (MCLK/1/1) */
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);

    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    /* Configuring GPIOs (4.3 A10) */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN3,
    GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory */
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A10, false);

    /* Configuring DMA module */
    DMA_enableModule();
    DMA_setControlBase(controlTable);


    DMA_disableChannelAttribute(DMA_CH7_ADC14,
                                 UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                 UDMA_ATTR_HIGH_PRIORITY |
                                 UDMA_ATTR_REQMASK);


    /* Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 0
     *  and the destination to the
     * destination data array. */
    MAP_DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH7_ADC14,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
        UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
		(void*)data_array1, SAMPLE_LENGTH);

    MAP_DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH7_ADC14,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
        UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
		(void*)data_array2, SAMPLE_LENGTH);

    /* Assigning/Enabling Interrupts */
    MAP_DMA_assignInterrupt(DMA_INT1, 7);
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_DMA_assignChannel(DMA_CH7_ADC14);
    MAP_DMA_clearInterruptFlag(7);
    MAP_Interrupt_enableMaster();

    /* Now that the DMA is primed and setup, enabling the channels. The ADC14
     * hardware should take over and transfer/receive all bytes */
    MAP_DMA_enableChannel(7);
    MAP_ADC14_enableConversion();

    uint8_t might_have_code;
    uint8_t current_number_of_dma_buffers = 0;

    generate_code_sent();

    /* Set up the MiniGen as local oscillator */
    setup_local_oscillator();

    MAP_Timer32_startTimer(TIMER32_BASE, true);
    while (1) {
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN3);
        if ((MAP_Timer32_getValue(TIMER32_BASE) % 2) == 0)
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN3);
    }
    startValue = MAP_Timer32_getValue(TIMER32_BASE);
    transmitCode('a', curValue); //assume Android app starts before uC

    /*while (1) { //used for debugging UART
        UART_transmitData(EUSCI_A2_BASE, 'a');
        UART_transmitData(EUSCI_A2_BASE, 'b');
        UART_transmitData(EUSCI_A2_BASE, 'c');
        UART_transmitData(EUSCI_A2_BASE, 'd');};*/

    while(1)
    {
        MAP_PCM_gotoLPM0();

        if (switch_data & 1)
        {
            might_have_code = decode_bits(data_array1);
        }
        else
        {
            might_have_code = decode_bits(data_array2);
        }

        if (might_have_code){
           if (current_number_of_dma_buffers < NUMBER_DMA_BUFFERS){
                arm_copy_q15(&intermediate_bit_sequence[0], &merged_bit_sequence[current_number_of_dma_buffers * SAMPLE_LENGTH], SAMPLE_LENGTH );
                current_number_of_dma_buffers ++;
            }
            else{ // when there are enough DMA buffers
                code_found = 0;

                // find symbol sequence
                uint8_t symbol_sequence[300];
                symbol_sequence_idx = 0;
                last_idx = 0;
                for (idx = 1; idx < NUMBER_DMA_BUFFERS * SAMPLE_LENGTH; idx++){
                    if (merged_bit_sequence[idx] != merged_bit_sequence[idx-1]){
                        nbits = (uint32_t) round( (float)(idx - last_idx) / 20 );
                        for (i = 0; i < nbits; i++){
                            symbol_sequence[symbol_sequence_idx++] = merged_bit_sequence[idx-1];
                        }
                        last_idx = idx;
                    }
                }

                // Find code in the symbol sequence
                uint32_t symbol_sequence_length = symbol_sequence_idx;
                // find preamble
                for (idx = 0; idx < symbol_sequence_length - CODE_LENGTH - PREAMBLE_LENGTH; idx++){
                    idx2 = 0;
                    while (idx2 < PREAMBLE_LENGTH){
                        if (symbol_sequence[idx+idx2] == preamble[idx2]){
                            idx2++;
                        } else {
                            break;
                        }
                    }
                    if (idx2 == PREAMBLE_LENGTH){
                        code_found = 1;
                        // get code after the preamble
                        arm_copy_q7(&symbol_sequence[idx+PREAMBLE_LENGTH], &received_code[0], CODE_LENGTH );
                        break;
                    }
                }

                //code_found = 1; //debugging UART
                // notification
                if(code_found){
                    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                    curValue = MAP_Timer32_getValue(TIMER32_BASE);
                    transmitCode('g', curValue);
                }
                else{
                    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                    //UART_transmitData(EUSCI_A2_BASE, 0x67);
                }

                // reset count of dma buffers
                current_number_of_dma_buffers = 0;
            }
        }

        else {
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
            //UART_transmitData(EUSCI_A2_BASE, 0x67);
        }
    }
}


/* Completion interrupt for ADC14 MEM0 */
void DMA_INT1_IRQHandler(void)
{
	MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN4);
    /* Switch between primary and alternate buffers with DMA's PingPong mode */
    if (DMA_getChannelAttribute(7) & UDMA_ATTR_ALTSELECT)
    {
        DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH7_ADC14,
            UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
            UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
            (void*)data_array1, SAMPLE_LENGTH);
        switch_data = 1;
    }
    else
    {
        DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH7_ADC14,
            UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
            UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
			(void*)data_array2, SAMPLE_LENGTH);
        switch_data = 0;
    }
}

void systick_isr(void)
{
    curValue = MAP_Timer32_getValue(TIMER32_BASE);
}

