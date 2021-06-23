template<typename SM>
bool VerifyState(SM &uf, bool filamentLoaded, uint8_t idlerSlotIndex, uint8_t selectorSlotIndex,
    bool findaPressed, ml::Mode greenLEDMode, ml::Mode redLEDMode, ErrorCode err, ProgressCode topLevelProgress) {
    CHECKED_ELSE(mg::globals.FilamentLoaded() == filamentLoaded) { return false; }
    CHECKED_ELSE(mm::axes[mm::Idler].pos == mi::Idler::SlotPosition(idlerSlotIndex)) { return false; }
    CHECKED_ELSE(mi::idler.Engaged() == (idlerSlotIndex < 5)) { return false; }
    CHECKED_ELSE(mm::axes[mm::Selector].pos == ms::Selector::SlotPosition(selectorSlotIndex)) { return false; }
    CHECKED_ELSE(ms::selector.Slot() == selectorSlotIndex) { return false; }
    CHECKED_ELSE(mf::finda.Pressed() == findaPressed) { return false; }
    CHECKED_ELSE(ml::leds.Mode(selectorSlotIndex, ml::red) == redLEDMode) { return false; }
    CHECKED_ELSE(ml::leds.Mode(selectorSlotIndex, ml::green) == greenLEDMode) { return false; }
    CHECKED_ELSE(uf.Error() == err) { return false; }
    CHECKED_ELSE(uf.TopLevelState() == topLevelProgress) { return false; }
    return true;
}
