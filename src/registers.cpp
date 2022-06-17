#include <avr/pgmspace.h>

#include "registers.h"
#include "version.h"
#include "application.h"

#include "modules/globals.h"
#include "modules/finda.h"
#include "modules/fsensor.h"

struct RegisterFlags {
    uint8_t writable : 1;
    uint8_t rwfuncs : 1; // 1: register needs special read and write functions
    uint8_t size : 2; // 0: 1 bit, 1: 1 byte, 2: 2 bytes
    constexpr RegisterFlags(bool writable, uint8_t size)
        : writable(writable)
        , rwfuncs(0)
        , size(size) {}
    constexpr RegisterFlags(bool writable, bool rwfuncs, uint8_t size)
        : writable(writable)
        , rwfuncs(rwfuncs)
        , size(size) {}
};

using TReadFunc = uint16_t (*)();
using TWriteFunc = void (*)(uint16_t);

struct RegisterRec {
    RegisterFlags flags;
    union U1 {
        void *addr;
        TReadFunc readFunc;
        constexpr explicit U1(TReadFunc r)
            : readFunc(r) {}
        constexpr explicit U1(void *a)
            : addr(a) {}
    } A1;

    union U2 {
        void *addr;
        TWriteFunc writeFunc;
        constexpr explicit U2(TWriteFunc w)
            : writeFunc(w) {}
        constexpr explicit U2(void *a)
            : addr(a) {}
    } A2;

    template <typename T>
    constexpr RegisterRec(bool writable, T *address)
        : flags(RegisterFlags(writable, sizeof(T)))
        , A1((void *)address)
        , A2((void *)nullptr) {}
    constexpr RegisterRec(TReadFunc readFunc, uint8_t bytes)
        : flags(RegisterFlags(false, true, bytes))
        , A1(readFunc)
        , A2((void *)nullptr) {}

    constexpr RegisterRec(TReadFunc readFunc, TWriteFunc writeFunc, uint8_t bytes)
        : flags(RegisterFlags(true, true, bytes))
        , A1(readFunc)
        , A2(writeFunc) {}
};

// @@TODO it is nice to see all the supported registers at one spot,
// however it requires including all bunch of dependencies
// which makes unit testing and separation of modules much harder.
static const RegisterRec registers[] PROGMEM = {
    RegisterRec(false, &project_major),

    RegisterRec(false, &project_minor),

    RegisterRec(false, &project_build_number),

    RegisterRec( // MMU errors
        []() -> uint16_t { return mg::globals.DriveErrors(); },
        [](uint16_t) {}, // @@TODO think about setting/clearing the error counter from the outside
        2),

    RegisterRec([]() -> uint16_t { return application.CurrentProgressCode(); }, 1),

    RegisterRec([]() -> uint16_t { return application.CurrentErrorCode(); }, 2),

    RegisterRec( // filamentState
        []() -> uint16_t { return mg::globals.FilamentLoaded(); },
        [](uint16_t v) { return mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), (mg::FilamentLoadState)v); },
        1),

    RegisterRec( // FINDA
        []() -> uint16_t { return mf::finda.Pressed(); },
        1),

    RegisterRec( // fsensor
        []() -> uint16_t { return mfs::fsensor.Pressed(); },
        [](uint16_t v) { return mfs::fsensor.ProcessMessage(v); },
        1),

    RegisterRec([]() -> uint16_t { return mg::globals.MotorsStealth(); }, 1), // mode (stealth = 1/normal = 0)
};

static constexpr uint8_t registersSize = sizeof(registers) / sizeof(RegisterRec);

bool ReadRegister(uint8_t address, uint16_t &value) {
    if (address >= registersSize) {
        return false;
    }
    value = 0;
    if (!registers[address].flags.rwfuncs) {
        switch (registers[address].flags.size) {
        case 0:
        case 1:
            value = *(uint8_t *)registers[address].A1.addr;
            break;
        case 2:
            value = *(uint16_t *)registers[address].A1.addr;
            break;
        default:
            return false;
        }
        return true;
    } else {
        switch (registers[address].flags.size) {
        case 0:
        case 1:
        case 2:
            value = registers[address].A1.readFunc();
            break;
        default:
            return false;
        }
        return true;
    }
}

bool WriteRegister(uint8_t address, uint16_t value) {
    if (address >= registersSize) {
        return false;
    }
    if (!registers[address].flags.writable) {
        return false;
    }
    if (!registers[address].flags.rwfuncs) {
        switch (registers[address].flags.size) {
        case 0:
        case 1:
            *(uint8_t *)registers[address].A1.addr = value;
            break;
        case 2:
            *(uint16_t *)registers[address].A1.addr = value;
            break;
        default:
            return false;
        }
        return true;
    } else {
        switch (registers[address].flags.size) {
        case 0:
        case 1:
        case 2:
            registers[address].A2.writeFunc(value);
            break;
        default:
            return false;
        }
        return true;
    }
}
