set -x
interactive-calibration/build/opencv_interactive-calibration -v=/dev/video0 -of=${CAM_SERIAL_NO}.xml "$(< circle-board-params.sh)"
set +x
