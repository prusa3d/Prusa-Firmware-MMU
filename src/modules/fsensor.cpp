#include "fsensor.h"

namespace modules {
namespace fsensor {

FSensor fsensor;

void FSensor::Step(uint16_t time) {
    debounce::Debouncer::Step(time, reportedFSensorState);
}

void FSensor::ProcessMessage(bool on) {
    reportedFSensorState = on;
}

} // namespace finda
} // namespace modules
