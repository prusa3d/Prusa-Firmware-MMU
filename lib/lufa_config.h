#pragma once

#define USB_DEVICE_ONLY
#define DEVICE_STATE_AS_GPIOR 0
#define ORDERED_EP_CONFIG
#define FIXED_CONTROL_ENDPOINT_SIZE 8
#define FIXED_NUM_CONFIGURATIONS 1
#define USE_FLASH_DESCRIPTORS
#define USE_STATIC_OPTIONS (USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)
#define NO_INTERNAL_SERIAL
#define NO_DEVICE_SELF_POWER
#define NO_DEVICE_REMOTE_WAKEUP
#define NO_SOF_EVENTS
#define F_USB F_CPU
#define DEVICE_VID 0x2C99
#define DEVICE_PID 0x0004