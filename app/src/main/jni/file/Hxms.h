//
// Created by Hxms on 2016/4/15.
//

#ifndef FILEHOOK_HXMS_H
#define FILEHOOK_HXMS_H

#include <sys/types.h>
#include <android/log.h>
#include <dirent.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <string>
#include <map>
#include <sstream>
#include <regex>
#include <mutex>

#include "../saurikit/cydia_substrate/substrate.h"

#define TAG "FILEHOOK-JNI" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

using namespace std;

namespace  Hxms {
    namespace Hook {

        typedef FILE MF;

        class HxmsFHook {

        private:
            string rawContent;
            string content;
            MF file;
            int curPos;

        public:
            HxmsFHook(const string &rawContent, const string &content, const MF &file) : rawContent(
                    rawContent), content(content), file(file), curPos(0) { }

        public:
            const string &getRawContent() const {
                return rawContent;
            }

            const string &getContent() const {
                return content;
            }

            const MF &getFile() const {
                return file;
            }

            const int readFile(char *buffer, int size);
        };


        // 线程锁变量
        static pthread_rwlock_t rwlock_t;

        // 自定义文件打开函数
        MF *my_fopen(const char *path, const char *mode);

        // 自定义文件读取函数
        size_t my_fread(void *buffer, size_t size, size_t count, MF *stream);

        // 自定义文件关闭函数
        int my_fclose(MF *fp);

        // 读取文件函数
        string readFileFromSystem(string path, MF *pFile);

        // 关注文件
        void watchFile(string path, MF *pFile);

        bool replaceWithRegex(bool *hasModify, string &content, regex &regex, string &replaceText);

        // 使用的正则表达式
        static regex tracePid("TracerPid:\t\\d+");

        static regex tracingStop("t (tracing stop)");

        static regex ptraceStop("ptrace_stop");

        static regex tracePrefix(" t ");

        // 定义修改过的文件哈希表

        static map<int, HxmsFHook *> fileTable;

        // 定义原始函数地址
        static MF *(*old_fopen)(const char *, const char *) = NULL;

        static size_t (*old_fread)(void *buffer, size_t size, size_t count, MF *stream) = NULL;

        static int (*old_fclose)(MF *fp) = NULL;
    }
}


#endif //FILEHOOK_HXMS_H
