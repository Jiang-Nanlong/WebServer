#define INFO 0
#define DEBUG 1
#define ERROR 2

#define LOG(level, format, ...) do{\
        if (level < INFO) break;\
        time_t t = time(NULL);\
        struct tm *ltm = localtime(&t);\
        char time[32] = {0};\
        strftime(time, 31, "%H:%M:%S", ltm);\
        fprintf(stdout, "[%p %s %s:%d] " format "\n",(void*)pthread_self(),time, __FILE__, __LINE__, ##__VA_ARGS__);\
    }while(0)


// 日志先用简单的宏实现，并且不提供写文件的操作，仅输出提示消息