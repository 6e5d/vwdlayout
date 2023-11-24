#version 450

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in uint ldx_in;
layout(location = 0) out vec2 uv;
layout(location = 1) out uint ldx;

void main() {
	gl_Position = vec4(pos, 0.0, 1.0);
	uv = uv_in;
	ldx = ldx_in;
}
