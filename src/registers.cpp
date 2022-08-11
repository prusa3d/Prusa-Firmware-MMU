#ifndef UNITTEST
#include <avr/pgmspace.h>
#else
#define PROGMEM /* */
#endif

#include "registers.h"
#include "application.h"
#include "version.hpp"

#include "modules/finda.h"
#include "modules/fsensor.h"
#include "modules/globals.h"
#include "modules/idler.h"
#include "modules/selector.h"

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

// dummy zero register common to all empty registers
static constexpr uint16_t dummyZero = 0;

struct RegisterRec {
    RegisterFlags flags;
    union U1 {
        void *addr;
        TReadFunc readFunc;
        constexpr explicit U1(const TReadFunc &r)
            : readFunc(r) {}
        constexpr explicit U1(void *a)
            : addr(a) {}
    } A1;

    union U2 {
        void *addr;
        TWriteFunc writeFunc;
        constexpr explicit U2(const TWriteFunc &w)
            : writeFunc(w) {}
        constexpr explicit U2(void *a)
            : addr(a) {}
    } A2;

    template <typename T>
    constexpr RegisterRec(bool writable, T *address)
        : flags(RegisterFlags(writable, sizeof(T)))
        , A1((void *)address)
        , A2((void *)nullptr) {}
    constexpr RegisterRec(const TReadFunc &readFunc, uint8_t bytes)
        : flags(RegisterFlags(false, true, bytes))
        , A1(readFunc)
        , A2((void *)nullptr) {}

    constexpr RegisterRec(const TReadFunc &readFunc, const TWriteFunc &writeFunc, uint8_t bytes)
        : flags(RegisterFlags(true, true, bytes))
        , A1(readFunc)
        , A2(writeFunc) {}

    constexpr RegisterRec()
        : flags(RegisterFlags(false, false, 1))
        , A1((void *)&dummyZero)
        , A2((void *)nullptr) {}
};

