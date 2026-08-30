
/** @file mesh_platform_config.h
 *  @brief Native-test filesystem platform shim.
 */

#pragma once

#include "FS.h"

#ifndef FILESYSTEM
#define FILESYSTEM fs::FS
#endif
