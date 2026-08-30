/** @file CayenneLPPPolyline.cpp
 *  @brief Native-test translation unit for CayenneLPP.
 */

#include <cstdint>
// Stub replacing the real CayenneLPPPolyline.cpp: that vendor file has a
// Windows/MinGW portability bug (std::max(size_t, unsigned long) type
// mismatch) and polyline encode/decode isn't exercised by meshcore-bare at
// all (no GPS polyline usage in MyMesh.cpp) - only the constructor/encode/
// decode symbols need to exist to satisfy CayenneLPP.cpp's link deps.
#include "../../../.pio/libdeps/meshcore_bare/CayenneLPP/src/CayenneLPPPolyline.h"
#include <cstdint>

/**
 * @brief Creates the native-test polyline stub.
 * @param size Requested polyline capacity.
 * @return None.
 */
CayenneLPPPolyline::CayenneLPPPolyline(uint32_t /*size*/) {}

/**
 * @brief Encodes points using a scale factor in the native-test stub.
 * @param coords Points to encode.
 * @param factor Encoding scale factor.
 * @param simplification Simplification mode.
 * @return Empty encoded data from the stub.
 */
std::vector<uint8_t>
CayenneLPPPolyline::encode(const std::vector<Point> & /*coords*/,
                           uint8_t /*factor*/,
                           Simplification /*simplification*/) {
    return std::vector<uint8_t>();
}

/**
 * @brief Encodes points using precision in the native-test stub.
 * @param coords Points to encode.
 * @param precision Encoding precision.
 * @param simplification Simplification mode.
 * @return Empty encoded data from the stub.
 */
std::vector<uint8_t>
CayenneLPPPolyline::encode(const std::vector<Point> & /*coords*/,
                           Precision /*precision*/,
                           Simplification /*simplification*/) {
    return std::vector<uint8_t>();
}

/**
 * @brief Decodes polyline data in the native-test stub.
 * @param buffer Encoded polyline data.
 * @return Empty decoded points from the stub.
 */
std::vector<std::pair<double, double>>
CayenneLPPPolyline::decode(const std::vector<uint8_t> & /*buffer*/) {
    return std::vector<Point>();
}

/**
 * @brief Returns encoding statistics from the native-test stub.
 * @param None.
 * @return Default encoding statistics.
 */
CayenneLPPPolyline::Stats CayenneLPPPolyline::getEncodeStats() const {
    return Stats();
}
