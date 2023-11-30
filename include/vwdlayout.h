#ifndef INCLUDEGUARD_VWDLAYOUT
#define INCLUDEGUARD_VWDLAYOUT
#define VKBASIC2D_MAX_LAYER 256

#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

#include "../../dmgrect/include/dmgrect.h"
#include "../../vector/include/vector.h"
#include "../../vkhelper2/include/vkhelper2.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../../vwdlayer/include/vwdlayer.h"

typedef struct {
	VkRenderPass rp_layer;
	VkPipeline ppl_layer;
	VkPipelineLayout ppll_layer;
	bool rebuild_vbuf;
	Vkhelper2Buffer vbufg;
	Vkhelper2Buffer vbufc;
	VkSampler sampler;
	// layer 0/1 are for overlay/preview
	// TODO: mipmap is useful
	Vkhelper2Desc layer;
	Vector layers;
	Vwdlayer output;
	VkFramebuffer output_fb;
} Vwdlayout;

void vwdlayout_init(Vwdlayout *vl, Vkstatic *vks, Dmgrect *dmg);
void vwdlayout_deinit(Vwdlayout *vb2, VkDevice device);

void vwdlayout_build_command(
	Vwdlayout *vb2,
	VkDevice device,
	VkCommandBuffer cbuf);

Vwdlayer *vwdlayout_ldx(Vwdlayout *vb2, size_t ldx);
void vwdlayout_descset_init(Vwdlayout *vb2, VkDevice device);
void vwdlayout_descset_write(Vwdlayout *vb2, VkDevice device);
void vwdlayout_layer_info(Vwdlayout *vb2);
void vwdlayout_insert_layer(Vwdlayout *vb2, Vkstatic *vks, size_t ldx,
	int32_t ox, int32_t oy, uint32_t sx, uint32_t sy);

typedef struct {
	vec2 pos;
	vec2 uv;
	uint32_t ldx;
} VwdlayoutVertex;

#endif
