#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <vulkan/vulkan.h>

typedef struct Vwdlayout Vwdlayout;

#include "../../dmgrect/include/dmgrect.h"
#include "../../vector/include/vector.h"
#include "../../vkhelper/include/buffer.h"
#include "../../vkhelper/include/desc.h"
#include "../../vkhelper/include/image.h"
#include "../../vkhelper/include/pipeline.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../include/layer.h"

#define VKBASIC2D_MAX_LAYER 256

struct Vwdlayout {
	VkRenderPass rp_layer;
	VkPipeline ppl_layer;
	VkPipelineLayout ppll_layer;
	bool rebuild_vbuf;

	VkhelperBuffer vbufg;
	VkhelperBuffer vbufc;
	VkSampler sampler;

	// layer 0/1 are for overlay/preview
	// TODO: mipmap is useful
	VkhelperDesc layer;
	Vector layers;
	Vwdlayer output;
	VkFramebuffer output_fb;
};

void vwdlayout_init(Vwdlayout *vl, Vkstatic *vks, Dmgrect *dmg);
void vwdlayout_deinit(Vwdlayout *vb2, VkDevice device);
