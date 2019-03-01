#include "config.hpp"
#include "mjpeg_stream.hpp"

#include <cscore_oo.h>
#include <iomanip>
#include <iostream>
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

    // char buf[512];
    // static_assert(sizeof(buf) == 512, "MISMATCHED SIZEOF");
    // memset(buf, 0, sizeof(buf));
    // int count = readlink(CAM_FORWARD_ID.c_str(), buf, sizeof(buf) - 1);
    // if (count <= 0) {
    //     wpi::outs() << "Could not resolve link to " << CAM_FORWARD_ID << '\n';
    //     return 1;
    // }
    // std::string camForwardResolved(buf);

    // memset(buf, 0, sizeof(buf));
    // count = readlink(CAM_REVERSE_ID.c_str(), buf, sizeof(buf) - 1);
    // if (count <= 0) {
    //     wpi::outs() << "Could not resolve link to " << CAM_REVERSE_ID << '\n';
    //     return 1;
    // }
    // std::string camReverseResolved(buf);

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
    std::getchar();
}
