// #pragma once

// #define INFO 0
// #define DEBUG 1
// #define ERROR 2
// #define FATAL 3

// #define LOG(level, format, ...) \
//     do{ \
//         if (level < INFO) break; \
//         time_t t = time(NULL); \
//         struct tm *ltm = localtime(&t); \
//         char time[32] = {0}; \
//         strftime(time, 31, "%H:%M:%S", ltm);\
//         switch(level){\
//         case(0):fprintf(stdout,"[INFO]: ");break; \
//         case(1):fprintf(stdout,"[DEBUG]: ");break; \
//         case(2):fprintf(stdout,"[ERROR]: ");break; \
//         case(3):fprintf(stdout,"[FATAL]: ");break; \
//         }; \
//         fprintf(stdout, "[%p %s %s:%d] " format '\n', (void*)pthread_self(), time, __FILE__, __LINE__, ##__VA_ARGS__); \
//         if(level == FATAL) exit(1); \
//     } while (0)


// 日志先用简单的宏实现，并且不提供写文件的操作，仅输出提示消息

#pragma once

#include <stdio.h>
#include <time.h>
#include <pthread.h>

#define INFO 0
#define DEBUG 1
#define ERROR 2
#define FATAL 3

#define LOG(level, format, ...) \
    do { \
        if (level < INFO) break; \
        time_t t = time(NULL); \
        struct tm *ltm = localtime(&t); \
        char time[32] = {0}; \
        strftime(time, 31, "%H:%M:%S", ltm); \
        switch (level) { \
        case INFO: fprintf(stdout, "[INFO]: "); break; \
        case DEBUG: fprintf(stdout, "[DEBUG]: "); break; \
        case ERROR: fprintf(stdout, "[ERROR]: "); break; \
        case FATAL: fprintf(stdout, "[FATAL]: "); break; \
        } \
        fprintf(stdout, "[%p %s %s:%d] ", (void*)pthread_self(), time, __FILE__, __LINE__); \
        fprintf(stdout, format, ##__VA_ARGS__); \
        fprintf(stdout, "\n"); \
        if (level == FATAL) exit(1); \
    } while (0)
