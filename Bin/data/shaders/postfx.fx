#include "common.fx"

// PostFX Exponential distance fog.
float4 PS_PostFXFog(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float depth = txGBufferLinearDepth.Load(ss_load_coords).x;
	float3 wPos = getWorldCoords(iPosition.xy, depth);
  float4 in_color = txAlbedo.Sample(samClampLinear, iTex0.xy);
	
	float3 frag_dir = (wPos - camera_pos.xyz);
	float dist = abs(length(frag_dir));
			
	float fog_factor = 1.0 /exp( (dist * global_fog_density)* (dist * global_fog_density));	
	float4 final_color = lerp(float4(global_fog_color,1), in_color, saturate(fog_factor));
	
	//You have to tweak this values
	 float be = 0.225 * smoothstep(0.0, 6.0, 32.0 - wPos.y);
	 float bi = 0.375* smoothstep(0.0, 80, 10.0 - wPos.y);
	 
	float ext = exp(-dist * be);
	float insc = exp(-dist * bi);

	return in_color + float4(global_fog_color,1) * (1 - insc);
}

float2 getDistorsion(float2 iTex0, float value, float shift) 
{
	float r2 = (iTex0.x - 0.5) * (iTex0.x - 0.5) + (iTex0.y - 0.5) * (iTex0.y - 0.5);
	float f = 1 + r2 * (value + shift * sqrt(r2));

	float x = f*(iTex0.x - 0.5) + 0.5;
	float y = f*(iTex0.y - 0.5) + 0.5;

	return float2(x, y);
}

// PostFX Chromatic Aberration
float4 PS_PostFX_CA(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	float chromatic_amount = 0.05;
	float2 bDist = getDistorsion(iTex0, -0.05, 0.05);
	float2 gDist = getDistorsion(iTex0, -0.05 - chromatic_amount, 0.05 + chromatic_amount);
	float2 rDist = getDistorsion(iTex0, -0.05 - chromatic_amount * 2, 0.05 + chromatic_amount * 2);
  
	float4 distorsion_r = txAlbedo.Sample(samClampLinear, rDist);
	float4 distorsion_g = txAlbedo.Sample(samClampLinear, gDist);
	float4 distorsion_b = txAlbedo.Sample(samClampLinear, bDist);

  return float4(distorsion_r.r, distorsion_g.g, distorsion_b.b, 1);
}

// Adaptative lighting methods

float NUM_SAMPLES = 10;
float Density = 1;

float4 PS_PostFX_LScattering( in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	/*float2 deltaTexCoord = (iTex0 - ScreenLightPos.xy);
	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;

	float3 color = txAlbedo.Sample(samClampLinear, iTex0.xy);
	half illuminationDecay = 1.0f;
	
	for (int i = 0; i < NUM_SAMPLES; i++)
  {
    iTex0 -= deltaTexCoord;
		half3 sample = txAlbedo.Sample(samClampLinear, iTex0.xy);
		
		sample *= illuminationDecay * Weight;
    color += sample;
    illuminationDecay *= Decay;
  }
  // Output final color with a further scale control factor.
  return float4( color * Exposure, 1);
	 

  // Store initial sample.
  // Set up illumination decay factor.
  // Evaluate summation from Equation 3 NUM_SAMPLES iterations.
   for (int i = 0; i < NUM_SAMPLES; i++)
  {
    // Step sample location along ray.
    texCoord -= deltaTexCoord;
    // Retrieve sample at new location.
   half3 sample = tex2D(frameSampler, texCoord);
    // Apply sample attenuation scale/decay factors.
    sample *= illuminationDecay * Weight;
    // Accumulate combined color.
    color += sample;
    // Update exponential decay factor.
    illuminationDecay *= Decay;
  }
  // Output final color with a further scale control factor.
   return float4( color * Exposure, 1);
	 */
	return float4(1,0,0,1);
}
