#include "../include/vwdlayout.h"

static void vwdlayout(init_rp_layer)(Vwdlayout() *vl, VkDevice device) {
	// renderpass layer
	Vkhelper2(RenderpassConfig) renderpass_conf;
	vkhelper2(renderpass_config_offscreen)(&renderpass_conf);
	renderpass_conf.descs[0].initialLayout =
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	renderpass_conf.descs[0].finalLayout =
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	renderpass_conf.descs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	vkhelper2(renderpass_build)(
		&vl->rp_layer,
		&renderpass_conf,
		device
	);
}

static void vwdlayout(init_pipeline)(Vwdlayout() *vl, VkDevice device) {
	Vkhelper2(PipelineConfig) vpc = {0};
	vkhelper2(pipeline_config)(&vpc, 1, 3, 1);
	vkhelper2(pipeline_simple_shader)(&vpc, device,
		__FILE__, "../../vwdraw_shaders/build/layer");
	vpc.vib[0] = (VkVertexInputBindingDescription) {
		.binding = 0,
		.stride = sizeof(Vwdlayout(Vertex)),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	vpc.via[0] = (VkVertexInputAttributeDescription) {
		.binding = 0,
		.location = 0,
		.format = VK_FORMAT_R32G32B32A32_SFLOAT,
		.offset = 0,
	};
	vpc.via[1] = (VkVertexInputAttributeDescription) {
		.binding = 0,
		.location = 1,
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = 8,
	};
	vpc.via[2] = (VkVertexInputAttributeDescription) {
		.binding = 0,
		.location = 2,
		.format = VK_FORMAT_R8_UINT,
		.offset = 16,
	};
	vpc.dss.depthTestEnable = VK_FALSE;
	vpc.dss.depthWriteEnable = VK_FALSE;
	vpc.rast.cullMode = VK_CULL_MODE_NONE;
	vpc.desc[0] = vl->layer.layout;
	vkhelper2(pipeline_build)(&vl->ppll_layer, &vl->ppl_layer,
		&vpc, vl->rp_layer, device, 0);
	vkhelper2(pipeline_config_deinit)(&vpc, device);
}

void vwdlayout(init)(Vwdlayout() *vl, Vkstatic() *vks, Dmgrect() *dmg) {
	vl->sampler = vkhelper2(sampler)(vks->device);

	// setup output
	vl->output.offset[0] = dmg->offset[0];
	vl->output.offset[1] = dmg->offset[1];
	vkhelper2(image_new_color)(
		&vl->output.image, vks->device, vks->memprop,
		dmg->size[0], dmg->size[1], false,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	vkhelper2(buffer_init_cpu)(
		&vl->output_buffer, dmg->size[0] * dmg->size[1] * 4,
		vks->device, vks->memprop);
	assert(0 == vkMapMemory(vks->device, vl->output_buffer.memory, 0,
		vl->output_buffer.size, 0, (void**)&vl->output_img.data));
	vl->output_img.width = dmg->size[0];
	vl->output_img.height = dmg->size[1];

	VkCommandBuffer cbuf = vkstatic(oneshot_begin)(vks);
	vkhelper2(barrier)(cbuf, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		&vl->output.image);
	vkstatic(oneshot_end)(cbuf, vks);
	vwdlayout(init_rp_layer)(vl, vks->device);
	vl->rebuild_vbuf = true;

	vector(init)(&vl->layers, sizeof(Vwdlayer()));
	vkhelper2(buffer_init_cpu)(
		&vl->vbufc, sizeof(Vwdlayout(Vertex)) * vwdlayout(max_layer) * 6,
		vks->device, vks->memprop);
	vkhelper2(buffer_init_gpu)(
		&vl->vbufg, sizeof(Vwdlayout(Vertex)) * vwdlayout(max_layer) * 6,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		vks->device, vks->memprop);

	vwdlayout(descset_init)(vl, vks->device);
	vwdlayout(init_pipeline)(vl, vks->device);

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

void vwdlayout(deinit)(Vwdlayout() *vl, VkDevice device) {
	vkDestroyPipeline(device, vl->ppl_layer, NULL);
	vkDestroyPipelineLayout(device, vl->ppll_layer, NULL);
	vkhelper2(desc_deinit)(&vl->layer, device);
	vkDestroySampler(device, vl->sampler, NULL);
	for (size_t i = 0; i < vl->layers.len; i += 1) {
		Vwdlayer() *layer = vector(offset)(&vl->layers, i);
		vkhelper2(image_deinit)(&layer->image, device);
	}
	vkhelper2(image_deinit)(&vl->output.image, device);
	vkhelper2(buffer_deinit)(&vl->output_buffer, device);
	vector(deinit)(&vl->layers);
	vkhelper2(buffer_deinit)(&vl->vbufc, device);
	vkhelper2(buffer_deinit)(&vl->vbufg, device);
	vkDestroyRenderPass(device, vl->rp_layer, NULL);
	vkDestroyFramebuffer(device, vl->output_fb, NULL);
}
