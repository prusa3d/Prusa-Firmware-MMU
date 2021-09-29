bool VerifyEnvironmentState(bool filamentLoaded, uint8_t idlerSlotIndex, uint8_t selectorSlotIndex,
    bool findaPressed, ml::Mode greenLEDMode, ml::Mode redLEDMode) {
    CHECKED_ELSE(mg::globals.FilamentLoaded() == filamentLoaded) {
    return false;
    }
    CHECKED_ELSE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(idlerSlotIndex).v) {
    return false;
    }
    CHECKED_ELSE(mi::idler.Engaged() == (idlerSlotIndex < config::toolCount)) {
    return false;
    }
    CHECKED_ELSE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(selectorSlotIndex).v) {
    return false;
    }
    CHECKED_ELSE(ms::selector.Slot() == selectorSlotIndex) {
    return false;
    }
    CHECKED_ELSE(mf::finda.Pressed() == findaPressed) {
    return false;
    }

    for(uint8_t ledIndex = 0; ledIndex < config::toolCount; ++ledIndex){
        if( ledIndex != selectorSlotIndex ){
            // the other LEDs should be off
            CHECKED_ELSE(ml::leds.Mode(ledIndex, ml::red) == ml::off) {
            return false;
            }
            CHECKED_ELSE(ml::leds.Mode(ledIndex, ml::green) == ml::off) {
            return false;
            }
        } else {
            CHECKED_ELSE(ml::leds.Mode(selectorSlotIndex, ml::red) == redLEDMode) {
            return false;
            }
            CHECKED_ELSE(ml::leds.Mode(selectorSlotIndex, ml::green) == greenLEDMode) {
            return false;
            }
        }
    }
    return true;
}

// LED checked at selector's index
template<typename SM>
bool VerifyState(SM &uf, bool filamentLoaded, uint8_t idlerSlotIndex, uint8_t selectorSlotIndex,
    bool findaPressed, ml::Mode greenLEDMode, ml::Mode redLEDMode, ErrorCode err, ProgressCode topLevelProgress) {

    VerifyEnvironmentState(filamentLoaded, idlerSlotIndex, selectorSlotIndex, findaPressed, greenLEDMode, redLEDMode);

    CHECKED_ELSE(uf.Error() == err) {
        return false;
    }
    CHECKED_ELSE(uf.TopLevelState() == topLevelProgress) {
    return false;
    }
    return true;
}

// LED checked at their own ledCheckIndex index
template<typename SM>
bool VerifyState2(SM &uf, bool filamentLoaded, uint8_t idlerSlotIndex, uint8_t selectorSlotIndex,
    bool findaPressed, uint8_t ledCheckIndex, ml::Mode greenLEDMode, ml::Mode redLEDMode, ErrorCode err, ProgressCode topLevelProgress) {
    CHECKED_ELSE(mg::globals.FilamentLoaded() == filamentLoaded) {
    return false;
    }
    CHECKED_ELSE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(idlerSlotIndex).v) {
    return false;
    }
    CHECKED_ELSE(mi::idler.Engaged() == (idlerSlotIndex < config::toolCount)) { return false; }
    CHECKED_ELSE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(selectorSlotIndex).v) {
    return false;
    }
    CHECKED_ELSE(ms::selector.Slot() == selectorSlotIndex) {
    return false;
    }
    CHECKED_ELSE(mf::finda.Pressed() == findaPressed) {
    return false;
    }

    for(uint8_t ledIndex = 0; ledIndex < config::toolCount; ++ledIndex){
        if( ledIndex != ledCheckIndex ){
            // the other LEDs should be off
            CHECKED_ELSE(ml::leds.Mode(ledIndex, ml::red) == ml::off) {
            return false;
            }
            CHECKED_ELSE(ml::leds.Mode(ledIndex, ml::green) == ml::off) {
            return false;
            }
        } else {
            CHECKED_ELSE(ml::leds.Mode(ledCheckIndex, ml::red) == redLEDMode) {
            return false;
            }
            CHECKED_ELSE(ml::leds.Mode(ledCheckIndex, ml::green) == greenLEDMode) {
            return false;
            }
        }
    }

    CHECKED_ELSE(uf.Error() == err) {
    return false;
    }
    CHECKED_ELSE(uf.TopLevelState() == topLevelProgress) {
    return false;
    }
    return true;
}



template<typename SM>
void InvalidSlot(SM &logicSM,  uint8_t activeSlot, uint8_t invSlot){
    ForceReinitAllAutomata();

    REQUIRE(VerifyEnvironmentState(false, mi::Idler::IdleSlotIndex(), 0, false, ml::off, ml::off));

    EnsureActiveSlotIndex(activeSlot);

    logicSM.Reset(invSlot);
    REQUIRE(VerifyState(logicSM, false, mi::Idler::IdleSlotIndex(), activeSlot, false, ml::off, ml::off, ErrorCode::INVALID_TOOL, ProgressCode::OK));
}
