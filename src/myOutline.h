#pragma once
#include <vector>
#include <string>
#include <math.h>
#include <cassert>

#ifndef REGISTER_AOVS_CUSTOM
#define REGISTER_AOVS_CUSTOM \
data->aovs_custom.clear(); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_outline")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_normal")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_fresnel")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_depth")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_occlusion")); \
assert(data->aovs_custom.size() == 5 && "NUM_AOVs does not match size of aovs array!"); \
 for(size_t i = 0; i < data->aovs_custom.size(); i++) \
	 AiAOVRegister(data->aovs_custom[i].c_str(), AI_TYPE_RGB, AI_AOV_BLEND_OPACITY);
#endif

enum AovIndicesOutline{
	k_my_aov_outline = 0,
	k_my_aov_normal,
	k_my_aov_fresnel,
	k_my_aov_depth,
	k_my_aov_occlusion,
};

struct ShaderDataOutline{
	// AOV names
	std::vector<AtString> aovs;
	std::vector<AtString> aovs_rgba;
	std::vector<AtString> aovs_custom;
};
