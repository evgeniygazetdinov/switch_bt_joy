#!/bin/bash

# Проверяем, запущен ли скрипт с правами root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Устанавливаем базовые инструменты
echo "Installing basic tools..."
apt-get update
apt-get install -y wget git make python3

# Создаем директорию для devkitPro
echo "Creating devkitPro directory..."
mkdir -p /opt/devkitpro
chmod 777 /opt/devkitpro

# Скачиваем и устанавливаем devkitPro pacman
echo "Installing devkitPro pacman..."
wget https://apt.devkitpro.org/install-devkitpro-pacman
chmod +x ./install-devkitpro-pacman
./install-devkitpro-pacman
rm ./install-devkitpro-pacman

# Обновляем базу данных pacman
echo "Updating pacman database..."
dkp-pacman -Sy

# Устанавливаем необходимые пакеты для разработки под Switch
echo "Installing Switch development packages..."
dkp-pacman -S --needed --noconfirm switch-dev switch-portlibs

# Устанавливаем переменные окружения
echo "Setting up environment variables..."
echo 'export DEVKITPRO=/opt/devkitpro' >> ~/.bashrc
echo 'export DEVKITARM=/opt/devkitpro/devkitARM' >> ~/.bashrc
echo 'export DEVKITPPC=/opt/devkitpro/devkitPPC' >> ~/.bashrc
echo 'export PATH=$PATH:/opt/devkitpro/tools/bin' >> ~/.bashrc

# Применяем изменения в текущей сессии
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
export DEVKITPPC=/opt/devkitpro/devkitPPC
export PATH=$PATH:/opt/devkitpro/tools/bin

echo "Installation complete!"
echo "Please restart your terminal or run 'source ~/.bashrc' to apply environment changes"
echo "You can now build Switch applications using 'make'
