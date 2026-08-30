
/** @file Stream.h
 *  @brief Minimal Arduino stream abstraction used by native tests.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

/**
 * @brief Mocked Arduino Print base class.
 */
class Print {
  public:
    /** @brief Writes one byte.
     *  @param b Byte to write.
     *  @return Number of bytes written.
     */
    virtual size_t write(uint8_t /*b*/) { return 1; }
    /** @brief Writes a null-terminated string.
     *  @param str String to write.
     *  @return Number of bytes written.
     */
    size_t write(const char *str) {
        if (str == nullptr)
            return 0;
        return write((const uint8_t *)str, strlen(str));
    }
    /** @brief Writes a byte buffer.
     *  @param buffer Bytes to write.
     *  @param size Number of bytes.
     *  @return Number of bytes written.
     */
    virtual size_t write(const uint8_t *buffer, size_t size) {
        size_t t = 0;
        for (size_t i = 0; i < size; i++)
            t += write(buffer[i]);
        return t;
    }
    /** @brief Writes a character buffer.
     *  @param buffer Characters to write.
     *  @param size Number of bytes.
     *  @return Number of bytes written.
     */
    size_t write(const char *buffer, size_t size) {
        return write((const uint8_t *)buffer, size);
    }

    /** @brief Prints an unsigned byte.
     *  @param b Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t print(unsigned char b, int r = DEC) { return printUnsigned(b, r); }
    /** @brief Prints a signed integer.
     *  @param v Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t print(int v, int r = DEC) { return printSigned(v, r); }
    /** @brief Prints an unsigned integer.
     *  @param v Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t print(unsigned int v, int r = DEC) { return printUnsigned(v, r); }
    /** @brief Prints a long integer.
     *  @param v Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t print(long v, int r = DEC) { return printSigned(v, r); }
    /** @brief Prints an unsigned long integer.
     *  @param v Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t print(unsigned long v, int r = DEC) { return printUnsigned(v, r); }
    /** @brief Prints a long-long integer.
     *  @param v Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t print(long long v, int r = DEC) { return printSigned(v, r); }
    /** @brief Prints an unsigned long-long integer.
     *  @param v Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t print(unsigned long long v, int r = DEC) {
        return printUnsigned(v, r);
    }
    /** @brief Prints a floating-point value.
     *  @param v Value to print.
     *  @param p Decimal precision.
     *  @return Number of bytes written.
     */
    size_t print(double v, int p = 2) {
        char text[32];
        snprintf(text, sizeof(text), "%.*f", p, v);
        return write(text);
    }
    /** @brief Prints one character.
     *  @param c Character to print.
     *  @return Number of bytes written.
     */
    size_t print(char c) { return write((uint8_t)c); }
    /** @brief Prints a string.
     *  @param str String to print.
     *  @return Number of bytes written.
     */
    size_t print(const char *str) { return write(str); }

    /** @brief Prints a string followed by a newline.
     *  @param str String to print.
     *  @return Number of bytes written.
     */
    size_t println(const char *str) {
        size_t n = write(str);
        n += write("\n");
        return n;
    }
    /** @brief Prints a newline.
     *  @param None.
     *  @return Number of bytes written.
     */
    size_t println() { return write("\n"); }

    /** @brief Writes a formatted string to the mock sink.
     *  @param fmt Format string.
     *  @param ... Format arguments.
     *  @return Number of bytes written.
     */
    size_t printf(const char * /*fmt*/, ...) { return 0; }

    /** @brief Flushes the mock output.
     *  @param None.
     *  @return None.
     */
    virtual void flush() {}

  private:
    /** @brief Prints a signed value using the selected base.
     *  @param value Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t printSigned(long long value, int r) {
        char text[24];
        if (r == HEX) {
            snprintf(text, sizeof(text), "%llx", (unsigned long long)value);
        } else {
            snprintf(text, sizeof(text), "%lld", value);
        }
        return write(text);
    }
    /** @brief Prints an unsigned value using the selected base.
     *  @param value Value to print.
     *  @param r Numeric base.
     *  @return Number of bytes written.
     */
    size_t printUnsigned(unsigned long long value, int r) {
        char text[24];
        if (r == HEX) {
            snprintf(text, sizeof(text), "%llx", value);
        } else {
            snprintf(text, sizeof(text), "%llu", value);
        }
        return write(text);
    }
};

/**
 * @brief Mocked Arduino Stream base class.
 */
class Stream : public Print {
  public:
    /** @brief Destroys the mock stream.
     *  @param None.
     *  @return None.
     */
    virtual ~Stream() = default;
    /** @brief Returns readable bytes available.
     *  @param None.
     *  @return Available byte count.
     */
    virtual int available() { return 0; }
    /** @brief Returns writable space available.
     *  @param None.
     *  @return Available write capacity.
     */
    virtual int availableForWrite() { return 0; }
    /** @brief Reads one byte.
     *  @param None.
     *  @return Byte, or -1 at end of input.
     */
    virtual int read() { return -1; }
    /** @brief Peeks at the next byte.
     *  @param None.
     *  @return Byte, or -1 at end of input.
     */
    virtual int peek() { return 0; }

    /** @brief Reads characters into a buffer.
     *  @param buffer Destination buffer.
     *  @param length Maximum bytes to read.
     *  @return Number of bytes read.
     */
    virtual size_t readBytes(char *buffer, size_t length) {
        size_t i = 0;
        while (i < length && available()) {
            buffer[i++] = read();
        }
        return i;
    }
    /** @brief Reads bytes into a byte buffer.
     *  @param buffer Destination buffer.
     *  @param length Maximum bytes to read.
     *  @return Number of bytes read.
     */
    size_t readBytes(uint8_t *buffer, size_t length) {
        return readBytes((char *)buffer, length);
    }
};
