// re-defines logging to just die on fatal

#include <stdio.h>

#define FATAL 4

#define AOS_LOG(level, format, args...)                                  \
    do                                                                   \
    {                                                                    \
        if (level == FATAL)                                              \
        {                                                                \
            fprintf(stderr, "log_do(FATAL) fell through!!!!!\n");        \
            printf("see stderr\n");                                      \
            abort();                                                     \
        }                                                                \
        else                                                             \
        {                                                                \
            fprintf(stderr, "AOS_LOG called, wish I knew what it said"); \
        }                                                                \
    } while (0)

// Like PLOG except allows specifying an error other than errno.
#define AOS_PELOG(level, error_in, format, args...)               \
    do                                                            \
    {                                                             \
        const int error = error_in;                               \
        AOS_LOG(level, format " due to %d (%s)\n", ##args, error, \
                aos_strerror(error));                             \
    } while (0);

// Same as LOG except appends " due to %d (%s)\n" (formatted with errno and
// aos_strerror(errno)) to the message.
#define AOS_PLOG(level, format, args...) AOS_PELOG(level, errno, format, ##args)
