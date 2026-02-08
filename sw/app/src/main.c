#include "xgpiops.h"
#include "sleep.h"
#include "xstatus.h"

#define GPIOPS_DEVICE_ID 0U
#define LED_MIO          7U   // Zybo Z7: LED4 is on MIO[7]

int main(void)
{
    XGpioPs gpio;
    XGpioPs_Config *cfg = XGpioPs_LookupConfig(GPIOPS_DEVICE_ID);
    if (!cfg) return -1;

    if (XGpioPs_CfgInitialize(&gpio, cfg, cfg->BaseAddr) != XST_SUCCESS)
        return -1;

    XGpioPs_SetDirectionPin(&gpio, LED_MIO, 1);
    XGpioPs_SetOutputEnablePin(&gpio, LED_MIO, 1);

    while (1) {
        XGpioPs_WritePin(&gpio, LED_MIO, 1);
        usleep(500000);
        XGpioPs_WritePin(&gpio, LED_MIO, 0);
        usleep(50000);
    }
}
