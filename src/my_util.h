#pragma once

#include <ai.h>

#define UV_INVALID -1000000.0f
#define UV_GLOBALS -2000000.0f


enum RampInterpolationType{
	RIT_NONE = 0,
	RIT_LINEAR,
	RIT_EXP_UP,
	RIT_EXP_DOWN,
	RIT_SMOOTH,
	RIT_BUMP,
	RIT_SPIKE
};

const char* RampInterpolationNames[] = {
	"none",
	"linear",
	"exponentialup",
	"exponentialdown",
	"smooth",
	"bump",
	"spike",
	NULL
};

namespace my{
	bool IsValidUV(AtVector2 &uv)
	{
		// place2dTexture return (UV_INVALID, UV_INVALID) for invalid uvs
		return (uv.x > UV_INVALID && uv.y > UV_INVALID);
	}

	bool SortFloatIndexArray(AtArray *a, unsigned int *shuffle)
	{
		bool modified = false;
		if(a && shuffle && AiArrayGetNumElements(a) > 0){
			float p0, p1;
			int tmp;
			bool swapped = true;
			uint32_t n = AiArrayGetNumElements(a);

			for(uint32_t i = 0; i < n; i++)
				shuffle[i] = i;

			while(swapped){
				swapped = false;
				n -= 1;
				for(uint32_t i = 0; i < n; i++){
					p0 = AiArrayGetFlt(a, shuffle[i]);
					p1 = AiArrayGetFlt(a, shuffle[i + 1]);
					if(p0 > p1){
						swapped = true;
						modified = true;

						tmp = shuffle[i];
						shuffle[i] = shuffle[i + 1];
						shuffle[i + 1] = tmp;
					}
				}
			}		
		}
		return modified;
	}

	AtRGB _GetArrayRGB(AtArray *a, unsigned int i)
	{
		return AiArrayGetRGB(a, i);
	}

	float RampLuminance(float v)
	{
		return v;
	}

	float RampLuminance(const AtRGB &color)
	{
		return 0.3 * color.r + 0.3 * color.g + 0.3 * color.b;
	}

	template <typename ValType>
		ValType Mix(ValType a, ValType b, float t)
		{
			return a + t * (b - a);
		}

	template <typename ValType>
		void RampT(AtArray *p, AtArray *c, float t, RampInterpolationType it, ValType &result, ValType (*getv)(AtArray*, unsigned int), unsigned int *shuffle)
		{
			unsigned int inext = AiArrayGetNumElements(p);
			unsigned int oriext = inext;
			for(unsigned int i = 0; i < oriext; i++){
				if(t < AiArrayGetFlt(p, shuffle[i])){
					inext = i;
					break;
				}
			}
			
			if(inext >= oriext){
				result = getv(c, shuffle[oriext - 1]);
				return;
			}

			if(inext == 0){
				result = getv(c, shuffle[0]);
				return;
			}

			unsigned int icur = inext - 1;
			float tcur = AiArrayGetFlt(p, shuffle[icur]);
			float tnext = AiArrayGetFlt(p, shuffle[inext]);
			ValType ccur = getv(c, shuffle[icur]);
			ValType cnext = getv(c, shuffle[inext]);
			float u = (t - tcur) / (tnext - tcur);

			switch(it){
			case RIT_LINEAR:
				//u = u;
				break;
			case RIT_EXP_UP:
				u = u * u;
				break;
			case RIT_EXP_DOWN:
				u = 1 - (1 - u) * (1 - u);
				break;
			case RIT_SMOOTH:
				u = 0.5 * (static_cast<float>(cos((u + 1) * AI_PI)) + 1.0);
				break;
			case RIT_BUMP:
				{
					float lcur = RampLuminance(ccur);
					float lnext = RampLuminance(cnext);
					if(lcur < lnext)
						u = sin(u * static_cast<float>(AI_PI) / 2);
					else
						u = sin((u - 1) * static_cast<float>(AI_PI) / 2) + 1.0;
				}
				break;
			case RIT_SPIKE:
				{
					float lcur = RampLuminance(ccur);
					float lnext = RampLuminance(cnext);
					if(lcur > lnext)
						u = sin(u * static_cast<float>(AI_PI) / 2);
					else
						u = sin((u - 1) * static_cast<float>(AI_PI) / 2) + 1.0;
				}
				break;
			case RIT_NONE:
			default:
				u = 0.0;
			}

			result = Mix(ccur, cnext, u);
		}

	template <typename ValType>
		void Ramp(AtArray *p, AtArray *v, float t, RampInterpolationType it, ValType &out, unsigned int *shuffle)
		{
			RampT(p, v, t, it, out, _GetArrayRGB, shuffle);
		}
}
