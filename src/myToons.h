#pragma once
#include <vector>
#include <string>
#include <math.h>
#include <cassert>

#ifndef REGISTER_AOVS_CUSTOM
#define REGISTER_AOVS_CUSTOM \
data->aovs_custom.clear(); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_mytoons_beauty")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_color_major")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_color_shadow")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_color_mask")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_outline")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_dynamic_shadow")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_dynamic_shadow_raw")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_normal")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_fresnel")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_depth")); \
data->aovs_custom.push_back(AiNodeGetStr(node, "my_aov_occlusion")); \
assert(data->aovs_custom.size() == 11 && "NUM_AOVs does not match size of aovs array!"); \
 for(size_t i = 0; i < data->aovs_custom.size(); i++) \
	 AiAOVRegister(data->aovs_custom[i].c_str(), AI_TYPE_RGB, AI_AOV_BLEND_OPACITY);
#endif

enum AovIndicesToons{
	k_my_aov_mytoons_beauty = 0,
	k_my_aov_color_major,
	k_my_aov_color_shadow,
	k_my_aov_color_mask,
	k_my_aov_outline,
	k_my_aov_dynamic_shadow,
	k_my_aov_dynamic_shadow_raw,
	k_my_aov_normal,
	k_my_aov_fresnel,
	k_my_aov_depth,
	k_my_aov_occlusion,
};	

struct ShaderDataToons{
	bool hasChainedNormal;
	// AOV names
	std::vector<AtString> aovs;
	std::vector<AtString> aovs_rgba;
	std::vector<AtString> aovs_custom;
};

#define LUT_SIZE 32

namespace my{
	bool getMayaRampArrays(AtNode* node, const char* paramName, AtArray** positions, AtArray** colors, RampInterpolationType* interp)
	{
		bool result = false;
		*positions = NULL;
		*colors = NULL;
		if(AiNodeIsLinked(node, paramName)){
			AtNode* cn = AiNodeGetLink(node, paramName);
			const AtNodeEntry* cne = AiNodeGetNodeEntry(cn);
			if(!strcmp(AiNodeEntryGetName(cne), "MayaRamp")){
				*positions = AiNodeGetArray(cn, "position");
				*colors = AiNodeGetArray(cn, "color");
				*interp = (RampInterpolationType)AiNodeGetInt(cn, "interpolation");
				result = true;
			}
			else
				AiMsgWarning("[syToons] %s is connected but connection is not a MayaRamp", paramName);
		}
		return result;
	}
}
