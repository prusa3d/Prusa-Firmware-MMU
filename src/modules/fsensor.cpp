#include "fsensor.h"
#include "timebase.h"

namespace modules {
namespace fsensor {

FSensor fsensor;

void FSensor::Step() {
    debounce::Debouncer::Step(modules::time::timebase.Millis(), reportedFSensorState);
}

void FSensor::ProcessMessage(bool on) {
    reportedFSensorState = on;
}

} // namespace fsensor
} // namespace modules