// @@TODO it is nice to see all the supported registers at one spot,
// however it requires including all bunch of dependencies
// which makes unit testing and separation of modules much harder.
// @@TODO clang complains that we are initializing this array with an uninitialized referenced variables (e.g. mg::globals)
// Imo that should be safe as long as we don't call anything from this array before the FW init is completed (which we don't).
// Otherwise all the addresses of global variables should be known at compile time and the registers array should be consistent.
//
// Note:
// The lambas seem to be pretty cheap:
//    void SetFSensorToNozzleFeedrate(uint8_t fs2NozzleFeedrate) { fsensorToNozzleFeedrate = fs2NozzleFeedrate; }
// compiles to:
// sts <modules::globals::globals+0x4>, r24
// ret
//
// @@TODO at the moment we are having problems compiling this array statically into PROGMEM.
// In this project that's really not an issue since we have half of the RAM empty:
// Data: 1531 bytes (59.8% Full)
// But it would be nice to fix that in the future - might be hard to push the compiler to such a construct
static const RegisterRec registers[] /*PROGMEM*/ = {
    // 0x00
    //    RegisterRec([]()->uint16_t { return PROJECT_VERSION_MAJOR; }, 1),
    //    // 0x01
    //    RegisterRec([]()->uint16_t { return PROJECT_VERSION_MINOR; }, 1),
    //    // 0x02
    //    RegisterRec([]()->uint16_t { return PROJECT_VERSION_REV; }, 2),
    //    // 0x03
    //    RegisterRec([]()->uint16_t { return PROJECT_BUILD_NUMBER; }, 2),
    // 0x00
    RegisterRec(false, &project_major),
    // 0x01
    RegisterRec(false, &project_minor),
    // 0x02
    RegisterRec(false, &project_revision),
    // 0x03
    RegisterRec(false, &project_build_number),
    // 0x04
    RegisterRec( // MMU errors
        []() -> uint16_t { return mg::globals.DriveErrors(); },
        [](uint16_t) {}, // @@TODO think about setting/clearing the error counter from the outside
        2),
    // 0x05
    RegisterRec([]() -> uint16_t { return application.CurrentProgressCode(); }, 1),
    // 0x06
    RegisterRec([]() -> uint16_t { return application.CurrentErrorCode(); }, 2),
    // 0x07 filamentState
    RegisterRec(
        []() -> uint16_t { return mg::globals.FilamentLoaded(); },
        [](uint16_t v) { return mg::globals.SetFilamentLoaded(mg::globals.ActiveSlot(), static_cast<mg::FilamentLoadState>(v)); },
        1),
    // 0x08 FINDA
    RegisterRec(
        []() -> uint16_t { return static_cast<uint16_t>(mf::finda.Pressed()); },
        1),
    // 09 fsensor
    RegisterRec(
        []() -> uint16_t { return static_cast<uint16_t>(mfs::fsensor.Pressed()); },
        [](uint16_t v) { return mfs::fsensor.ProcessMessage(v != 0); },
        1),
    // 0xa motor mode (stealth = 1/normal = 0)
    RegisterRec([]() -> uint16_t { return static_cast<uint16_t>(mg::globals.MotorsStealth()); }, 1),
    // 0xb extra load distance after fsensor triggered (30mm default)
    RegisterRec(
        []() -> uint16_t { return mg::globals.FSensorToNozzle_mm().v; },
        [](uint16_t d) { mg::globals.SetFSensorToNozzle_mm(d); },
        1),
    // 0x0c fsensor unload check distance (40mm default)
    RegisterRec(
        []() -> uint16_t { return mg::globals.FSensorUnloadCheck_mm().v; },
        [](uint16_t d) { mg::globals.SetFSensorUnloadCheck_mm(d); },
        1),
    // 0xd empty register - one empty register takes 5 bytes of code
    RegisterRec(),
    // 0xe
    RegisterRec(),
    // 0xf
    RegisterRec(),

    // Pulley
    // 0x10 Pulley acceleration RW
    RegisterRec(
        []() -> uint16_t { return config::pulleyLimits.accel.v; },
        //@@TODO
        1),
    // 0x11 2 Pulley fast load feedrate RW
    RegisterRec(
        []() -> uint16_t { return mg::globals.PulleyLoadFeedrate_mm_s().v; },
        [](uint16_t d) { mg::globals.SetPulleyLoadFeedrate_mm_s(d); },
        2),
    // 0x12 2 Pulley slow load to fsensor feedrate RW
    RegisterRec(
        []() -> uint16_t { return mg::globals.PulleySlowFeedrate_mm_s().v; },
        [](uint16_t d) { mg::globals.SetPulleySlowFeedrate_mm_s(d); },
        2),
    // 0x13 2 Pulley unload feedrate RW
    RegisterRec(
        []() -> uint16_t { return mg::globals.PulleyUnloadFeedrate_mm_s().v; },
        [](uint16_t d) { mg::globals.SetPulleyUnloadFeedrate_mm_s(d); },
        2),
    RegisterRec(), // 14
    RegisterRec(), // 15
    RegisterRec(), // 16
    RegisterRec(), // 17
    RegisterRec(), // 18
    RegisterRec(), // 19
    RegisterRec(), // 1a
    RegisterRec(), // 1b
    RegisterRec(), // 1c
    RegisterRec(), // 1d
    RegisterRec(), // 1e
    RegisterRec(), // 1f

    // Selector
    //20 2 Selector acceleration  RW
    RegisterRec(
        []() -> uint16_t { return config::selectorLimits.accel.v; },
        //@@TODO
        1),
    //21 2 Selector nominal speed  RW
    RegisterRec(
        []() -> uint16_t { return mg::globals.SelectorFeedrate_mm_s().v; },
        [](uint16_t d) { mg::globals.SetSelectorFeedrate_mm_s(d); },
        2),
    //22 2 Selector sg_thrs  RW
    RegisterRec(
        []() -> uint16_t { return config::selector.sg_thrs; },
        //@@TODO
        1),
    //23 1 Set/Get Selector slot RW
    RegisterRec(
        []() -> uint16_t { return ms::selector.Slot(); },
        [](uint16_t d) { ms::selector.MoveToSlot(d); },
        1),

    // Idler
    //30 2 Idler acceleration  RW
    RegisterRec(
        []() -> uint16_t { return config::idlerLimits.accel.v; },
        //@@TODO
        1),
    //31 2 Idler nominal speed  RW
    RegisterRec(
        []() -> uint16_t { return mg::globals.IdlerFeedrate_deg_s().v; },
        [](uint16_t d) { mg::globals.SetIdlerFeedrate_deg_s(d); },
        1),
    //32 2 Idler sg_thrs  RW
    RegisterRec(
        []() -> uint16_t { return config::idler.sg_thrs; },
        //@@TODO
        1),
    //33 1 Set/Get Idler slot  RW
    RegisterRec(
        []() -> uint16_t { return mi::idler.Slot(); },
        // [](uint16_t d) { mi::idler.MoveToSlot(d); }, // @@TODO can be theoretically done as well
        1),
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
            value = *static_cast<uint8_t *>(registers[address].A1.addr);
            break;
        case 2:
            value = *static_cast<uint16_t *>(registers[address].A1.addr);
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
            *static_cast<uint8_t *>(registers[address].A1.addr) = value;
            break;
        case 2:
            *static_cast<uint16_t *>(registers[address].A1.addr) = value;
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
