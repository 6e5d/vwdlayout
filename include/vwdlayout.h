#ifndef INCLUDEGUARD_VWDLAYOUT_VWDLAYOUTH
#define INCLUDEGUARD_VWDLAYOUT_VWDLAYOUTH

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <vulkan/vulkan.h>

typedef struct Vwdlayout Vwdlayout;

#include "../../dmgrect/include/dmgrect.h"
#include "../../vector/include/vector.h"
#include "../../vkhelper2/include/vkhelper2.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../include/layer.h"

#define VKBASIC2D_MAX_LAYER 256

struct Vwdlayout {
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
};

void vwdlayout_init(Vwdlayout *vl, Vkstatic *vks, Dmgrect *dmg);
void vwdlayout_deinit(Vwdlayout *vb2, VkDevice device);

#endif
