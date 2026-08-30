
/** @file FS.h
 *  @brief In-memory filesystem used by native tests.
 */

#pragma once

#include "Arduino.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fs {

/**
 * @brief Backing storage for a single mocked file.
 */
struct MockFileData {
    std::vector<uint8_t> bytes;
    std::string path;
    bool isDir = false;
};

/**
 * @brief Mocked Arduino File handle, backed by an in-memory byte buffer.
 */
class File : public Stream {
    std::shared_ptr<MockFileData> _data;
    size_t _pos = 0;
    bool _writable = false;

  public:
    /** @brief Creates an invalid mock file handle.
     *  @param None.
     *  @return None.
     */
    File() : _data(nullptr) {}
    /** @brief Creates a mock file handle.
     *  @param data Backing file data.
     *  @param writable Whether writes are allowed.
     *  @return None.
     */
    File(std::shared_ptr<MockFileData> data, bool writable)
        : _data(data), _writable(writable) {
        _pos = writable ? 0 : 0;
    }

    /** @brief Reports whether the file handle is valid.
     *  @param None.
     *  @return true when valid; otherwise false.
     */
    operator bool() const { return _data != nullptr; }

    /** @brief Returns unread file bytes.
     *  @param None.
     *  @return Available bytes.
     */
    int available() override {
        if (!_data)
            return 0;
        return (int)(_data->bytes.size() - _pos);
    }

    /** @brief Reads one file byte.
     *  @param None.
     *  @return Byte, or -1 at end of file.
     */
    int read() override {
        if (!_data || _pos >= _data->bytes.size())
            return -1;
        return _data->bytes[_pos++];
    }

    /** @brief Peeks at the next file byte.
     *  @param None.
     *  @return Byte, or -1 at end of file.
     */
    int peek() override {
        if (!_data || _pos >= _data->bytes.size())
            return -1;
        return _data->bytes[_pos];
    }

    /** @brief Reads file bytes into a buffer.
     *  @param buf Destination buffer.
     *  @param len Maximum bytes to read.
     *  @return Number of bytes read.
     */
    size_t read(uint8_t *buf, size_t len) {
        if (!_data)
            return 0;
        size_t avail = _data->bytes.size() - _pos;
        size_t n = len < avail ? len : avail;
        if (n > 0)
            memcpy(buf, &_data->bytes[_pos], n);
        _pos += n;
        return n;
    }

    /** @brief Writes one file byte.
     *  @param b Byte to write.
     *  @return Number of bytes written.
     */
    size_t write(uint8_t b) override {
        if (!_data || !_writable)
            return 0;
        if (_pos < _data->bytes.size()) {
            _data->bytes[_pos] = b;
        } else {
            _data->bytes.push_back(b);
        }
        _pos++;
        return 1;
    }

    /** @brief Writes bytes to a file.
     *  @param buf Source bytes.
     *  @param len Number of bytes.
     *  @return Number of bytes written.
     */
    size_t write(const uint8_t *buf, size_t len) override {
        if (!_data || !_writable)
            return 0;
        for (size_t i = 0; i < len; i++)
            write(buf[i]);
        return len;
    }

    /** @brief Returns file size.
     *  @param None.
     *  @return File size in bytes.
     */
    size_t size() const { return _data ? _data->bytes.size() : 0; }

    /** @brief Reports whether this handle is a directory.
     *  @param None.
     *  @return true for directories; otherwise false.
     */
    bool isDirectory() const { return _data && _data->isDir; }
    /** @brief Returns the file path.
     *  @param None.
     *  @return Null-terminated path.
     */
    const char *name() const { return _data ? _data->path.c_str() : ""; }
    /** @brief Returns the next directory entry.
     *  @param None.
     *  @return Invalid handle because iteration is not modeled.
     */
    File openNextFile() { return File(); } // dir iteration not modeled natively

    /** @brief Closes the file handle.
     *  @param None.
     *  @return None.
     */
    void close() { _data.reset(); }
};

/**
 * @brief Mocked fs::FS filesystem, backed by an in-memory path->bytes map.
 */
class FS {
  protected:
    std::map<std::string, std::shared_ptr<MockFileData>> _files;

  public:
    /**
     * @brief Destroys the mock filesystem.
     * @param None.
     * @return None.
     */
    virtual ~FS() = default;

    /**
     * @brief Opens a mock file.
     * @param path File path.
     * @param mode Open mode.
     * @param create Whether to create a missing file.
     * @return Open file handle.
     */
    virtual File open(const char *path, const char *mode = "r",
                      bool create = false) {
        std::string p(path);
        bool wantWrite = mode && strcmp(mode, "w") == 0;
        auto it = _files.find(p);
        if (it == _files.end()) {
            if (wantWrite || create) {
                auto data = std::make_shared<MockFileData>();
                data->path = p;
                _files[p] = data;
                return File(data, true);
            }
            return File();
        }
        if (wantWrite) {
            it->second->bytes.clear();
            return File(it->second, true);
        }
        return File(it->second, false);
    }

    /**
     * @brief Checks whether a mock path exists.
     * @param path File path.
     * @return true when present; otherwise false.
     */
    virtual bool exists(const char *path) {
        return _files.find(path) != _files.end();
    }

    /**
     * @brief Removes a mock file.
     * @param path File path.
     * @return true when removed; otherwise false.
     */
    virtual bool remove(const char *path) { return _files.erase(path) > 0; }

    /**
     * @brief Creates a mock directory.
     * @param path Directory path.
     * @return true when successful.
     */
    virtual bool mkdir(const char * /*path*/) { return true; }
    /**
     * @brief Removes a mock directory.
     * @param path Directory path.
     * @return true when successful.
     */
    virtual bool rmdir(const char * /*path*/) { return true; }

    /**
     * @brief Formats the mock filesystem.
     * @param None.
     * @return true when successful.
     */
    virtual bool format() {
        _clear();
        return true;
    }

    /**
     * @brief Totals bytes stored in mock files.
     * @param None.
     * @return Stored byte count.
     */
    size_t _totalDataBytes() const {
        size_t total = 0;
        for (auto &kv : _files)
            total += kv.second->bytes.size();
        return total;
    }

    /**
     * @brief Clears all mock files.
     * @param None.
     * @return None.
     */
    void _clear() { _files.clear(); }
};

/**
 * @brief Mocked SPIFFSFS, matching the ESP32 Arduino core's `fs::SPIFFSFS`.
 */
class SPIFFSFS : public FS {
  public:
    /**
     * @brief Starts the mock SPIFFS filesystem.
     * @param formatOnFail Whether to format after failure.
     * @param basePath Filesystem base path.
     * @param maxOpenFiles Maximum open files.
     * @param partitionLabel Optional partition label.
     * @return true when started successfully.
     */
    bool begin(bool /*formatOnFail*/ = false,
               const char * /*basePath*/ = "/spiffs",
               uint8_t /*maxOpenFiles*/ = 10,
               const char * /*partitionLabel*/ = nullptr) {
        return true;
    }
    /**
     * @brief Formats the mock SPIFFS filesystem.
     * @param None.
     * @return true when successful.
     */
    bool format() {
        _clear();
        return true;
    }
    /**
     * @brief Returns mock used capacity.
     * @param None.
     * @return Used bytes.
     */
    size_t usedBytes() { return _totalDataBytes(); }
    /**
     * @brief Returns mock total capacity.
     * @param None.
     * @return Total bytes.
     */
    size_t totalBytes() { return 4 * 1024 * 1024; }
};

} // namespace fs

typedef fs::File File;
