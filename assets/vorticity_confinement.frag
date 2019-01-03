#version 400

in vec2	v_TexCoord0;

uniform sampler2D u_Vel;
uniform sampler2D u_Crl;

uniform vec2  u_CellSize;
uniform float u_VorticityScale;

out vec4 Out;

void main()
{
    vec2 p = v_TexCoord0;

    // Find derivative of the magnitude (n = del |w|)
    float dwdx = (texture(u_Crl, p + vec2(u_CellSize.x, 0.0)).x - texture(u_Crl, p - vec2(u_CellSize.x, 0.0)).x) * 0.5;
    float dwdy = (texture(u_Crl, p + vec2(0.0, u_CellSize.y)).x - texture(u_Crl, p - vec2(0.0, u_CellSize.y)).x) * 0.5;

    // Calculate vector length: (|n|). The add small factor to prevent divide by zeros.
    float lngthSq = dwdx * dwdx + dwdy * dwdy;
    float lngth = sqrt(lngthSq) + 0.000001;
    lngth = 1.0 / lngth;
    dwdx *= lngth;
    dwdy *= lngth;
    float v = texture(u_Crl, p).y;
    vec2 outVel = texture(u_Vel, p).xy + u_VorticityScale * vec2(dwdy * -v, dwdx * v);
    
    float edge = (1.0 - step(1.0 - u_CellSize.x * 2.0, abs(p.x - 0.5) * 2.0))
               * (1.0 - step(1.0 - u_CellSize.y * 2.0, abs(p.y - 0.5) * 2.0));
    
    outVel *= edge;

    Out = vec4(outVel.x, outVel.y, 0.0, 0.0);
}
