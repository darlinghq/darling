#ifndef COMMON_ATTRIBUTE_H
#define COMMON_ATTRIBUTE_H
#include <objc/runtime.h>
#include <stdint.h>
#include <string>
#include "./AppleLayout.h"
#include <vector>

bool nextAttribute(objc_property_attribute_t& next, const char*& pos, std::vector<std::string>& strings);

#endif
