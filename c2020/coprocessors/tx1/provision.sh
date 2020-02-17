#!/usr/bin/env bash

# Since we are running a TX1 on a Auvidea J120, we are at the behest of Auvidea
# firmware devs if we want two USB ports, which we do. So, we're stuck on
# Jetpack 3.3 this year using a firmware release from August 2018. On the
# bright side, this was the same as 2019, so in all likelihood this script will
# never be need to be used. The boards are running Auvidea firmware version 3.0,
# Jetpack 3.3, L4T 28.2

set -eux

# Disable Windowing
sudo systemctl disable lightdm.service
sudo systemctl stop lightdm.service

# Disable automatic updates of all kinds
# https://askubuntu.com/questions/1057458/how-to-remove-ubuntus-automatic-internet-connection-needs/1057463#1057463
sudo apt remove popularity-contest
sudo systemctl stop apt-daily.timer
sudo systemctl disable apt-daily.timer
sudo systemctl disable apt-daily.service

sudo dpkg-reconfigure -plow unattended-upgrades
# sudo apt purge update-manager-core # requires internet
sudo apt purge snapd ubuntu-core-launcher squashfs-tools
# sudo systemctl disable snapd.refresh.service # handled by above line

# Install deps of gstreamer streams
sudo apt install v4l-utils # CLI camera configuration
sudo apt install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools

# Nice to haves, just in case
sudo apt install build-essential cmake g++

# TODO nannycam autostart?

# Configure static IP -- not idempotent
#sudo systemctl stop NetworkManager.service
#sudo systemctl disable NetworkManager.service
#cp frc-static-eth0 /etc/network/interfaces.d/
#ifup eth0
