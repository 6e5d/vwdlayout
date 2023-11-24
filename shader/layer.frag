#version 450
#extension GL_EXT_nonuniform_qualifier: enable

layout(location = 0) in vec2 uv;
layout(location = 1) flat in uint ldx;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler2D tex[];

void main() {
	out_color = texture(tex[nonuniformEXT(ldx)], uv);
	// out_color += vec4(0.5, uv[0], uv[1], 0.3);
}
