#ifndef PRESETS_H
#define PRESETS_H

#include <string>
#include <vector>
#include <functional>

// Forward declarations
void init_presets();
const std::vector<std::string>& get_preset_names();
void apply_preset(int index);

#endif // PRESETS_H

