//
// Created by Hxms on 2016/4/15.
//

#include "Hxms.h"

using namespace Hxms::Hook;

MSConfig(MSFilterLibrary, "/system/lib/libc.so")

// this is a macro that uses __attribute__((__constructor__))
MSInitialize {
    MSImageRef image;
    char outputBuf[0x100];
    char procName[0x50];
    pid_t pid = getpid();

    sprintf(outputBuf, "Hxms Api Hook Substrate [Pid:%d]......", pid);
    LOGD(outputBuf);

    image = MSGetImageByName("/system/lib/libc.so");
    if (image != NULL) {
        void *pFOpen = MSFindSymbol(image, "fopen");
        void *pFRead = MSFindSymbol(image, "fread");
        void *pFClose = MSFindSymbol(image, "fclose");

        if (pFClose != NULL) MSHookFunction(pFClose, (void *) &my_fclose, (void **) &old_fclose);
        else
            LOGD("error find fclose");


        if (pFOpen != NULL) MSHookFunction(pFOpen, (void *) &my_fopen, (void **) &old_fopen);
        else
            LOGD("error find fopen");

        if (pFRead != NULL) MSHookFunction(pFRead, (void *) &my_fread, (void **) &old_fread);
        else
            LOGD("error find fread");
    }
    else {
        LOGD("ERROR FIND /system/lib/libc.so");
    }
    pthread_rwlock_init(&rwlock_t, NULL);
}


MF *Hxms::Hook::my_fopen(const char *path, const char *mode) {
    pid_t pid = getpid();
    pid_t tid = gettid();
    // 执行读取操作
    MF *result = old_fopen(path, mode);


    LOGI("Looking fopen [%d:%d] ----  %s ", pid, tid, path);

    // 关注文件
    if (result && strstr(path, "/proc")) {
        watchFile(path, result);
    }
    return result;
}

size_t Hxms::Hook::my_fread(void *buffer, size_t size, size_t count, MF *stream) {
    int result = -1;
    pthread_rwlock_rdlock(&rwlock_t);
    map<int, HxmsFHook *>::iterator it = fileTable.find(stream->_file);
    if (it != fileTable.end()) {
        result = it->second->readFile((char *) buffer, size * count);
    }
    pthread_rwlock_unlock(&rwlock_t);
    if (result == -1)
        result = old_fread(buffer, size, count, stream);
    return result;
}

int Hxms::Hook::my_fclose(MF *fp) {
    HxmsFHook *memFile = NULL;

    pthread_rwlock_wrlock(&rwlock_t);

    map<int, HxmsFHook *>::iterator it = fileTable.find(fp->_file);
    if (it != fileTable.end()) {
        memFile = it->second;
        fileTable.erase(it);
    }

    pthread_rwlock_unlock(&rwlock_t);

    if (memFile) {
        LOGI("关闭文件：%d", memFile->getFile()._file);
        delete memFile;
    }

    return old_fclose(fp);
}

void Hxms::Hook::watchFile(string path, MF *pFile) {
    static string tracePidZero = "TracerPid:\t0";
    static string traceSleeping = "S (sleeping)";
    static string sysEpollWait = "sys_epoll_wait";
    static string s = " S ";
    bool needOverWrite = false;
    bool valid = false;
    string content = "";
    string rawContent = "";
    if (path.find("/status") != string::npos) {
        LOGI("读取操作从:/status");
        rawContent = readFileFromSystem(path, pFile);
        content = rawContent;
        valid = replaceWithRegex(&needOverWrite, content, tracePid, tracePidZero);
        valid = replaceWithRegex(&needOverWrite, content, tracingStop, traceSleeping);
    } else if (path.find("/wchan") != string::npos) {
        LOGI("读取操作从:/wchan");
        rawContent = readFileFromSystem(path, pFile);
        content = rawContent;
        valid = replaceWithRegex(&needOverWrite, content, ptraceStop, sysEpollWait);
    } else if (path.find("/stat") != string::npos) {
        LOGI("读取操作从:/stat");
        rawContent = readFileFromSystem(path, pFile);
        content = rawContent;
        valid = replaceWithRegex(&needOverWrite, content, tracePrefix, s);
    }
    if (needOverWrite) {
        LOGI("生成文件：");
        LOGI(path.c_str());
        LOGI(rawContent.c_str());
        LOGI(content.c_str());

        pthread_rwlock_wrlock(&rwlock_t);
        fileTable[pFile->_file] = new HxmsFHook(rawContent, content, *pFile);
        pthread_rwlock_unlock(&rwlock_t);
    }
}

bool Hxms::Hook::replaceWithRegex(bool *hasModify, string &content, regex &regex,
                                  string &replaceText) {
    bool result = false;
    result = regex_match(content, tracePid);
    if (result) {
        *hasModify = true;
        content = regex_replace(content, tracePid, replaceText);
    }
    return result;
}


string Hxms::Hook::readFileFromSystem(string path, MF *pFile) {
    char *result = NULL;
    string ret = "";
    int blockSize = 0x100;
    int blockCount = 0;
    if (!pFile) return NULL;
    result = (char *) calloc(sizeof(char), blockSize);
    do {
        int realSize;
        char *curPoint = result + blockCount * blockSize;
        LOGI("调用 calloc 成功  10 %s %d %d, ", path.c_str(), result, curPoint);
        realSize = fread(curPoint, sizeof(char), blockSize, pFile);
        LOGI("读取文件大小： %d", realSize);
        if (blockSize > realSize) {
            fseek(pFile, 0, SEEK_SET);
            break;
        }
        else {
            blockCount++;
            int reallocSize = (blockCount + 1) * blockSize;
            LOGI("realloc 大小 : %d", reallocSize);
            result = (char *) realloc(result, reallocSize);
            memset(result + blockCount * blockSize, 0, blockSize);
        }
    } while (true);
    ret = string(result);
    LOGI("readFile ::: %s - %s", path.c_str(), ret.c_str());
    free(result);

    return ret;
}


const int HxmsFHook::readFile(char *buffer, int size) {
    const char *str = this->content.c_str();
    int length = this->content.length();
    int available = length - this->curPos;
    int realRead = size <= available ? size : 0;
    if (realRead) {
        memset((void *) buffer, 0, size);
        for (int i = 0; i < realRead; ++i) {
            buffer[i] = str[this->curPos];
            this->curPos++;
        }
    }
    return realRead;
}
