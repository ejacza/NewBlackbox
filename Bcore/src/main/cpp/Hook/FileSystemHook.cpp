#include "FileSystemHook.h"
#include "Log.h"
#include <shadowhook.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <cstring>
#include <errno.h>

static int (*orig_open)(const char *pathname, int flags, ...) = nullptr;
static int (*orig_open64)(const char *pathname, int flags, ...) = nullptr;

int new_open(const char *pathname, int flags, ...) {
    if (pathname != nullptr) {
        if (strstr(pathname, "resource-cache") ||
            strstr(pathname, "@idmap") ||
            strstr(pathname, ".frro") ||
            strstr(pathname, "systemui") ||
            strstr(pathname, "data@resource-cache@")) {
            ALOGD("FileSystemHook: Blocking problematic file access: %s", pathname);
            errno = ENOENT;
            return -1;
        }
    }

    mode_t mode = 0;
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    return (orig_open != nullptr) ? orig_open(pathname, flags, mode) : -1;
}

int new_open64(const char *pathname, int flags, ...) {
    if (pathname != nullptr) {
        if (strstr(pathname, "resource-cache") ||
            strstr(pathname, "@idmap") ||
            strstr(pathname, ".frro") ||
            strstr(pathname, "systemui") ||
            strstr(pathname, "data@resource-cache@")) {
            ALOGD("FileSystemHook: Blocking problematic file access (64): %s", pathname);
            errno = ENOENT;
            return -1;
        }
    }

    mode_t mode = 0;
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    return (orig_open64 != nullptr) ? orig_open64(pathname, flags, mode) : -1;
}

void FileSystemHook::init() {
    ALOGD("FileSystemHook: Initializing file system hooks with ShadowHook");

    void *stub_open = shadowhook_hook_sym_name("libc.so", "open", (void *)new_open, (void **)&orig_open);
    if (stub_open) {
        ALOGD("FileSystemHook: Hooked open successfully");
    } else {
        ALOGE("FileSystemHook: Failed to hook open: %d", shadowhook_get_errno());
    }

    void *stub_open64 = shadowhook_hook_sym_name("libc.so", "open64", (void *)new_open64, (void **)&orig_open64);
    if (stub_open64) {
        ALOGD("FileSystemHook: Hooked open64 successfully");
    } else {
        ALOGE("FileSystemHook: Failed to hook open64: %d", shadowhook_get_errno());
    }
}
