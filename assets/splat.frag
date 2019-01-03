#version 400

in vec2	v_TexCoord0;

uniform sampler2D u_Target;

uniform vec2  u_Pos;
uniform vec4  u_Splat;
uniform float u_Radius;
uniform vec2  u_CellSize;
uniform float u_TargetAspect;

out vec4 Out;

void main()
{
    vec2 p = v_TexCoord0;
    vec2 pos = u_Pos;
    pos.y = 1.0 - pos.y;
    
    p -= pos;
    p.x *= u_TargetAspect;
    
    // Circular splat
    float r = length(p);
    Out = u_Splat * smoothstep(u_CellSize.x * u_Radius, 0.0, r);

//    Square splat
//    Out = u_Splat * (1.0 - step(u_CellSize.x * u_Radius, abs(p.x)))
//                  * (1.0 - step(u_CellSize.y * u_Radius, abs(p.y)));
}
