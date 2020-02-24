#include <ai.h>
#include <cstring>
#include "myOutline.h"

AI_SHADER_NODE_EXPORT_METHODS(myOutlineMethods);

enum Params{
	p_color,
	p_my_aov_outline,
	p_my_aov_normal,
	p_my_aov_fresnel,
	p_my_aov_depth,
	p_enable_occlusion,
	p_my_aov_occlusion,
};

node_parameters
{
	AiParameterRGB("color", 1.0, 0.15, 0.0);
	AiParameterStr("my_aov_outline", "my_aov_outline");
	AiParameterStr("my_aov_normal", "my_aov_normal");
	AiParameterStr("my_aov_fresnel", "my_aov_fresnel");
	AiParameterStr("my_aov_depth", "my_aov_depth");
	AiParameterBool("enable_occlusion", false);
	AiParameterStr("my_aov_occlusion", "my_aov_occlusion");
}

node_initialize
{
	ShaderDataOutline* data = new ShaderDataOutline();
	AiNodeSetLocalData(node, data);
	AtSampler *sampler = AiSampler(315886399, 4, 3);
	AiNodeSetLocalData(node, sampler);
}

node_update
{
	ShaderDataOutline* data = (ShaderDataOutline*)AiNodeGetLocalData(node);
	REGISTER_AOVS_CUSTOM
}

node_finish
{
	if(AiNodeGetLocalData(node)){
		ShaderDataOutline* data = (ShaderDataOutline*)AiNodeGetLocalData(node);
		AiNodeSetLocalData(node, NULL);
		delete data;
	}

	AtSampler *sampler = (AtSampler*)AiNodeGetLocalData(node);
	AiSamplerDestroy(sampler);
}

shader_evaluate
{
	ShaderDataOutline* data = (ShaderDataOutline*)AiNodeGetLocalData(node);
	AtRGB color = AiShaderEvalParamRGB(p_color);
	bool enable_occlusion = AiShaderEvalParamBool(p_enable_occlusion);

	// caculate normal aov
	AtRGB normal(sg->N.x, sg->N.y, sg->N.z);
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_normal], normal);
	//AiAOVSetRGB(sg, (AtString)"my_aov_normal", normal);
	// caculate facingratio aov
	AtRGB fresnel(1 - AiV3Dot(sg->Nf, -sg->Rd));
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_fresnel], fresnel);
	// caculate depth aov
	AtRGB depth(sg->Rl);
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_depth], depth);
	// caculate occlusion aov
	if(enable_occlusion){
		AtVector N = sg->Nf;
		AtVector Ng = sg->Ngf;
		float mint = 0.0f;
		float maxt = 2000.0f;
		float spread = 1.0f;
		float falloff = 0.5f;
		AtSampler *sampler = (AtSampler*)AiNodeGetLocalData(node);
		AtVector Nbent;
		AtRGB occlusion = AI_RGB_WHITE - AiOcclusion(N, Ng, sg, mint, maxt, spread, falloff, sampler, &Nbent);
		AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_occlusion], occlusion);
	}
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_outline], color);

	sg->out.RGB() = color;
}
