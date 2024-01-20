#include <FileUtil.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include "../../logger/include/log.h"

bool FileUtil::exists(std::string const& path) {
    return access(path.c_str(), F_OK) == 0;
}

bool FileUtil::isDirectory(std::string const& path) {
    struct stat s;
    return !stat(path.c_str(), &s) && S_ISDIR(s.st_mode);
}

std::string FileUtil::getParent(std::string const& path) {
    auto iof = path.rfind('/');
    if (iof == std::string::npos)
        return std::string();
    while (iof > 0 && path[iof - 1] == '/')
        iof--;
    return path.substr(0, iof);
}

void FileUtil::mkdirRecursive(std::string const& path) {
    if (isDirectory(path))
        return;
    if (exists(path))
        throw std::runtime_error(std::string("File exists and is not a directory: ") + path);
    mkdirRecursive(getParent(path));
    if (mkdir(path.c_str(), 0744) != 0)
        throw std::runtime_error(std::string("mkdir failed, path = ") + path);
}

bool FileUtil::readFile(std::string const &path, std::string &out) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
#ifndef NDEBUG
        Log::error("FileUtil", "readFile: not found: %s\n", path.c_str());
#endif
        return false;
    }
    struct stat sr;
    if (fstat(fd, &sr) < 0 || (sr.st_mode & S_IFDIR)) {
        close(fd);
#ifndef NDEBUG
        Log::error("FileUtil", "readFile: opening a directory: %s\n", path.c_str());
#endif
        return false;
    }
    auto size = lseek(fd, 0, SEEK_END);
    if (size == (off_t) -1) {
        Log::error("FileUtil", "readFile: lseek error\n");
        close(fd);
        return false;
    }
    out.resize((size_t) size);
    lseek(fd, 0, SEEK_SET);
    for (size_t o = 0; o < (size_t) size; ) {
        int res = read(fd, &out[o], size - o);
        if (res < 0) {
            Log::error("FileUtil", "readFile: read error\n");
            close(fd);
            return false;
        }
        o += res;
    }
    close(fd);
    return true;
}