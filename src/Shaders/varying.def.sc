vec3 a_position  : POSITION;
vec4 a_normal    : NORMAL;
vec3 a_tangent   : TANGENT;
vec3 a_bitangent : BITANGENT;
vec2 a_texcoord0 : TEXCOORD0;

vec3 v_position  : POSITION  = vec3(0.0, 0.0, 0.0);
vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 0.0);
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
vec3 v_tangent   : TANGENT   = vec3(0.0, 0.0, 0.0);
vec3 v_bitangent : BITANGENT = vec3(0.0, 0.0, 0.0);
vec4 v_indices   : INDICES   = ivec4(0.0, 0.0, 0.0, 0.0);
vec4 v_weights   : WEIGHTS   = vec4(0.0, 0.0, 0.0, 0.0);


