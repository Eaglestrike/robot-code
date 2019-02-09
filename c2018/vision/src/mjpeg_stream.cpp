#include "mjpeg_stream.hpp"
#include "config.hpp"

#include <cscore_oo.h>
#include <iomanip>
#include <iostream>
#include <wpi/raw_ostream.h>

using namespace cs;
using namespace team114::c2019::vision;

void create_server(std::string name, int cam_id)
{
    wpi::outs() << "hostname: " << cs::GetHostname() << '\n';
    wpi::outs() << "IPv4 network addresses:\n";
    for (const auto &addr : cs::GetNetworkInterfaces())
        wpi::outs() << "  " << addr << '\n';
    UsbCamera camera{name, cam_id};
    camera.SetVideoMode(cs::VideoMode::kMJPEG, MJPEG_WIDTH, MJPEG_HEIGHT, MJPEG_FPS);
    cs::MjpegServer mjpegServer{"httpserver", MJPEG_PORT};
    mjpegServer.SetSource(camera);
    mjpegServer.SetFPS(MJPEG_FPS);

    CS_Status status = 0;
#ifdef DEBUG
    std::cout << std::fixed << std::showpoint << std::setprecision(4);
    cs::AddListener(
        [&](const cs::RawEvent &event) {
            std::cout << "FPS=" << camera.GetActualFPS() << " MBPS=" << (camera.GetActualDataRate() / 1000000.0) << std::endl;
        },
        cs::RawEvent::kTelemetryUpdated, false, &status);
    cs::SetTelemetryPeriod(1.0);
#endif
}
