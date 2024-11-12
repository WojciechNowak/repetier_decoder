#pragma once

#include <cstdint>
#include <algorithm>
#include <string>

using millis_t = int;
#define GCODE_BUFFER_SIZE 2
#define PSTR(x) x
#define FSTRINGVAR(x) static const std::string x;
#define FSTRINGPARAM(x) const std::string& x
#define FSTRINGVALUE(var, value) const std::string var = value;
