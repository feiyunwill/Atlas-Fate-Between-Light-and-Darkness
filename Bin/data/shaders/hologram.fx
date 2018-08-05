//--------------------------------------------------------------------------------------
#include "common.fx"

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Vertex
//--------------------------------------------------------------------------------------
void VS_GBuffer_Hologram(
	in float4 iPos     : POSITION
	, in float3 iNormal : NORMAL0
	, in float2 iTex0 : TEXCOORD0
	, in float2 iTex1 : TEXCOORD1
	, in float4 iTangent : NORMAL1

	, out float4 oPos : SV_POSITION
	, out float3 oNormal : NORMAL0
	, out float4 oTangent : NORMAL1
	, out float2 oTex0 : TEXCOORD0
	, out float2 oTex1 : TEXCOORD1
	, out float3 oWorldPos : TEXCOORD2
)
{
	iPos.x += 0.5 * (step(0.5, sin(global_world_time * 2.0 + iPos.y * 1.0)) * step(0.99, sin(global_world_time *4 * 0.5)));
	
	float4 world_pos = mul(iPos, obj_world);
	oPos = mul(world_pos, camera_view_proj);
	
	// Rotar la normal segun la transform del objeto
	oNormal = mul(iNormal, (float3x3)obj_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
	oTangent.w = iTangent.w;

	// Las uv's se pasan directamente al ps
	oTex0 = iTex0;
	oTex1 = iTex1;
	oWorldPos = world_pos.xyz;
}

//--------------------------------------------------------------------------------------
// GBuffer generation pass. Pixel
//--------------------------------------------------------------------------------------
float4 PS_GBuffer_Hologram(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
): SV_Target0
{
	// Retrieve main colors.
	float4 albedo = txAlbedo.Sample(samLinear, iTex0);
	float4 flicker = txNoiseMap.Sample(samLinear, iTex0 );
		
	float4 rim_base_color = float4(0,1,1,1);
		
	// Compute the scanline
	float vertex_sift = (dot(iWorldPos, normalize(float4(float3(0,-1,0), 1.0))) + 1) / 2;
	float scan = step(frac(vertex_sift * 22 + global_world_time * 4), 0.5) * 0.65;
		
	// Compute the glow factor
	float glow = frac(-vertex_sift * 1 - global_world_time * 0.7);
	
	// Compute the rim factor
	float3 view_dir = normalize((iWorldPos - camera_pos).xyz);
	float rim = 1.0 - saturate(dot(-view_dir, iNormal));
	float4 rim_color = rim_base_color * pow(rim, 4.1);
	
	// Compute the final result
	float4 final_color = rim_base_color * albedo + (glow * 0.35 * albedo) + rim_color;
	final_color.a = color_material.a * ( scan + rim + glow ) * 0.35;
	return final_color;
}


//--------------------------------------------------------------------------------------
// Hologram: Star Wars like
//--------------------------------------------------------------------------------------
void VS_GBuffer_SWHologram(
	in float4 iPos     : POSITION
	, in float3 iNormal : NORMAL0
	, in float2 iTex0 : TEXCOORD0
	, in float2 iTex1 : TEXCOORD1
	, in float4 iTangent : NORMAL1

	, out float4 oPos : SV_POSITION
	, out float3 oNormal : NORMAL0
	, out float4 oTangent : NORMAL1
	, out float2 oTex0 : TEXCOORD0
	, out float2 oTex1 : TEXCOORD1
	, out float3 oWorldPos : TEXCOORD2
	, out float3 oModelPos : TEXCOORD3
	, out float max_height : TEXCOORD4
)
{
	// Displacement control
	float prev_height = iPos.y;
	max_height = abs(sin(global_world_time));
	float int_width = 0.5;//abs(sin(global_world_time)) * 0.5;

	iPos.y *= max_height;
	float3 disp_center = float3(0, iPos.y, 0);
	float3 disp_dir = disp_center + (iPos - disp_center) * (1 * int_width + int_width * prev_height);
	iPos = float4(disp_dir,1);
	
	// Regular transforms
	float4 world_pos = mul(iPos, obj_world);
	oPos = mul(world_pos, camera_view_proj);
	//oModelPos = mul(float3(0,0,0), obj_world); 
	oModelPos = float3(obj_world[3][0], obj_world[3][1], obj_world[3][2]);
	
	oNormal = mul(iNormal, (float3x3)obj_world);
	oTangent.xyz = mul(iTangent.xyz, (float3x3)obj_world);
	oTangent.w = iTangent.w;

	oTex0 = iTex0;
	oTex1 = iTex1;
	oWorldPos = world_pos.xyz;
}

float4 PS_GBuffer_SWHologram(
  float4 Pos       : SV_POSITION
  , float3 iNormal : NORMAL0
  , float4 iTangent : NORMAL1
  , float2 iTex0 : TEXCOORD0
  , float2 iTex1 : TEXCOORD1
  , float3 iWorldPos : TEXCOORD2
	, float3 iModelPos : TEXCOORD3
	, float  iMaxHeight : TEXCOORD4
): SV_Target0
{
	// Retrieve main colors.
	float4 albedo = txAlbedo.Sample(samLinear, iTex0);
	
	// Compute the scanline
	float vertex_sift = (dot(iWorldPos, normalize(float4(float3(0,-1,0), 1.0))) + 1) * .5;
	float scan = frac(vertex_sift * 35 + global_world_time * 70) * 0.96;
	
	float3 mid_point = float3(iModelPos.x, iWorldPos.y, iModelPos.z);
	float3 cam_point = float3(camera_pos.x, iWorldPos.y, camera_pos.z);
	float3 mid_dir = normalize(mid_point - cam_point);
	float3 side_dir = normalize(iWorldPos - cam_point);
	
	float d_part = dot(mid_dir, side_dir);
	//d_part = pow(d_part, -4.7);
	
	// Compute the glow factor
	float glow = frac(-vertex_sift * 1 - global_world_time * 0.7);
	
	float3 dir = normalize(iWorldPos.xyz - iModelPos);
	float theta = dot(dir, float3(0,1,0));
	
	float att = 1 - distance(iModelPos, iWorldPos.xyz) / (2*iMaxHeight + 0.3);
	float att2 = 1/distance(float3(iModelPos.x, iWorldPos.y, iModelPos.z), iWorldPos.xyz) * .75;
	float radius = clamp((theta - 0.45) / (0.15 - 0.45), 0, 1);
	float clamp_spot = d_part > 0.92 ? 1.0: d_part; // spot factor 
		
	float4 flicker = txNoiseMap.Sample(samLinear, float2(iTex0.x, iTex0.y + global_world_time * 4));
	
	return scan * float4(0,0.15,1,1) * att * att2; 
}
