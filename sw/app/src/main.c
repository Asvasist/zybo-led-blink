/******************************************************************************
 * @file    main.c
 * @brief   Bare-metal LED blink for the Digilent Zybo Z7-20 (Zynq-7000 SoC).
 *
 * Toggles the board's PS-connected user LED (LD4) at roughly 1 Hz using the
 * Zynq PS GPIO controller. This is the "hello world" of the Zynq boot flow:
 * it proves that the FSBL ran, the PS was configured from the exported
 * hardware (XSA), and bare-metal C is executing on Cortex-A9 core 0.
 *
 * Target hardware : Digilent Zybo Z7-20 (xc7z020clg400-1)
 * Processor       : ps7_cortexa9_0 (ARM Cortex-A9, no OS / standalone BSP)
 * Toolchain       : AMD Vivado + Vitis 2025.2
 * Hardware handoff: hw/vivado/design_LED_wrapper.xsa
 *
 * LED wiring (Zybo Z7 Reference Manual, sections 4 and 13):
 *   LD4  -> MIO[7], driven by the PS GPIO controller  <-- used by this program
 *   LD0..LD3 -> PL pins M14/M15/G14/D18, reachable over EMIO GPIO[3:0]
 *
 * The user LEDs are anode-connected to the Zynq through 330 ohm resistors,
 * so a logic HIGH turns an LED on (active high, no inversion needed).
 *
 * Note: the block design also routes EMIO GPIO[3:0] out to LD0..LD3, but this
 * program does not drive them, so those four LEDs stay off. See the README
 * section "Known limitations / next steps".
 ******************************************************************************/

#include "xgpiops.h"   /* PS GPIO (MIO/EMIO) driver from the standalone BSP */
#include "sleep.h"     /* usleep() - busy-wait delay backed by the PS timer */
#include "xstatus.h"   /* XST_SUCCESS and friends                           */

/*
 * Device ID of the PS GPIO controller instance (ps7_gpio_0). The Zynq-7000
 * has exactly one, so this is always 0.
 */
#define GPIOPS_DEVICE_ID 0U

/*
 * MIO pin that LD4 hangs off. MIO pins are numbered 0..53; anything from 54
 * upwards in this driver's numbering refers to EMIO, which reaches the PL.
 */
#define LED_MIO          7U   // Zybo Z7: LED4 is on MIO[7]

/* Blink timing, in microseconds. */
#define LED_ON_US        500000U   /* LED lit  for ~500 ms */
#define LED_OFF_US       50000U    /* LED dark for  ~50 ms */

int main(void)
{
    XGpioPs gpio;                  /* Driver instance: holds the runtime state */
    XGpioPs_Config *cfg;           /* Base address + config from the BSP       */

    /*
     * Fetch the hardware configuration that Vitis generated from the XSA.
     * Returns NULL if the requested GPIO instance is not present in this
     * platform, which means the app was built against the wrong hardware.
     */
    cfg = XGpioPs_LookupConfig(GPIOPS_DEVICE_ID);
    if (!cfg) return -1;

    /* Bind the driver instance to that hardware and reset it to a known state. */
    if (XGpioPs_CfgInitialize(&gpio, cfg, cfg->BaseAddr) != XST_SUCCESS)
        return -1;

    /*
     * A Zynq GPIO pin needs two separate registers set before it will drive
     * anything, and forgetting the second one is the classic silent failure:
     *   1. DIRM  - direction: 1 = output
     *   2. OEN   - output enable: 1 = connect the output driver to the pad
     * With DIRM set but OEN clear the pin stays high-impedance and the LED
     * never lights, with no error reported anywhere.
     */
    XGpioPs_SetDirectionPin(&gpio, LED_MIO, 1);
    XGpioPs_SetOutputEnablePin(&gpio, LED_MIO, 1);

    /*
     * Blink forever. main() intentionally never returns: on bare metal there
     * is no OS to return to, and falling out of main() would land in the
     * BSP's exit stub and simply hang.
     */
    while (1) {
        XGpioPs_WritePin(&gpio, LED_MIO, 1);   /* LD4 on  */
        usleep(LED_ON_US);
        XGpioPs_WritePin(&gpio, LED_MIO, 0);   /* LD4 off */
        usleep(LED_OFF_US);
    }
}
