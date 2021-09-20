add_library(
  LUFA
  lufa/LUFA/Drivers/USB/Class/Device/CDCClassDevice.c
  lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c
  lufa/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c
  lufa/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c
  lufa/LUFA/Drivers/USB/Core/ConfigDescriptors.c
  lufa/LUFA/Drivers/USB/Core/Events.c
)

target_compile_definitions(LUFA PUBLIC
  -D USB_DEVICE_ONLY
  -D DEVICE_STATE_AS_GPIOR=0
  -D ORDERED_EP_CONFIG
  -D FIXED_CONTROL_ENDPOINT_SIZE=8
  -D FIXED_NUM_CONFIGURATIONS=1
  -D USE_RAM_DESCRIPTORS
#  -D USE_STATIC_OPTIONS= "(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)"
  -D NO_INTERNAL_SERIAL
  -D NO_DEVICE_SELF_POWER
  -D NO_DEVICE_REMOTE_WAKEUP
  -D NO_SOF_EVENTS
  -D F_USB=F_CPU
  -D DEVICE_VID=0x2C99
  -D DEVICE_PID=0x0003 #could also be 0x0004. TBD.
)

target_compile_features(LUFA PUBLIC c_std_99)
