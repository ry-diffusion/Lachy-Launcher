#include <cll/file_event_batch.h>
#include <unistd.h>
#include <log.h>
#include <fcntl.h>

#ifdef __APPLE__
#define ftruncate64 ftruncate
#define lseek64 lseek
#endif

using namespace cll;

FileEventBatch::FileEventBatch(std::string const& path) : path(path) {
    fd = open(path.c_str(), O_RDWR | O_CREAT, 0600);
    if (fd < 0)
        Log::warn("FileEventBatch", "Error occurred trying to open the specified file");
    seekToEndAndGetFileSize();
    countEventsInFile();
}

FileEventBatch::~FileEventBatch() {
    if (fd >= 0)
        close(fd);
}

void FileEventBatch::seekToEndAndGetFileSize() {
    off_t tg = lseek(fd, 0, SEEK_END);
    streamAtEnd = true;
    if (tg < 0)
        tg = 0;
    fileSize = (size_t) tg;
}

void FileEventBatch::countEventsInFile() {
    eventCount = 0;
    if (fd < 0)
        return;
    lseek(fd, 0, SEEK_SET);
    streamAtEnd = false;
    char buf[16 * 1024];
    size_t lastNewline = 0;
    ssize_t n;
    size_t o = 0;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        char* ptr = buf;
        while ((ptr = (char*) memchr(ptr, '\n', n - (ptr - buf))) != nullptr) {
            ptr++;
            lastNewline = o + (ptr - buf);
            eventCount++;
        }
        o += n;
    }
    if (lastNewline != fileSize) {
        Log::warn("FileEventBatch", "Invalid event at the end of the file; trimming the file from %li to %lu",
                  fileSize, lastNewline);
        ftruncate64(fd, lastNewline);
        fileSize = lastNewline;
    }
}

bool FileEventBatch::canAddEvent(size_t eventSize) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (finalized)
        return false;
    return canAddEventInt(eventSize);
}

bool FileEventBatch::addEvent(std::string const& data) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (fd < 0)
        return false;
    if (finalized) {
        Log::warn("FileEventBatch", "Trying to add an event to a finalized EventBatch");
        return false;
    }
    if (!canAddEventInt(data.length()))
        return false;
    if (!streamAtEnd)
        seekToEndAndGetFileSize();
    size_t o = 0;
    while (o < data.size()) {
        ssize_t ret = write(fd, &data[o], data.size() - o);
        if (ret < 0) {
            Log::warn("FileEventBatch", "Failed to write an event due to an IO error");
            if (o > 0) {
                // We got into a corrupted state pretty much. Try to remove the event using truncate.
                ftruncate64(fd, fileSize);
            }
            return false;
        }
        o += ret;
    }
    fileSize += o; // Let's assume the event got written correctly
    eventCount++;
    return true;
}

bool FileEventBatch::addEvent(nlohmann::json const& rawData) {
    return addEvent(rawData.dump() + "\r\n");
}

std::unique_ptr<BatchedEventList> FileEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (fd < 0 || fileSize == 0 || maxCount == 0)
        return nullptr;
    lseek(fd, 0, SEEK_SET);
    streamAtEnd = false;
    if (maxSize > 1024 * 1024)
        maxSize = 1024 * 1024;
    if (maxSize > fileSize)
        maxSize = fileSize;
    std::vector<char> data;
    data.resize(maxSize);
    // read as much as we can
    size_t n = 0;
    size_t events = 0;
    ssize_t m;
    while ((m = read(fd, &data.data()[n], maxSize - n)) > 0)
        n += m;
    auto data_ptr = data.data();
    char* end_data_ptr = data.data() + n;
    while (true) {
        char* n_data_ptr = (char*) memchr(data_ptr, '\n', end_data_ptr - data_ptr);
        if (n_data_ptr == nullptr)
            break;
        n_data_ptr++;

        data_ptr = n_data_ptr;
        if (++events == maxCount)
            break;
    }
    n = data_ptr - data.data();
    data.resize(n);
    return std::unique_ptr<BatchedEventList>(new VectorBatchedEventList(std::move(data), events, n < fileSize));
}

void FileEventBatch::onEventsUploaded(BatchedEventList& events) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (fd < 0)
        return;
    if (fileSize == events.getDataSize()) {
        fileSize = 0;
        eventCount = 0;
        if (finalized) {
            close(fd);
            fd = -1;
            remove(path.c_str());
        } else {
            ftruncate(fd, 0);
        }
        return;
    }
    int fdIn = open(path.c_str(), O_RDONLY);
    if (fdIn < 0)
        throw std::runtime_error("Failed to open input stream for onEventsUploaded");
    lseek64(fdIn, events.getDataSize(), SEEK_SET);
    lseek64(fd, 0, SEEK_SET);
    const int bufSize = 1024 * 1024;
    char* buf = new char[bufSize];
    size_t n = 0;
    ssize_t m;
    while ((m = read(fdIn, buf, bufSize)) > 0) {
        for (size_t o = 0; o < m; ) {
            ssize_t m2 = write(fd, &buf[o], m - o);
            if (m2 < 0) {
                Log::warn("FileEventBatch", "onEventsUploaded: failed to write data from buffer");
                break;
            }
            o += m2;
            n += m2;
        }
    }
    delete[] buf;
    close(fdIn);
    ftruncate64(fd, n);
    fileSize = n;
    eventCount -= ((VectorBatchedEventList&) events).getEventCount();
}
