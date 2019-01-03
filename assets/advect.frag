#version 400

in vec2	v_TexCoord0;

uniform sampler2D u_Src;
uniform sampler2D u_Vel;

uniform float u_Dt;
uniform float u_Dissipation;
uniform vec2  u_CellSize;
uniform vec2  u_Resolution;

out vec4 Out;

void main()
{
    vec2 p = v_TexCoord0 - u_Dt * u_CellSize * texture(u_Vel, v_TexCoord0).xy;

    p = p * u_Resolution; // scale up to texel coords

    vec4 st;
    st.xy = floor(p - 0.5) + 0.5; // left & bottom cell centers
    st.zw = st.xy + 1.0;          // right & top cell centers

    vec2 t = p - st.xy;
    
    st *= u_CellSize.xyxy; // Scale back to normalized coords

    // Bilinear interpolation
    vec4 tex11 = texture(u_Src, st.xy);
    vec4 tex21 = texture(u_Src, st.zy);
    vec4 tex12 = texture(u_Src, st.xw);
    vec4 tex22 = texture(u_Src, st.zw);

    Out = mix(mix(tex11, tex21, t.x), mix(tex12, tex22, t.x), t.y) * u_Dissipation;
}
