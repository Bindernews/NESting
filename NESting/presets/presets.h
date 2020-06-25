#pragma once
#include <stdint.h>

struct rp_preset_t {
  rp_preset_t(const char* name, const uint8_t* data, uint32_t size) : name(name), data(data), size(size) {}
  const char* name;
  const uint8_t* data;
  const uint32_t size;
};
extern const uint8_t PRESET_Default[1343];
extern const uint8_t PRESET_Creepy_Square[1349];
extern const uint8_t PRESET_Running[1343];
extern const rp_preset_t PRESET_LIST[3];
#define PRESET_LIST_SIZE (3)

