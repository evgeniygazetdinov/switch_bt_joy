#pragma once
#include "bluetooth_types.hpp"

namespace bluetooth {
namespace core {

    /**
     * @brief Инициализация Bluetooth стека
     */
    Result Initialize();

    /**
     * @brief Освобождение ресурсов Bluetooth стека
     */
    void Finalize();

    /**
     * @brief Получение информации об устройстве
     * @param address MAC-адрес устройства
     * @param out_device_info Указатель на структуру для записи информации
     */
    Result GetDeviceInfo(const Address *address, DeviceInfo *out_device_info);

    /**
     * @brief Отключение от устройства
     * @param address MAC-адрес устройства
     */
    Result Disconnect(const Address *address);

    /**
     * @brief Регистрация обработчика событий
     * @param event_type Тип события
     * @param event Указатель на событие
     */
    Result RegisterEventCallback(EventType event_type, Event* event);

    /**
     * @brief Получение информации о событии
     * @param out_type Тип события
     * @param out_buffer Буфер для данных события
     * @param size Размер буфера
     */
    Result GetEventInfo(EventType *out_type, void *out_buffer, size_t size);

    /**
     * @brief Начать поиск устройств
     */
    Result StartDiscovery();

    /**
     * @brief Остановить поиск устройств
     */
    Result CancelDiscovery();

    /**
     * @brief Установить режим работы Bluetooth
     * @param mode Режим работы
     */
    Result SetMode(BluetoothMode mode);

    /**
     * @brief Включить Bluetooth
     */
    Result Enable();

    /**
     * @brief Выключить Bluetooth
     */
    Result Disable();

} // namespace core
} // namespace bluetooth
