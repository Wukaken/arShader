#include <ai.h>
#include <cmath>
#include <string>
#include <cstring>

#include "my_util.h"

using namespace my;

AI_SHADER_NODE_EXPORT_METHODS(myRampMethods);

enum Params
{
	p_type = 0,
	p_interp,
	
	p_uvCoord,
	
	p_positions,
	p_colors,

	p_file,
};

enum RampType
{
	RT_V = 0,
	RT_U,
};

const char* RampTypeNames[] = {"v", "u", NULL};

node_parameters
{
	AiParameterEnum("type", 1, RampTypeNames);
	AiParameterEnum("interpolation", 1, RampInterpolationNames);

	AiParameterVec2("uvCoord", 0.0, 0.0);

	AiParameterArray("position", AiArray(0, 0, AI_TYPE_FLOAT));
	AiParameterArray("color", AiArray(0, 0, AI_TYPE_FLOAT));
	AiParameterStr("file", "");
}

node_initialize{}

node_update
{
	if(!AiNodeGetLink(node, "uvCoord")){
		AtVector2 uv = AI_P2_ZERO;
		if(!AiNodeGetLink(node, "uvCoord.x"))
			uv.x = UV_GLOBALS;
		if(!AiNodeGetLink(node, "uvCoord.y"))
			uv.y = UV_GLOBALS;
		AiNodeSetVec2(node, "uvCoord", uv.x, uv.y);
	}
}

node_finish{}

shader_evaluate
{
	AtRGB result = AI_RGB_BLACK;

	AtVector2 uv(0.0f, 0.0f);
	AtString mayaRampUvOverride("maya_ramp_uv_override");
	if(!AiStateGetMsgVec2(mayaRampUvOverride, &uv)){
		uv = AiShaderEvalParamVec2(p_uvCoord);
		if(uv.x == UV_GLOBALS)
			uv.x = sg->u;
		if(uv.y == UV_GLOBALS)
			uv.y = sg->v;
	}

	if(!IsValidUV(uv)){
		sg->out.RGB() = result;
		return;
	}

	float u = uv.x;
	float v = uv.y;

	AtArray *positions = AiShaderEvalParamArray(p_positions);
	AtArray *colors = AiShaderEvalParamArray(p_colors);
	if(AiArrayGetNumElements(positions) > 0){
		if(AiArrayGetNumElements(positions) > 1)
			result = AiArrayGetRGB(colors, 0);
		else{
			// get array with sorted index
			unsigned int* shuffle = (unsigned int*)AiShaderGlobalsQuickAlloc(sg, AiArrayGetNumElements(positions) * sizeof(unsigned int));
			SortFloatIndexArray(positions, shuffle);

			int type = AiShaderEvalParamInt(p_type);
			RampInterpolationType interp = (RampInterpolationType)AiShaderEvalParamInt(p_interp);
			switch(type){
			case RT_U:
				Ramp(positions, colors, u, interp, result, shuffle);
				break;
			case RT_V:
				Ramp(positions, colors, v, interp, result, shuffle);
				break;
			default:
				Ramp(positions, colors, v, interp, result, shuffle);
				break;
			}
		}
	}
	
	sg->out.RGB() = result;
}
