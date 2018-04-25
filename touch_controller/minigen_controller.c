/* DriverLib Includes */
#include <minigen.h>
//#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
//#include <ti/drivers/GPIO.h>
//#include <ti/drivers/SPI.h>
#include <driverlib.h>
#include <GPIO.h>
#include <SPI.h>
#include <stdint.h>
#include <stdbool.h>
#include "minigen_controller.h"

/* SPI Master Configuration Parameter */
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,             // SMCLK Clock Source
        3000000,                                   // SMCLK = DCO = 3MHZ
        500000,                                    // SPICLK = 500khz
        EUSCI_B_SPI_MSB_FIRST,                     // MSB First
        EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,    // Phase
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH, // High polarity
        EUSCI_B_SPI_3PIN                           // 3Wire SPI Mode
};

void configSpi(){
    /* Selecting P1.5 P1.6 and P1.7 in SPI mode */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
        GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);
    /*  P1.5 SCLK, P1.6 SIMO */

    /* Configuring SPI in 3wire master mode */
    SPI_initMaster(EUSCI_B0_BASE, &spiMasterConfig);

    /* Enable SPI module */
    SPI_enableModule(EUSCI_B0_BASE);

    //Pin 3.0 FSYNC
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);
}

void SpiWrite(uint16_t data) {
    uint16_t byte2 = data & 0xff00;
    byte2 >>= 8;
    uint8_t transByte2 = (uint8_t)byte2;

    uint16_t byte1 = data & 0x00ff;
    uint8_t transByte1 = (uint8_t)byte1;

    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
    SPI_transmitData(EUSCI_B0_BASE, transByte2);
    SPI_transmitData(EUSCI_B0_BASE, transByte1);
    __delay_cycles(100); //FSYNC now going high only after transmit
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);
}

void setup_local_oscillator(void)
{
    /* Halting WDT  */
    //WDT_A_holdTimer();

    configSpi();

    minigen_reset();
    minigen_setMode(SINE);
    minigen_selectFreqReg(FREQ0);
    uint32_t newFreq = minigen_freqCalc(140000.0);
    minigen_adjustFreq(FREQ0, FULL, newFreq);

}
