#include <ai.h>
#include <cstring>

#include "al_util.h"
#include "my_util.h"
#include "myToons.h"

using namespace my;

AI_SHADER_NODE_EXPORT_METHODS(myToonsMethods);

enum EngineParams{
	S_SCANLINE = 0,
	S_RAYTRACE
};

const char* engine_params[] = {
	"Scanline",
	"Raytrace(beta)",
	NULL
};

enum Params{
	p_engine,
	p_color_major,
	p_color_shadow,
	p_color_mask,
	p_enable_outline,
	p_color_outline,
	p_lambert_color,
	p_shadow_ramp,
	p_shadow_mask,
	p_shadow_position,
	p_normal,
	p_opacity,
	p_casting_light,
	p_enable_occlusion,
	p_use_ramp_color,
	p_my_aov_mytoons_beauty,
	p_my_aov_color_major,
	p_my_aov_color_shadow,
	p_my_aov_color_mask,
	p_my_aov_outline,
	p_my_aov_dynamic_shadow,
	p_my_aov_dynamic_shadow_raw,
	p_my_aov_normal,
	p_my_aov_fresnel,
	p_my_aov_depth,
	p_my_aov_occlusion,
};

node_parameters
{
	AiParameterEnum("engine", S_SCANLINE, engine_params);
	AiParameterRGB("color_major", 1.0, 1.0, 1.0);
	AiParameterRGB("color_shadow", 0.0, 0.0, 0.0);
	AiParameterRGB("color_mask", 1.0, 1.0, 1.0);
	AiParameterBool("enable_outline", false);
	AiParameterRGB("color_outline", 0.0, 0.0, 0.0);
	AiParameterRGB("lambert_color", 1.0, 1.0, 1.0);
	AiParameterRGB("shadow_ramp", 0.15, 0.15, 0.15);
	AiParameterRGB("shadow_mask", 1.0, 1.0, 1.0);
	AiParameterFlt("shadow_position", 0.1);
	AiParameterRGB("normal", 1.0, 1.0, 1.0);
	AiParameterRGB("opacity", 1.0, 1.0, 1.0);
	AiParameterBool("casting_light", true);
	AiParameterBool("enable_occlusion", false);
	AiParameterBool("use_ramcolor", false);
		
	AiParameterStr("my_aov_mytoons_beauty", "my_aov_mytoons_beauty");
	AiParameterStr("my_aov_color_major", "my_aov_color_major");
	AiParameterStr("my_aov_color_shadow", "my_aov_color_shadow");
	AiParameterStr("my_aov_color_mask", "my_aov_color_mask");
	AiParameterStr("my_aov_outline", "my_aov_outline");
	AiParameterStr("my_aov_dynamic_shadow", "my_aov_dynamic_shadow");
	AiParameterStr("my_aov_dynamic_shadow_raw", "my_aov_dynamic_shadow_raw");
	AiParameterStr("my_aov_normal", "my_aov_normal");
	AiParameterStr("my_aov_fresnel", "my_aov_fresnel");
	AiParameterStr("my_aov_depth", "my_aov_depth");
	AiParameterStr("my_aov_occlusion", "my_aov_occlusion");
}

node_initialize
{
	ShaderDataToons* data = new ShaderDataToons();
	data->hasChainedNormal = false;
	AiNodeSetLocalData(node, data);
}

node_update
{
	ShaderDataToons* data = (ShaderDataToons*)AiNodeGetLocalData(node);
	data->hasChainedNormal = AiNodeIsLinked(node, "normal");
	REGISTER_AOVS_CUSTOM
}

node_finish
{
	if(AiNodeGetLocalData(node)){
		ShaderDataToons *data = (ShaderDataToons*)AiNodeGetLocalData(node);
		AiNodeSetLocalData(node, NULL);
		delete data;
	}
}

