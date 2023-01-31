#version 430 core
// This shader is based on the work by user Rabbid76 (Gernot Steinegger, https://github.com/Rabbid76)
// as published on stack overflow: https://stackoverflow.com/a/59688394

layout(std430, binding = 0) buffer TVertex
{
	// Always use vec4 in buffers! Otherwise, memory alignment will be hellish
	vec4 vertex[];
};

uniform mat4 tform;
uniform vec2 screen;
// Given in pixel units
uniform float thickness;
uniform float f_coef;

out float flogz;

void main()
{
	// The technique is based on drawing "fake" vertices, and using
	// the indices of them to build the polygons that form the line
	int line_idx = gl_VertexID / 6;
	int tri_idx = gl_VertexID % 6;

	// Transform vertices into screen space, as thickness is given in pixel units
	vec4 va[4];
	for(int i = 0; i < 4; i++)
	{
		va[i] = tform * vertex[line_idx + i];
		// Perspective divide
		va[i].xyz /= va[i].w;
		va[i].xy = (va[i].xy + 1.0) * 0.5 * screen;
	}

	// Line and perpendicular vectors
	vec2 v_line = normalize(va[2].xy - va[1].xy);
	vec2 nv_line = vec2(-v_line.y, v_line.x);

	// Mitering (joining succesive lines smoothly)
	vec4 pos;
	if (tri_idx == 0 || tri_idx == 1 || tri_idx == 3)
	{
		vec2 v_pred = normalize(va[1].xy - va[0].xy);
		vec2 v_miter = normalize(nv_line + vec2(-v_pred.y, v_pred.x));

		pos = va[1];
		pos.xy += v_miter * thickness * (tri_idx == 1 ? -0.5 : 0.5) / dot(v_miter, nv_line);
	}
	else
	{
		vec2 v_succ = normalize(va[3].xy - va[2].xy);
		vec2 v_miter = normalize(nv_line + vec2(-v_succ.y, v_succ.x));

		pos = va[2];
		pos.xy += v_miter * thickness * (tri_idx == 5 ? 0.5 : -0.5) / dot(v_miter, nv_line);
	}

	// Transform into clipspace
	pos.xy = pos.xy / screen * 2.0 - 1.0;
	pos.xyz *= pos.w;
	gl_Position = pos;

	// Logarithmic depth buffer
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * f_coef - 1.0;
	flogz = 1.0 + gl_Position.w;

}
