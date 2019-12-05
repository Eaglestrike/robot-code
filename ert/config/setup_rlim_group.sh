set -Eux

sudo groupadd -f aos
# logname is expanded before sudo is run
sudo usermod -a -G aos "$LOGNAME"
