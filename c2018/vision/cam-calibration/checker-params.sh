-t=chessboard \ # use circles board pattern
-sz=22.33 \ # distance between two nearest centers of circles or squares on calibration board
# currentyl set to 134 mm / 6
-dst=200 \ # distance between white and black parts of daulCircles pattern
-w=6 \ # width of pattern (in corners or circles)
-h=4 \ # height of pattern (in corners or circles)
-of=${CAM_SERIAL_NO}.xml \ # output file name
-ft=true \ # auto tuning of calibration flags
# -vis=[grid]: captured boards visualization (grid, window)
-d=0.2 \ # delay between captures in seconds
