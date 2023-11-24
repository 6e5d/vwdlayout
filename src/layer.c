#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "../../simpleimg/include/simpleimg.h"
#include "../../vector/include/vector.h"
#include "../../vkhelper/include/barrier.h"
#include "../../vkhelper/include/desc.h"
#include "../../vkhelper/include/image.h"
#include "../../vkhelper/include/sampler.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../../vkstatic/include/oneshot.h"
#include "../include/vwdlayout.h"
#include "../include/layer.h"

Vwdlayer *vwdlayout_ldx(Vwdlayout *vb2, size_t ldx) {
	assert(ldx >= 0 && ldx < vb2->layers.len);
	return vector_offset(&vb2->layers, ldx);
}

void vwdlayout_descset_init(Vwdlayout *vb2, VkDevice device) {
	uint32_t len = VKBASIC2D_MAX_LAYER;
	VkhelperDescConfig conf;
	vkhelper_desc_config(&conf, 1);
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
	vkhelper_desc_build(&vb2->layer, &conf, device);
}

void vwdlayout_descset_write(Vwdlayout *vb2, VkDevice device) {
	size_t len2 = vb2->layers.len;
	uint32_t len = (uint32_t)len2;
	vwdlayout_layer_info(vb2);
	VkDescriptorImageInfo *infos = calloc(
		len2, sizeof(VkDescriptorImageInfo));
	for (size_t ldx = 0; ldx < len2; ldx += 1) {
		Vwdlayer *layer = vwdlayout_ldx(vb2, ldx);
		infos[ldx] = (VkDescriptorImageInfo) {
			.imageView = layer->image.imageview,
			.sampler = layer->sampler,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
	}

	VkWriteDescriptorSet descwrite = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.dstSet = vb2->layer.set,
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = len,
		.pImageInfo = infos,
	};
	vkUpdateDescriptorSets(device, 1, &descwrite, 0, NULL);
	free(infos);
}

void vwdlayout_layer_info(Vwdlayout *vb2) {
	printf("===layer info begin===\n");
	for (size_t ldx = 0; ldx < vb2->layers.len; ldx += 1) {
		printf("ldx:%zu lid:%d size:%u,%u offset:%d,%d\x1b[0m\n",
			ldx,
			vwdlayout_ldx(vb2, ldx)->id,
			vwdlayout_ldx(vb2, ldx)->size[0],
			vwdlayout_ldx(vb2, ldx)->size[1],
			vwdlayout_ldx(vb2, ldx)->offset[0],
			vwdlayout_ldx(vb2, ldx)->offset[1]);
	}
	printf("\n");
}

// return lid
// insert above current focus, if no focus, insert on top
void vwdlayout_insert_layer(Vwdlayout *vb2, Vkstatic *vks,
	size_t ldx, int32_t lid,
	int32_t ox, int32_t oy, uint32_t sx, uint32_t sy
) {
	Vwdlayer *pl = (Vwdlayer*)vector_insert(&vb2->layers, ldx);
	pl->offset[0] = ox;
	pl->offset[1] = oy;
	pl->size[0] = sx;
	pl->size[1] = sy;
	pl->sampler = vkhelper_sampler(vks->device);
	pl->id = lid;
	vkhelper_image_new(
		&pl->image, vks->device, vks->memprop, sx, sy,
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | // save
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | // load
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | // focus
			VK_IMAGE_USAGE_SAMPLED_BIT, // render
		VK_IMAGE_ASPECT_COLOR_BIT);
	VkCommandBuffer cbuf = vkstatic_oneshot_begin(vks);
	vkhelper_barrier(cbuf,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_HOST_BIT,
		pl->image.image);
	vkstatic_oneshot_end(cbuf, vks);
}
