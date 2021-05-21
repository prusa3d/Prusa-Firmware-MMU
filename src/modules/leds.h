#pragma once

#include <stdint.h>

/// We have 5 pairs of LEDs
/// In each pair there is a green and a red LED
///
/// A typical scenario in the past was visualization of error states.
/// The combination of colors with blinking frequency had a specific meaning.
///
/// We'd like to drive each pair? separately, which includes:
/// - blinking (none/slow/fast)
/// - what shall blink (red/green/both-at-once/both-interlaced)
///
/// The physical connection is not important on this level (i.e. how and what shall be sent into the shift registers)

namespace modules {
namespace leds {

    /// Mode of LED
    /// blink0 and blink1 allow for interlaced blinking of LEDs (one is on and the other off)
    enum Mode {
        off,
        on,
        blink0, ///< start blinking at even periods
        blink1 ///< start blinking at odd periods
    };

    /// a single LED
    class LED {
    public:
        constexpr inline LED() = default;
        void SetMode(Mode mode);

        /// @returns true if the LED shines
        bool Step(bool oddPeriod);
        inline bool On() const { return state.on; }

    private:
        struct State {
            uint8_t on : 1;
            uint8_t mode : 2;
            constexpr inline State()
                : on(0)
                , mode(Mode::off) {}
        };

        State state;
    };

    /// main LED API
    class LEDs {
    public:
        constexpr inline LEDs()
            : ms(0) {};

        /// step LED automaton
        /// @returns statuses of LEDs - one bit per LED and 1 = on, 0 = off
        uint16_t Step(uint8_t delta_ms);

        inline constexpr uint8_t LedPairsCount() const { return ledPairs; }

        inline void SetMode(uint8_t slot, bool red, Mode mode) {
            SetMode(slot * 2 + red, mode);
        }
        inline void SetMode(uint8_t index, Mode mode) {
            leds[index].SetMode(mode);
        }

        inline bool LedOn(uint8_t index) const {
            return leds[index].On();
        }
        inline bool LedOn(uint8_t slot, bool red) const {
            return leds[slot * 2 + red].On();
        }

    private:
        constexpr static const uint8_t ledPairs = 5;
        LED leds[ledPairs * 2];
        uint16_t ms;
    };

    //// asi nechame moznost jednu blikat rychle a druhou pomalu

    //// tohle ale nevyresi, ze jedna LED ma blikat a druha svitit

    //// problem je, ze zapnuti/vypnuti led je blink none ... mozna by to chtelo blink none-on a none-off
    //// jaka bude mnozina rezimu?
    ////                green-off    green-on   green-blink-slow green-blink-fast
    //// red-off        +            +          +                +
    //// red-on         +            +          +                +
    //// red-blink-slow +            +          2                x
    //// red-blink-fast +            +          x                2
    ////
    //// rezim 2 muze mit jeste varianty - blink-sync, blink-interlaced, coz v podstate znamena rezimy:
    //// blink-slow1, blink-slow2, blink-fast1, blink-fast2, pricemz nemaji smysl kombinace:
    //// blink-slow + blink-fast

    //// revize
    //// v aktualnim FW led jen blikaji jednou rychlosti
    //// takze to spis udelam jako pole LED, pricemz cervene budou na sudych a zelene na lichych indexech
    //// stav ledky bude: off, on, blink0 a blink1
    //// blink0 znamena zacni pocitat blikaci interval od sude periody
    //// blink1 od liche
    //// tim pujde zmodelovat jak sync tak async blikani
    //// Dale je otazka, jestli chceme rychle a pomale blikani... asi ne, kdyz bude report na LCD tiskarny

    //// Dale castecne souvisejici
    //// Start MMU by mel reportovat progress, pricemz aktualni stav je takovy, ze nabihaji LED a kazda neco znamena
    //// Tiskarna by klidne mohla posilat Q a dostavat odpoved ve stylu "starting" a progress/state code
    ////
    //// Mozna taky budeme potrebovat nastavovat ruzne vnitrni promenne (treba use slots 1-3 namisto 0-4), takze operace SetVar a k tomu obracena GetVar
    //// Dava mi smysl modifikovat protokol V jako getvar (zamerne ne G, aby to nekolidovalo s gcodes) a S jako setvar
    //// cislo je index variable a odpovi se A_hodnota, cili accepted a hodnota odpovedi

    //void SetLEDs(uint8_t slot0, uint8_t slot1, uint8_t slot2, uint8_t slot3, uint8_t slot4);

} // namespace LEDs
} // namespace modules
