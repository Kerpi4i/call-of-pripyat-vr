cbuffer dynamic_transforms { float4x4 m_WVP; };
struct vin { float3 P:POSITION; float4 C:COLOR0; float2 uv:TEXCOORD0; };
struct vout { float4 P:SV_Position; float2 uv:TEXCOORD0; float2 tube:TEXCOORD1; };
vout main(vin i) {
    vout o; o.P=mul(m_WVP,float4(i.P,1)); o.uv=i.uv;
    float4 bytes=round(i.C.bgra*255);
    o.tube=float2(bytes.r+bytes.g*256,bytes.b+bytes.a*256)*(16.0/65535.0)-8;
    return o;
}
