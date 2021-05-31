#pragma once

/// @@TODO @3d-gussner
/// Extract the current state machines of high-level operations (load fillament, unload fillament etc.) here
/// Design some nice non-blocking API for these operations

namespace logic {

// schvalne zkusime udelat operaci unload filament

class Logic {

public:
    inline Logic() = default;

    void UnloadFilament();
};

} // namespace logic
