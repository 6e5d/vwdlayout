#ifndef INCLUDEGUARD_VWDLAYOUT_VERTEXH
#define INCLUDEGUARD_VWDLAYOUT_VERTEXH

#include <cglm/cglm.h>
#include <stdint.h>

typedef struct {
	vec2 pos;
	vec2 uv;
	uint32_t ldx;
} VwdlayoutVertex;

#endif
