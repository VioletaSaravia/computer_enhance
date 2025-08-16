#ifndef LOG_LEVEL
#define LOG_LEVEL 0
#endif

#if LOG_LEVEL <= 0
#define LOG_INFO(fmt, ...) \
    printf("[INFO] [%s] " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL <= 1
#define LOG_WARNING(fmt, ...) \
    printf("[WARNING] [%s] " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define LOG_WARNING(fmt, ...)
#endif

#if LOG_LEVEL <= 2
#define LOG_ERROR(fmt, ...) \
    printf("[ERROR] [%s] " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif

#if LOG_LEVEL <= 3
#define LOG_FATAL(fmt, ...)                                        \
    do                                                             \
    {                                                              \
        printf("[FATAL] [%s] " fmt "\n", __func__, ##__VA_ARGS__); \
        abort();                                                   \
    } while (0)
#else
#define LOG_FATAL(fmt, ...)
#endif

#define UNREACHABLE LOG_FATAL("unreachable line %d in %s", __LINE__, __FILE__)

#define PROFILE(msg, code, ...)                                  \
    do                                                           \
    {                                                            \
        LOG_INFO("Started %s", msg);                             \
        uint64_t start = __rdtsc();                              \
        code;                                                    \
        LOG_INFO("Finished %s in %lld", msg, __rdtsc() - start); \
    } while (0)