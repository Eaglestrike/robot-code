
#include "HeaderUtilMacros.h"

SYSTEM_INCLUDE_BEGIN
#include "gtest/gtest.h"
#include <hal/HAL.h>
SYSTEM_INCLUDE_END

int main(int argc, char **argv)
{
    HAL_Initialize(500, 0);
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
