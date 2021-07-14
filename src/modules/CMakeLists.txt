target_sources(
  firmware
  PRIVATE protocol.cpp
          buttons.cpp
          debouncer.cpp
          finda.cpp
          fsensor.cpp
          globals.cpp
          idler.cpp
          leds.cpp
          motion.cpp
          permanent_storage.cpp
          selector.cpp
          timebase.cpp
          user_input.cpp
          pulse_gen.cpp
  )
