#include "FileSystemHook.h"
#include "Log.h"
#include "IO.h"
#include <shadowhook.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <cstring>
#include <errno.h>
#include <unistd.h>

static int (*orig_open)(const char *pathname, int flags, ...) = nullptr;
static int (*orig_open64)(const char *pathname, int flags, ...) = nullptr;
static int (*orig_openat)(int dirfd, const char *pathname, int flags, ...) = nullptr;
static int (*orig_openat64)(int dirfd, const char *pathname, int flags, ...) = nullptr;
static int (*orig_access)(const char *pathname, int mode) = nullptr;
static int (*orig_faccessat)(int dirfd, const char *pathname, int mode, int flags) = nullptr;
static int (*orig_stat)(const char *pathname, struct stat *statbuf) = nullptr;
static int (*orig_fstatat)(int dirfd, const char *pathname, struct stat *statbuf, int flags) = nullptr;
static int (*orig_lstat)(const char *pathname, struct stat *statbuf) = nullptr;
static int (*orig_unlink)(const char *pathname) = nullptr;
static int (*orig_mkdir)(const char *pathname, mode_t mode) = nullptr;
static int (*orig_rmdir)(const char *pathname) = nullptr;
static int (*orig_rename)(const char *oldpath, const char *newpath) = nullptr;

#define REDIRECT_PATH(pathname) \
    const char *redirected = IO::redirectPath(pathname); \
    if (redirected != pathname) { \
        pathname = redirected; \
    }

int new_open(const char *pathname, int flags, ...) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
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
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    mode_t mode = 0;
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    return (orig_open64 != nullptr) ? orig_open64(pathname, flags, mode) : -1;
}

int new_openat(int dirfd, const char *pathname, int flags, ...) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    mode_t mode = 0;
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    return (orig_openat != nullptr) ? orig_openat(dirfd, pathname, flags, mode) : -1;
}

int new_openat64(int dirfd, const char *pathname, int flags, ...) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    mode_t mode = 0;
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    return (orig_openat64 != nullptr) ? orig_openat64(dirfd, pathname, flags, mode) : -1;
}

int new_access(const char *pathname, int mode) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    return (orig_access != nullptr) ? orig_access(pathname, mode) : -1;
}

int new_faccessat(int dirfd, const char *pathname, int mode, int flags) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    return (orig_faccessat != nullptr) ? orig_faccessat(dirfd, pathname, mode, flags) : -1;
}

int new_stat(const char *pathname, struct stat *statbuf) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    return (orig_stat != nullptr) ? orig_stat(pathname, statbuf) : -1;
}

int new_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    return (orig_fstatat != nullptr) ? orig_fstatat(dirfd, pathname, statbuf, flags) : -1;
}

int new_lstat(const char *pathname, struct stat *statbuf) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    return (orig_lstat != nullptr) ? orig_lstat(pathname, statbuf) : -1;
}

int new_unlink(const char *pathname) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    return (orig_unlink != nullptr) ? orig_unlink(pathname) : -1;
}

int new_mkdir(const char *pathname, mode_t mode) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    return (orig_mkdir != nullptr) ? orig_mkdir(pathname, mode) : -1;
}

int new_rmdir(const char *pathname) {
    if (pathname == nullptr) return -1;
    REDIRECT_PATH(pathname);
    return (orig_rmdir != nullptr) ? orig_rmdir(pathname) : -1;
}

int new_rename(const char *oldpath, const char *newpath) {
    if (oldpath == nullptr || newpath == nullptr) return -1;
    const char *red_old = IO::redirectPath(oldpath);
    const char *red_new = IO::redirectPath(newpath);
    return (orig_rename != nullptr) ? orig_rename(red_old, red_new) : -1;
}

void FileSystemHook::init() {
    ALOGD("FileSystemHook: Initializing comprehensive file system hooks");

    shadowhook_hook_sym_name("libc.so", "open", (void *)new_open, (void **)&orig_open);
    shadowhook_hook_sym_name("libc.so", "open64", (void *)new_open64, (void **)&orig_open64);
    shadowhook_hook_sym_name("libc.so", "openat", (void *)new_openat, (void **)&orig_openat);
    shadowhook_hook_sym_name("libc.so", "openat64", (void *)new_openat64, (void **)&orig_openat64);
    shadowhook_hook_sym_name("libc.so", "access", (void *)new_access, (void **)&orig_access);
    shadowhook_hook_sym_name("libc.so", "faccessat", (void *)new_faccessat, (void **)&orig_faccessat);
    shadowhook_hook_sym_name("libc.so", "__stat64", (void *)new_stat, (void **)&orig_stat);
    shadowhook_hook_sym_name("libc.so", "fstatat64", (void *)new_fstatat, (void **)&orig_fstatat);
    shadowhook_hook_sym_name("libc.so", "__lstat64", (void *)new_lstat, (void **)&orig_lstat);
    shadowhook_hook_sym_name("libc.so", "unlink", (void *)new_unlink, (void **)&orig_unlink);
    shadowhook_hook_sym_name("libc.so", "mkdir", (void *)new_mkdir, (void **)&orig_mkdir);
    shadowhook_hook_sym_name("libc.so", "rmdir", (void *)new_rmdir, (void **)&orig_rmdir);
    shadowhook_hook_sym_name("libc.so", "rename", (void *)new_rename, (void **)&orig_rename);
}
