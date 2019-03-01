#include "config.hpp"
#include "mjpeg_stream.hpp"

#include <chrono>
#include <cscore_oo.h>
#include <iomanip>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <wpi/raw_ostream.h>

using namespace cs;
using namespace team114::c2019::vision;

int main(int argc, char **argv)
{
    wpi::outs() << "hostname: " << cs::GetHostname() << '\n';
    wpi::outs() << "IPv4 network addresses:\n";
    for (const auto &addr : cs::GetNetworkInterfaces())
        wpi::outs() << "  " << addr << '\n';

    UsbCamera fcam{"ForwardCamera", CAM_FORWARD_ID};
    fcam.SetVideoMode(cs::VideoMode::kMJPEG, MJPEG_WIDTH, MJPEG_HEIGHT, MJPEG_FPS);
    cs::MjpegServer fMjpegServer{"ForwardHTTPMjpeg", MJPEG_FORWARD_PORT};
    fMjpegServer.SetSource(fcam);
    fMjpegServer.SetFPS(MJPEG_FPS);

    UsbCamera rcam{"ReverseCamera", CAM_REVERSE_ID};
    fcam.SetVideoMode(cs::VideoMode::kMJPEG, MJPEG_WIDTH, MJPEG_HEIGHT, MJPEG_FPS);
    cs::MjpegServer rMjpegServer{"ReverseHTTPMjpeg", MJPEG_REVERSE_PORT};
    rMjpegServer.SetSource(rcam);
    rMjpegServer.SetFPS(MJPEG_FPS);

    CS_Status status = 0;
#ifdef DEBUG
    std::cout << std::fixed << std::showpoint << std::setprecision(4);
    cs::AddListener(
        [&](const cs::RawEvent &event) {
            std::cout << "FFPS=" << fcam.GetActualFPS() << " FMBPS=" << (fcam.GetActualDataRate() / 1000000.0) << std::endl;
            std::cout << "RFPS=" << rcam.GetActualFPS() << " RMBPS=" << (rcam.GetActualDataRate() / 1000000.0) << std::endl;
        },
        cs::RawEvent::kTelemetryUpdated, false, &status);
    cs::SetTelemetryPeriod(2.5);
#endif
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
