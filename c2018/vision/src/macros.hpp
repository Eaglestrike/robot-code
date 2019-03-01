// defines DEBUG
#include "config.hpp"

#ifdef DEBUG
#define ALGORITHM_PARAM(NAME, DEFAULT, max) \
    int NAME = DEFAULT;                     \
    createTrackbar(#NAME ":", "params", &NAME, max);
#else
#define ALGORITHM_PARAM(name, default, max) \
    const int name = default;
#endif

#ifdef DEBUG
#define SHOW(name, mat)                 \
    namedWindow(name, WINDOW_NORMAL);   \
    resizeWindow(name, Size(450, 450)); \
    imshow(name, mat);
#define SHOW_CHANNEL(name, mat, channel)   \
    {                                      \
        Mat tmp;                           \
        extractChannel(mat, tmp, channel); \
        SHOW(name, tmp)                    \
    }
#define DBP(x) std::cout << x << std::endl;
#else
#define SHOW(name, mat) ;
#define SHOW_CHANNEL(name, mat, channel) ;
#define DBP(x) ;
#endif

#define SHOW_ALWAYS(name, mat)          \
    namedWindow(name, WINDOW_NORMAL);   \
    resizeWindow(name, Size(450, 450)); \
    imshow(name, mat);
