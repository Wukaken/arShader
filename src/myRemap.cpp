#include <ai.h>
#include <iostream>
#include "al_util.h"

AI_SHADER_NODE_EXPORT_METHODS(myRemapMethods);

enum Params{
	p_input,
	p_gamma,
	p_saturation,
	p_hueOffset,
	p_contrast,
	p_contrastPivot,
	p_gain,
	p_exposure,
	p_mask,
};

node_parameters
{
	AiParameterRGB("input", 0.18f, 0.18f, 0.18f);
	AiParameterFlt("gamma", 1.0f);
	AiParameterFlt("saturation", 1.0f);
	AiParameterFlt("hueOffset", 0.0f);
	AiParameterFlt("contrast", 1.0f);
	AiParameterFlt("contrastPivot", 0.18f);
	AiParameterFlt("gain", 1.0f);
	AiParameterFlt("exposure", 0.0f);
	AiParameterFlt("mask", 1.0f);
}

node_initialize {}
node_finish {}
node_update {}

shader_evaluate
{
	AtRGB input = AiShaderEvalParamRGB(p_input);
	float gamma = AiShaderEvalParamFlt(p_gamma);
	float saturation = AiShaderEvalParamFlt(p_saturation);
	float hueOffset = AiShaderEvalParamFlt(p_hueOffset);
	float contrastVal = AiShaderEvalParamFlt(p_contrast);
	float contrastPivot = AiShaderEvalParamFlt(p_contrastPivot);
	float gain = AiShaderEvalParamFlt(p_gain);
	float exposure = AiShaderEvalParamFlt(p_exposure);
	float mask = AiShaderEvalParamFlt(p_mask);

	AtRGB result = input;
	if(mask > 0.0f){
		// gamma
		result = pow(input, 1.0f / gamma);

		// saturation
		if(saturation != 1.0f){
			float l = luminance(result);   // do check
			result = lerp(rgb(l), result, saturation);  // do check
		}

		if(hueOffset != 0.0f){
			AtRGB hsv = rgb2hsv(result);
			hsv.r += hueOffset;
			result = hsv2rgb(hsv);
		}

		if(contrastVal != 1.0f)
			result = contrast(result, contrastVal, contrastPivot);
			
		result = result * powf(2.0f, exposure) * gain;

		if(mask < 1.0f)
			result = lerp(input, result, mask);
	}
	sg->out.RGB() = result;
}
