add_library(
  LUFA
  lufa/LUFA/Drivers/USB/Class/Device/CDCClassDevice.c
  lufa/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c
  lufa/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c
  lufa/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c
  lufa/LUFA/Drivers/USB/Core/ConfigDescriptors.c
  lufa/LUFA/Drivers/USB/Core/Events.c
  )

# target_compile_features(LUFA PUBLIC cxx_std_14)
