#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <vulkan/vulkan.h>

typedef struct Vwdlayout Vwdlayout;

#include "../include/layer.h"
#include "../../vector/include/vector.h"
#include "../../simpleimg/include/simpleimg.h"
#include "../../vkhelper/include/buffer.h"
#include "../../vkhelper/include/desc.h"
#include "../../vkhelper/include/image.h"
#include "../../vkhelper/include/pipeline.h"
#include "../../vkstatic/include/vkstatic.h"

#define VKBASIC2D_MAX_LAYER 256

struct Vwdlayout {
	VkRenderPass rp_layer;
	VkPipeline ppl_layer;
	VkPipelineLayout ppll_layer;
	bool rebuild_vbuf;

	VkhelperBuffer vbufg;
	VkhelperBuffer vbufc;

	// layer 0/1 are for overlay/preview
	// TODO: mipmap is useful
	VkhelperDesc layer;
	Vector layers;
	VkFramebuffer fb_focus;
	uint32_t size[2];
	VkFramebuffer output;
};

void vwdlayout_init(Vwdlayout *vb2, Vkstatic *vks, VkhelperImage *image);
void vwdlayout_deinit(Vwdlayout *vb2, VkDevice device);
