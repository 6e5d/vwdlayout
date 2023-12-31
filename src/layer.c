#include "../include/vwdlayout.h"

Vwdlayer() *vwdlayout(ldx)(Vwdlayout() *vl, size_t ldx) {
	assert(ldx < vl->layers.len);
	return vector(offset)(&vl->layers, ldx);
}

void vwdlayout(descset_init)(Vwdlayout() *vl, VkDevice device) {
	uint32_t len = vwdlayout(max_layer);
	Vkhelper2(DescConfig) conf;
	vkhelper2(desc_config)(&conf, 1);
	VkDescriptorType ty = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	conf.bindings[0].descriptorType = ty;
	conf.bindings[0].descriptorCount = len;
	VkDescriptorBindingFlags flags =
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	VkDescriptorSetLayoutBindingFlagsCreateInfo flag_ci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = 1,
		.pBindingFlags = &flags,
	};
	conf.layout_ci.pNext = &flag_ci;
	conf.sizes[0].type = ty;
	conf.sizes[0].descriptorCount = len;
	VkDescriptorSetVariableDescriptorCountAllocateInfo vdesc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
		.descriptorSetCount = 1,
		.pDescriptorCounts = &len,
	};
	conf.allocinfo.pNext = &vdesc_info;
	vkhelper2(desc_build)(&vl->layer, &conf, device);
	vkhelper2(desc_config_deinit)(&conf);
}

void vwdlayout(descset_write)(Vwdlayout() *vl, VkDevice device) {
	size_t len2 = vl->layers.len;
	uint32_t len = (uint32_t)len2;
	vwdlayout(layer_info)(vl);
	VkDescriptorImageInfo *infos = calloc(
		len2, sizeof(VkDescriptorImageInfo));
	for (size_t ldx = 0; ldx < len2; ldx += 1) {
		Vwdlayer() *layer = vwdlayout(ldx)(vl, ldx);
		infos[ldx] = (VkDescriptorImageInfo) {
			.imageView = layer->image.imageview,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.sampler = vl->sampler,
		};
	}

	VkWriteDescriptorSet descwrite = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.dstSet = vl->layer.set,
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = len,
		.pImageInfo = infos,
	};
	vkUpdateDescriptorSets(device, 1, &descwrite, 0, NULL);
	free(infos);
}

void vwdlayout(layer_info)(Vwdlayout() *vl) {
	printf("===layer info begin===\n");
	for (size_t ldx = 0; ldx < vl->layers.len; ldx += 1) {
		printf("ldx:%zu size:%u,%u offset:%d,%d\x1b[0m\n",
			ldx,
			vwdlayout(ldx)(vl, ldx)->image.size[0],
			vwdlayout(ldx)(vl, ldx)->image.size[1],
			vwdlayout(ldx)(vl, ldx)->offset[0],
			vwdlayout(ldx)(vl, ldx)->offset[1]);
	}
	printf("\n");
}

void vwdlayout(insert_layer)(Vwdlayout() *vl, Vkstatic() *vks, size_t ldx,
	int32_t ox, int32_t oy, uint32_t sx, uint32_t sy
) {
	Vwdlayer() *pl = (Vwdlayer()*)vector(insert)(&vl->layers, ldx);
	pl->offset[0] = ox;
	pl->offset[1] = oy;
	vkhelper2(image_new_color)(
		&pl->image, vks->device, vks->memprop, sx, sy, false,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | // save
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | // load
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | // focus
			VK_IMAGE_USAGE_SAMPLED_BIT // render
	);
	VkCommandBuffer cbuf = vkstatic(oneshot_begin)(vks);
	vkhelper2(barrier)(cbuf,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_HOST_BIT,
		&pl->image);
	vkstatic(oneshot_end)(cbuf, vks);
}
