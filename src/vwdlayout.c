#include <assert.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "../../ppath/include/ppath.h"
#include "../../vector/include/vector.h"
#include "../../vkhelper/include/barrier.h"
#include "../../vkhelper/include/desc.h"
#include "../../vkhelper/include/renderpass.h"
#include "../../vkhelper/include/shader.h"
#include "../../vkhelper/include/pipeline.h"
#include "../../vkhelper/include/sampler.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../include/vertex.h"
#include "../include/vwdlayout.h"
#include "../include/layer.h"

static void vwdlayout_init_rp_layer(Vwdlayout *vb2, Vkstatic *vks) {
	// renderpass layer
	VkhelperRenderpassConfig renderpass_conf;
	vkhelper_renderpass_config_offscreen(
		&renderpass_conf,
		vks->device);
	renderpass_conf.descs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	vkhelper_renderpass_build(
		&vb2->rp_layer,
		&renderpass_conf,
		vks->device
	);
}

static void vwdlayout_init_pipeline(Vwdlayout *vb2, VkDevice device) {
	char *path;
	VkhelperPipelineConfig vpc = {0};
	vkhelper_pipeline_config(&vpc, 1, 3, 1);

	path = ppath_rel(__FILE__, "../../shader/layer_vert.spv");
	vpc.stages[0].module = vkhelper_shader_module(device, path);
	free(path);

	path = ppath_rel(__FILE__, "../../shader/layer_frag.spv");
	vpc.stages[1].module = vkhelper_shader_module(device, path);
	free(path);

	vpc.vib[0] = (VkVertexInputBindingDescription) {
		.binding = 0,
		.stride = sizeof(VwdlayoutVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	typedef VkVertexInputAttributeDescription Via;
	vpc.via[0] = (Via) {
		.binding = 0,
		.location = 0,
		.format = VK_FORMAT_R32G32B32A32_SFLOAT,
		.offset = offsetof(VwdlayoutVertex, pos),
	};
	vpc.via[1] = (Via) {
		.binding = 0,
		.location = 1,
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = offsetof(VwdlayoutVertex, uv),
	};
	vpc.via[2] = (Via) {
		.binding = 0,
		.location = 2,
		.format = VK_FORMAT_R8_UINT,
		.offset = offsetof(VwdlayoutVertex, ldx),
	};
	vpc.dss.depthTestEnable = VK_FALSE;
	vpc.dss.depthWriteEnable = VK_FALSE;
	vpc.rast.cullMode = VK_CULL_MODE_NONE;
	vpc.desc[0] = vb2->layer.layout;
	vkhelper_pipeline_build(&vb2->ppll_layer, &vb2->ppl_layer,
		&vpc, vb2->rp_layer, device, 0);
	vkhelper_pipeline_config_deinit(&vpc, device);
}

void vwdlayout_init(Vwdlayout *vb2, Vkstatic *vks, VkhelperImage *image) {
	vwdlayout_init_rp_layer(vb2, vks);
	vb2->rebuild_vbuf = true;

	vector_init(&vb2->layers, sizeof(Vwdlayer));
	vector_resize(&vb2->layers, 1);
	Vwdlayer *layers = (Vwdlayer*)vb2->layers.p;
	// dummy layer, no specific reason but just reserve 0
	layers[0] = (Vwdlayer) {
		.sampler = vkhelper_sampler(vks->device),
	};
	vkhelper_image_new(
		&layers[0].image, vks->device, vks->memprop, 1, 1, false,
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);

	vkhelper_buffer_init_cpu(
		&vb2->vbufc, sizeof(VwdlayoutVertex) * VKBASIC2D_MAX_LAYER * 6,
		vks->device, vks->memprop);
	vkhelper_buffer_init_gpu(
		&vb2->vbufg, sizeof(VwdlayoutVertex) * VKBASIC2D_MAX_LAYER * 6,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		vks->device, vks->memprop);

	vwdlayout_descset_init(vb2, vks->device);
	vwdlayout_init_pipeline(vb2, vks->device);

	vb2->size[0] = image->size[0];
	vb2->size[1] = image->size[1];
	VkFramebufferCreateInfo fbci = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = vb2->rp_layer,
		.attachmentCount = 1,
		.pAttachments = &image->imageview,
		.width = image->size[0],
		.height = image->size[1],
		.layers = 1
	};
	assert(0 == vkCreateFramebuffer(
		vks->device, &fbci, NULL, &vb2->output));
}

void vwdlayout_deinit(Vwdlayout *vb2, VkDevice device) {
	vkDestroyPipeline(device, vb2->ppl_layer, NULL);
	vkDestroyPipelineLayout(device, vb2->ppll_layer, NULL);
	vkhelper_desc_deinit(&vb2->layer, device);
	if (vb2->fb_focus != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(device, vb2->fb_focus, NULL);
	}
	for (size_t i = 0; i < vb2->layers.len; i += 1) {
		Vwdlayer *layer = vector_offset(&vb2->layers, i);
		vkhelper_image_deinit(&layer->image, device);
		vkDestroySampler(device, layer->sampler, NULL);
	}
	vector_deinit(&vb2->layers);
	vkhelper_buffer_deinit(&vb2->vbufc, device);
	vkhelper_buffer_deinit(&vb2->vbufg, device);
	vkDestroyRenderPass(device, vb2->rp_layer, NULL);
	vkDestroyFramebuffer(device, vb2->output, NULL);
}
