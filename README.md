# Switch Bluetooth Joy

Расширенная реализация Bluetooth-функциональности для Nintendo Switch, позволяющая использовать консоль в режиме джостика

## Особенности

- Поддержка режима хоста (Host) и устройства (Device)
- Расширенное управление Bluetooth-подключениями
- Обработка HID-событий
- Поддержка различных типов Bluetooth-устройств
- Встроенная обработка ошибок и отчетность

## Требования

- devkitPro с установленным devkitA64
- libnx (последняя версия)
- Прошивка Nintendo Switch с поддержкой Bluetooth

## Установка

1. Установите devkitPro и необходимые компоненты:
```bash
# На Linux
wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.deb
sudo dpkg -i devkitpro-pacman.deb
sudo dkp-pacman -S switch-dev
```

2. Настройте переменные окружения:
```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
export DEVKITPPC=/opt/devkitpro/devkitPPC
```

3. Клонируйте репозиторий:
```bash
git clone https://github.com/yourusername/switch_bt_joy.git
cd switch_bt_joy
```

4. Соберите проект:
```bash
make clean
make
```

## Использование

1. Скопируйте файл `switch_bt_joy.nro` на SD-карту вашей Nintendo Switch в папку `/switch/`
2. Запустите приложение через меню Homebrew

### Основные функции

- Инициализация Bluetooth:
```cpp
BluetoothHandler bt;
if (!bt.initialize()) {
    // Обработка ошибки
}
```

- Включение режима поиска устройств:
```cpp
if (!bt.startAdvertising()) {
    // Обработка ошибки
}
```

- Ожидание подключения:
```cpp
if (!bt.waitForConnection(5000)) { // таймаут 5 секунд
    // Обработка ошибки
}
```

## Структура проекта

```
switch_bt_joy/
├── source/
│   ├── bluetooth/
│   │   ├── bluetooth_core.cpp
│   │   ├── bluetooth_core.hpp
│   │   └── bluetooth_types.hpp
│   ├── bluetooth_handler.cpp
│   ├── bluetooth_handler.h
│   └── main.cpp
├── Makefile
├── icon.jpg
└── README.md
```

## Архитектура

Проект использует трехуровневую архитектуру:

1. **Верхний уровень** (`bluetooth_handler.h/cpp`):
   - Предоставляет удобный API для работы с Bluetooth
   - Управляет состоянием подключения
   - Обрабатывает ошибки

2. **Средний уровень** (`bluetooth_core.hpp/cpp`):
   - Абстрагирует системные вызовы
   - Реализует базовые Bluetooth-операции
   - Управляет режимами работы

3. **Нижний уровень** (`bluetooth_types.hpp`):
   - Определяет типы данных
   - Предоставляет константы и перечисления
   - Обеспечивает совместимость типов

## Известные проблемы

- Возможны проблемы совместимости с некоторыми версиями прошивки
- Ограниченная поддержка некоторых типов Bluetooth-устройств
- Требуется тестирование на различных версиях системного ПО

## Вклад в проект

1. Форкните репозиторий
2. Создайте ветку для ваших изменений
3. Внесите изменения
4. Отправьте pull request

## Лицензия

MIT License. См. файл LICENSE для подробностей.

## Авторы

- Основной разработчик: [Ваше имя]
- Соавторы: [Список соавторов]

## Благодарности

- Команде devkitPro за отличный инструментарий
- Сообществу Nintendo Switch homebrew за поддержку и тестирование
- Всем контрибьюторам за помощь в развитии проекта
