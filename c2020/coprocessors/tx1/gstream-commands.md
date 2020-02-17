# Gstreamer CLI

## Sending

The nannycam script is basically a wrapper around these commands:

```bash
v4l2-ctl -d /dev/video0 -c exposure_auto=1 -c exposure_absolute=300
gst-launch-1.0 -v v4l2src device=/dev/video0 ! "video/x-raw,width=320,height=240,framerate=30/1" ! x264enc speed-preset=1 tune=zerolatency bitrate=512 ! rtph264pay ! udpsink host=10.1.14.5 port=5808 &
v4l2-ctl -d /dev/video1 -c exposure_auto=1 -c exposure_absolute=300
gst-launch-1.0 -v v4l2src device=/dev/video1 ! "video/x-raw,width=320,height=240,framerate=30/1" ! x264enc speed-preset=1 tune=zerolatency bitrate=512 ! rtph264pay ! udpsink host=10.1.14.5 port=5809 &
wait
```

but adds functionality to deal without disconnects and changing camera IDs. This is needed because gstreamer doesn't like USB-port based names, only `/dev/videoX`, and the cameras we use are indistinguishable to udev.

## Receiving

This was somewhere in the 2019 history:

```sh
gst-launch-1.0 udpsrc port=5000 ! application/x-rtp, clock-rate=90000,payload=96 ! rtph263pdepay queue-delay=0 ! ffdec_h263 ! xvimagesink
```

Not sure how exactly this differs from the commands below, but it seems broken at the moment.

This command works to receive on linux:

```sh
gst-launch-1.0 udpsrc port={} caps="application/x-rtp" ! rtph264depay ! avdec_h264 ! autovideosink sync=false
```

Currently the driver stations has two batch scripts that run this command, but with ports 5808 and 5809.

```bat
gst-launch-1.0 udpsrc port=5000 caps="application/x-rtp" ! rtph264depay ! avdec_h264 ! autovideosink sync=false
```