shader_evaluate
{
	ShaderDataToons *data = (ShaderDataToons*)AiNodeGetLocalData(node);
	int shading_engine = AiShaderEvalParamInt(p_engine);
	AtRGB color_major = AiShaderEvalParamRGB(p_color_major);
	AtRGB color_shadow = AiShaderEvalParamRGB(p_color_shadow);
	AtRGB color_mask = AiShaderEvalParamRGB(p_color_mask);
	AtRGB color_outline = AiShaderEvalParamRGB(p_color_outline);
	bool enable_outline = AiShaderEvalParamBool(p_enable_outline);
	bool casting_light = AiShaderEvalParamBool(p_casting_light);
	bool enable_occlusion = AiShaderEvalParamBool(p_enable_occlusion);
	bool use_ramp_color = AiShaderEvalParamBool(p_use_ramp_color);
	AtRGB shadow_mask = AiShaderEvalParamRGB(p_shadow_mask);

	AtRGB result = AI_RGB_BLACK;
	AtRGB result_opacity = AiShaderEvalParamRGB(p_opacity);
	AtRGB diffuse_raw = AI_RGB_BLACK;
	AtRGB texture_result = AI_RGB_BLACK;
	AtRGB lighting_result = AI_RGB_WHITE;
	AtRGB shadow_result = AI_RGB_WHITE;
	AtRGB shadow_raw_result = AI_RGB_WHITE;

	if(data->hasChainedNormal){
		sg->Nf = AiShaderEvalParamVec(p_normal);
		sg->N = AiShaderEvalParamVec(p_normal);
	}

	texture_result = lerp(color_shadow, color_major, color_mask.r);

	switch(shading_engine){
	case S_SCANLINE:
		if(casting_light && sg->Rt & AI_RAY_CAMERA){
			AtRGB Kd = AiShaderEvalParamRGB(p_lambert_color);
			AtRGB Ks = AI_RGB_WHITE;
			float Wig = 0.28;
			float roughness = 10 / 0.2;
			AiLightsPrepare(sg);
			AtRGB LaD = AI_RGB_BLACK;
			AtRGB LaS = AI_RGB_BLACK;

			AtLightSample ls;
			while(AiLightsGetSample(sg, ls)){  // loop over the lights
				float LdotN = AiV3Dot(ls.Ld, sg->Nf);
				if(LdotN < 0)
					LdotN = 0;
				
				AtVector H = AiV3Normalize(-sg->Rd + ls.Ld);
				float spec = AiV3Dot(sg->Nf, H);
				if(spec < 0)
					spec = 0;

				LaD += ls.Li * Wig * (1 / ls.pdf) * LdotN * Kd;
				LaS += ls.Li * Wig * (1 / ls.pdf) * pow(spec, roughness) * Ks;
			}

			diffuse_raw = LaD;
			lighting_result = LaD + LaS;

			float diff_t = clamp(diffuse_raw.r, 0.0, 1.0f);
			AtRGB shadow_ramp = AiShaderEvalParamRGB(p_shadow_ramp);
			float shadow_position = AiShaderEvalParamFlt(p_shadow_position);

			bool isMayaRamp = false;
			AtRGB diffuseLUT[LUT_SIZE];
			AtArray* diffusePositions = NULL;
			AtArray* diffuseColors = NULL;
			RampInterpolationType diffuseInterp;
			
			isMayaRamp = getMayaRampArrays(node, "shadow_ramp", &diffusePositions,
											   &diffuseColors, &diffuseInterp);
			if(isMayaRamp){
				unsigned int* shuffle = (unsigned int*)AiShaderGlobalsQuickAlloc(sg, sizeof(unsigned int) * AiArrayGetNumElements(diffusePositions));
				SortFloatIndexArray(diffusePositions, shuffle);
				Ramp(diffusePositions, diffuseColors, diff_t, diffuseInterp, shadow_result, shuffle);
				shadow_raw_result = (shadow_result.r + shadow_result.g + shadow_result.b) / 3;
			}
			else{
				if(diff_t >= shadow_position){
					shadow_result = AI_RGB_WHITE;
					shadow_raw_result = AI_RGB_WHITE;
				}
				else{
					shadow_result = shadow_ramp;
					shadow_raw_result = AI_RGB_BLACK;
				}
			}
		}

		if(use_ramp_color)
			result = lerp(texture_result, texture_result * shadow_result, shadow_mask.r);
		else
			result = lerp(texture_result, lerp(color_shadow, texture_result, shadow_raw_result.r), shadow_mask.r);
		break;
	case S_RAYTRACE:
		AtRGB Kd = AiShaderEvalParamRGB(p_lambert_color);
		AtRGB Ks = AI_RGB_WHITE;
		float roughness = 0.2f;
		float specbalance = 0.1f;

		AtRGB Dsa, Dda, IDs, IDd;
		Dsa = Dda = IDs = IDd = AI_RGB_BLACK;
		void *spec_data = AiWardDuerMISCreateData(sg, NULL, NULL, roughness, roughness);
		void *diff_data = AiOrenNayarMISCreateData(sg, 0.0f);
		AiLightsPrepare(sg);
		while(AiLightsGetSample(sg)){
			// direct specular
			if(AiLightGetAffectSpecular(sg->Lp))
				Dsa += AiEvaluateLightSample(sg, spec_data, AiWardDuerMISSample, AiWardDuerMISBRDF, AiWardDuerMISPDF) * specbalance;
			// direct diffuse
			if(AiLightGetAffectDiffuse(sg->Lp))
				Dda += AiEvaluateLightSample(sg, diff_data, AiOrenNayarMISSample, AiOrenNayarMISBRDF, AiOrenNayarMISPDF) * (1 - specbalance);
		}

		// indirect specular
		IDs = AiWardDuerIntegrate(&sg->Nf, sg, &sg->dPdu, &sg->dPdu, &sg->dPdv, roughness, roughness) * specbalance;
		// indirect diffuse
		IDd = AiOrenNayarIntegrate(&sg->Nf, sg, 0.0f) * (1 - specbalance);
		break;
	default:
		result = AI_RGB_RED;
		break;
	}

	if(enable_outline){
		// set outline aov
		result = color_outline;
		AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_outline].c_str(), color_outline);
	}

	// set flat shader aovs
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_color_major], color_major);
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_color_shadow], color_shadow);
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_color_mask], color_mask);
	// set dynamic shadow aov
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_dynamic_shadow], shadow_result);
	// set dynamic shadow raw aov
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_dynamic_shadow_raw], shadow_raw_result);
	// set beauty aov
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_mytoons_beauty], result);

	// caculate normal aov
	AtRGB normal = AiColor(sg->N.x, sg->N.y, sg->N.z);
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_normal], normal);
	// caculate facingratio aov
	AtRGB fresnel = AiColor(1 - AiV3Dot(sg->Nf, -sg->Rd));
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_fresnel], fresnel);
	// caculate depth aov
	AtRGB depth = AiColor(sg->Rl);
	AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_depth], depth);
	// caculate occlusion aov
	if(enable_occlusion){
		AtVector N = sg->Nf;
		AtVector Ng = sg->Ngf;
		float mint = 0.0f;
		float maxt = 2000.0f;
		float spread = 1.0f;
		float falloff = 0.5f;
		AtSampler *sampler = AiSampler(6, 2);
		AtVector Nbent;
		AtRGB occlusion = AI_RGB_WHITE - AiOcclusion(&N, &Ng, sg, mint, maxt, spread, falloff, sampler, &Nbent);
		AiAOVSetRGB(sg, data->aovs_custom[k_my_aov_occlusion], occlusion);
	}

	sg->out.RGB() = result;
	sg->out_opacity = result_opacity;
}
