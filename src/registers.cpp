#include "registers.h"
#include "version.h"

struct RegisterFlags {
    uint8_t writable : 1;
    uint8_t eeprom : 1; // 1: register is located in eeprom
    uint8_t size : 2; // 0: 1 bit, 1: 1 byte, 2: 2 bytes
    constexpr RegisterFlags(bool writable, uint8_t size)
        : writable(writable)
        , eeprom(0)
        , size(size) {}
    constexpr RegisterFlags(bool writable, bool eeprom, uint8_t size)
        : writable(writable)
        , eeprom(eeprom)
        , size(size) {}
};

struct RegisterRec {
    RegisterFlags flags;
    void *address;
    template <typename T>
    constexpr RegisterRec(bool writable, T *address)
        : flags(RegisterFlags(writable, sizeof(T)))
        , address((void *)address) {}
    constexpr RegisterRec(bool writable, bool eeprom, void *eepromAddress, uint8_t bytes)
        : flags(RegisterFlags(writable, eeprom, bytes))
        , address(eepromAddress) {}
};

static const RegisterRec registers[] = {
    RegisterRec(false, &project_major),
    RegisterRec(false, &project_minor),
    RegisterRec(false, &project_build_number),
    RegisterRec(false, true, (void *)0, 2), // @@TODO needs improvement

};

static constexpr uint8_t registersSize = sizeof(registers) / sizeof(RegisterRec);

bool ReadRegister(uint8_t address, uint16_t &value) {
    if (address >= registersSize) {
        return false;
    }
    value = 0;
    if (!registers[address].flags.eeprom) {
        switch (registers[address].flags.size) {
        case 0:
        case 1:
            value = *(uint8_t *)registers[address].address;
            break;
        case 2:
            value = *(uint16_t *)registers[address].address;
            break;
        default:
            return false;
        }
        return true;
    } else {
        return false; // @@TODO
    }
}

bool WriteRegister(uint8_t address, uint16_t value) {
    if (address >= registersSize) {
        return false;
    }
    if (!registers[address].flags.writable) {
        return false;
    }
    if (!registers[address].flags.eeprom) {
        switch (registers[address].flags.size) {
        case 0:
        case 1:
            *(uint8_t *)registers[address].address = value;
            break;
        case 2:
            *(uint16_t *)registers[address].address = value;
            break;
        default:
            return false;
        }
        return true;
    } else {
        return false; // @@TODO
    }
}
