#include <assert.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "../../dmgrect/include/dmgrect.h"
#include "../../vector/include/vector.h"
#include "../../vkhelper/include/barrier.h"
#include "../../vkhelper/include/desc.h"
#include "../../vkhelper/include/renderpass.h"
#include "../../vkhelper/include/shader.h"
#include "../../vkhelper/include/pipeline.h"
#include "../../vkhelper/include/sampler.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../../vkstatic/include/oneshot.h"
#include "../include/vertex.h"
#include "../include/vwdlayout.h"
#include "../include/layer.h"

static void vwdlayout_init_rp_layer(Vwdlayout *vl, Vkstatic *vks) {
	// renderpass layer
	VkhelperRenderpassConfig renderpass_conf;
	vkhelper_renderpass_config_offscreen(
		&renderpass_conf,
		vks->device);
	renderpass_conf.descs[0].initialLayout =
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	renderpass_conf.descs[0].finalLayout =
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	renderpass_conf.descs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	vkhelper_renderpass_build(
		&vl->rp_layer,
		&renderpass_conf,
		vks->device
	);
}

static void vwdlayout_init_pipeline(Vwdlayout *vl, VkDevice device) {
	VkhelperPipelineConfig vpc = {0};
	vkhelper_pipeline_config(&vpc, 1, 3, 1);
	vkhelper_pipeline_simple_shader(&vpc, device,
		__FILE__, "../../shader/layer");
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
	vpc.desc[0] = vl->layer.layout;
	vkhelper_pipeline_build(&vl->ppll_layer, &vl->ppl_layer,
		&vpc, vl->rp_layer, device, 0);
	vkhelper_pipeline_config_deinit(&vpc, device);
}

void vwdlayout_init(Vwdlayout *vl, Vkstatic *vks, Dmgrect *dmg) {
	vl->sampler = vkhelper_sampler(vks->device);
	vl->output.offset[0] = dmg->offset[0];
	vl->output.offset[1] = dmg->offset[1];
	vkhelper_image_new_color(
		&vl->output.image, vks->device, vks->memprop,
		dmg->size[0], dmg->size[1], false,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	VkCommandBuffer cbuf = vkstatic_oneshot_begin(vks);
	vkhelper_barrier(cbuf, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		&vl->output.image);
	vkstatic_oneshot_end(cbuf, vks);
	vwdlayout_init_rp_layer(vl, vks);
	vl->rebuild_vbuf = true;

	vector_init(&vl->layers, sizeof(Vwdlayer));
	vkhelper_buffer_init_cpu(
		&vl->vbufc, sizeof(VwdlayoutVertex) * VKBASIC2D_MAX_LAYER * 6,
		vks->device, vks->memprop);
	vkhelper_buffer_init_gpu(
		&vl->vbufg, sizeof(VwdlayoutVertex) * VKBASIC2D_MAX_LAYER * 6,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		vks->device, vks->memprop);

	vwdlayout_descset_init(vl, vks->device);
	vwdlayout_init_pipeline(vl, vks->device);

	VkFramebufferCreateInfo fbci = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = vl->rp_layer,
		.attachmentCount = 1,
		.pAttachments = &vl->output.image.imageview,
		.width = dmg->size[0],
		.height = dmg->size[1],
		.layers = 1
	};
	assert(0 == vkCreateFramebuffer(
		vks->device, &fbci, NULL, &vl->output_fb));
}

void vwdlayout_deinit(Vwdlayout *vl, VkDevice device) {
	vkDestroyPipeline(device, vl->ppl_layer, NULL);
	vkDestroyPipelineLayout(device, vl->ppll_layer, NULL);
	vkhelper_desc_deinit(&vl->layer, device);
	vkDestroySampler(device, vl->sampler, NULL);
	for (size_t i = 0; i < vl->layers.len; i += 1) {
		Vwdlayer *layer = vector_offset(&vl->layers, i);
		vkhelper_image_deinit(&layer->image, device);
	}
	vkhelper_image_deinit(&vl->output.image, device);
	vector_deinit(&vl->layers);
	vkhelper_buffer_deinit(&vl->vbufc, device);
	vkhelper_buffer_deinit(&vl->vbufg, device);
	vkDestroyRenderPass(device, vl->rp_layer, NULL);
	vkDestroyFramebuffer(device, vl->output_fb, NULL);
}
