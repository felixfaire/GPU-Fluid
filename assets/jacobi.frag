#version 400

in vec2	v_TexCoord0;

uniform sampler2D u_XMat;
uniform sampler2D u_BMat;

uniform float u_Alpha;
uniform float u_InvBeta;
uniform vec2  u_CellSize;

out vec4 Out;


void main()
{
    vec2 p = v_TexCoord0;

    vec4 xL = texture(u_XMat, p - u_CellSize * vec2(1.0, 0.0)); // Left
    vec4 xR = texture(u_XMat, p + u_CellSize * vec2(1.0, 0.0)); // Right
    vec4 xB = texture(u_XMat, p - u_CellSize * vec2(0.0, 1.0)); // Bottom
    vec4 xT = texture(u_XMat, p + u_CellSize * vec2(0.0, 1.0)); // Top
    vec4 bC = texture(u_BMat, p);                               // Center
    
    Out = (xL + xR + xB + xT + u_Alpha * bC) * u_InvBeta;
}
