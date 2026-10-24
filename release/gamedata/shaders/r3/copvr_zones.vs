cbuffer dynamic_transforms { float4x4 m_WVP; };
struct vin { float3 P:POSITION; float4 C:COLOR0; float2 uv:TEXCOORD0; };
struct vout { float4 P:SV_Position; float4 C:COLOR0; };
vout main(vin i) { vout o; o.P=mul(m_WVP,float4(i.P,1)); o.C=i.C.bgra; return o; }
