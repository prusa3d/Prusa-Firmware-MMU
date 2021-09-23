#include "usb_cdc.h"
#include "../hal/cpu.h"
#include "../hal/usart.h"
#include "../hal/watchdog.h"

extern "C" {
#include "lufa_config.h"
#include "Descriptors.h"
#include "lufa/LUFA/Drivers/USB/USB.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface = {
    .Config = {
        .ControlInterfaceNumber = INTERFACE_ID_CDC_CCI,
        .DataINEndpoint = {
            .Address = CDC_TX_EPADDR,
            .Size = CDC_TXRX_EPSIZE,
            .Type = EP_TYPE_BULK,
            .Banks = 1,
        },
        .DataOUTEndpoint = {
            .Address = CDC_RX_EPADDR,
            .Size = CDC_TXRX_EPSIZE,
            .Type = EP_TYPE_BULK,
            .Banks = 1,
        },
        .NotificationEndpoint = {
            .Address = CDC_NOTIFICATION_EPADDR,
            .Size = CDC_NOTIFICATION_EPSIZE,
            .Type = EP_TYPE_INTERRUPT,
            .Banks = 1,
        },
    },
};

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void) {
    hal::usart::usart1.puts("EVENT_USB_Device_Connect\n");
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void) {
    hal::usart::usart1.puts("EVENT_USB_Device_Disconnect\n");
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void) {
    bool ConfigSuccess = true;

    ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

    // LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
    // char str1[] = "ready\n";
    // char str0[] = "error\n";
    // hal::usart::usart1.puts("EVENT_USB_Device_ConfigurationChanged:");
    // hal::usart::usart1.puts(ConfigSuccess ? str1 : str0);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void) {
    // hal::usart::usart1.puts("EVENT_USB_Device_ControlRequest\n");
    CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo) {
    // Printing to serial from here will make Windows commit suicide when opening the port

    // hal::usart::usart1.puts("EVENT_CDC_Device_ControLineStateChanged ");
    // bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
    // char str[50];
    // sprintf_P(str, PSTR("DTR:%hu\n"), HostReady);
    // hal::usart::usart1.puts(str);
}

void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo) {
    // Printing to serial from here will make Windows commit suicide when opening the port

    // hal::usart::usart1.puts("EVENT_CDC_Device_LineEncodingChanged ");
    // char str[50];
    // sprintf_P(str, PSTR("baud:%lu\n"), CDCInterfaceInfo->State.LineEncoding.BaudRateBPS);
    // hal::usart::usart1.puts(str);

    if (CDCInterfaceInfo->State.LineEncoding.BaudRateBPS == 1200) {
        // *(uint16_t *)0x0800U = 0x7777; //old bootloader?
        *(uint16_t *)(RAMEND - 1) = 0x7777;
        hal::cpu::resetPending = true;
        hal::watchdog::Enable(hal::watchdog::configuration::compute(250));
    }
}
}

namespace modules {
namespace usb {

CDC cdc;

void CDC::Init() {
    USB_Init();
}

void CDC::Step() {
    CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
    USB_USBTask();
}

} // namespace usb

} // namespace modules
