#pragma once
#include <switch.h>

namespace bluetooth {

    using Address = ::BtdrvAddress;
    using DeviceClass = ::BtdrvClassOfDevice;
    using PinCode = ::BtdrvBluetoothPinCode;
    using AdapterProperty = ::BtdrvAdapterProperty;
    using HidReport = ::BtdrvHidReport;
    using HhReportType = ::BtdrvBluetoothHhReportType;

    using EventType = ::BtdrvEventType;
    using EventInfo = ::BtdrvEventInfo;

    using HidEventType = ::BtdrvHidEventType;
    using HidEventInfo = ::BtdrvHidEventInfo;

    // Константы для типов событий
    constexpr EventType EventType_Connection = ::BtdrvEventType_Connection;
    // TODO: добавить другие типы событий когда они будут доступны в SDK
    
    // Константы для типов HID отчетов
    constexpr HhReportType HhReportType_Input = ::BtdrvBluetoothHhReportType_Input;
    constexpr HhReportType HhReportType_Output = ::BtdrvBluetoothHhReportType_Output;
    constexpr HhReportType HhReportType_Feature = ::BtdrvBluetoothHhReportType_Feature;

    struct DeviceInfo {
        Address address;
        char name[0xFF];
        DeviceClass cod;
        bool paired;
        bool connected;
    };

    enum class BluetoothMode {
        None,
        Host,    // Switch работает как хост (принимает подключения)
        Device   // Switch работает как устройство (подключается к другим)
    };

}
