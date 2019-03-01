#!/usr/bin/env bash
set -x

VISION_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# -1. Disable Windowing
sudo systemctl disable lightdm.service
sudo systemctl stop lightdm.service

# 0. Disable automatic updates of all kinds
# https://askubuntu.com/questions/1057458/how-to-remove-ubuntus-automatic-internet-connection-needs/1057463#1057463
sudo apt remove popularity-contest
sudo systemctl stop apt-daily.timer
sudo systemctl disable apt-daily.timer
sudo systemctl disable apt-daily.service
sudo systemctl stop apt-daily-upgrade.timer
sudo systemctl disable apt-daily-upgrade.timer
sudo systemctl disable apt-daily-upgrade.service

sudo dpkg-reconfigure -plow unattended-upgrades
# sudo apt purge update-manager-core # requires internet
sudo apt purge snapd ubuntu-core-launcher squashfs-tools
# sudo systemctl disable snapd.refresh.service # handled by above line


# No openCV 3.3 install; its installed by default on TX1
echo "OpenCV 3.3 should be installed!"

# Configure Camera udev rules
# For this ruleset to work, ensure you are running a TX1 on a J120 with our exact camera spec and updated J120 firmware
# TODO add camera link
sudo cp frc-cams-j120-tx1-firmware-3.0.rules /etc/udev/rules.d/
# Reload rules
sudo udevadm control --reload-rules && sudo udevadm trigger

# Build code
cd ../..
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make del-mar-cams
cd $VISION_DIR
cp bin/del-mar-cams /home/nvidia/

# Configure autostart
cp del-mar-cams.service /etc/systemd/system
sudo systemctl start del-mar-cams
sudo systemctl enable del-mar-cams

# Configure static IP
#sudo systemctl stop NetworkManager.service
#sudo systemctl disable NetworkManager.service
#cp frc-static-eth0 /etc/network/interfaces.d/
#ifup eth0
