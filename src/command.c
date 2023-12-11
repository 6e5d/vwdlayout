#include <vulkan/vulkan.h>

#include "../../dmgrect/include/dmgrect.h"
#include "../../vkhelper2/include/vkhelper2.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../include/vwdlayout.h"

static void p2(vec2 out, float x, float y, float size[2], float offset[2]) {
	vec3 tmp = {x, y, 1.0f};
	mat3 transform;
	glm_mat3_identity(transform);
	transform[0][0] = 2.0f / size[0];
	transform[1][1] = 2.0f / size[1];
	transform[2][0] = -(2.0f * offset[0]) / size[0] - 1.0f;
	transform[2][1] = -(2.0f * offset[1]) / size[1] - 1.0f;
	glm_mat3_mulv(transform, tmp, tmp);
	out[0] = tmp[0];
	out[1] = tmp[1];
}

static void uv(vec2 out, float x, float y) {
	out[0] = x;
	out[1] = y;
}

static void vwdlayout_build_vbuf(Vwdlayout *vl, VkDevice device) {
	VwdlayoutVertex* target;
	assert(0 == vkMapMemory(device, vl->vbufc.memory, 0,
		vl->vbufc.size, 0, (void**)&target));
	float size[2] = {
		(float)vl->output.image.size[0],
		(float)vl->output.image.size[1]
	};
	float offset[2] = {
		(float)vl->output.offset[0],
		(float)vl->output.offset[1],
	};
	for (size_t i = 0; i < vl->layers.len; i += 1) {
		Vwdlayer *l = vector_offset(&vl->layers, i);
		float x0 = (float)l->offset[0];
		float y0 = (float)l->offset[1];
		float x1 = (float)l->offset[0] + (float)l->image.size[0];
		float y1 = (float)l->offset[1] + (float)l->image.size[1];
		p2(target[i * 6 + 0].pos, x0, y0, size, offset);
		p2(target[i * 6 + 1].pos, x1, y0, size, offset);
		p2(target[i * 6 + 2].pos, x0, y1, size, offset);
		p2(target[i * 6 + 3].pos, x0, y1, size, offset);
		p2(target[i * 6 + 4].pos, x1, y0, size, offset);
		p2(target[i * 6 + 5].pos, x1, y1, size, offset);
		uv(target[i * 6 + 0].uv, 0.0, 0.0);
		uv(target[i * 6 + 1].uv, 1.0, 0.0);
		uv(target[i * 6 + 2].uv, 0.0, 1.0);
		uv(target[i * 6 + 3].uv, 0.0, 1.0);
		uv(target[i * 6 + 4].uv, 1.0, 0.0);
		uv(target[i * 6 + 5].uv, 1.0, 1.0);
		uint32_t ii = (uint32_t)i;
		target[i * 6 + 0].ldx = ii;
		target[i * 6 + 1].ldx = ii;
		target[i * 6 + 2].ldx = ii;
		target[i * 6 + 3].ldx = ii;
		target[i * 6 + 4].ldx = ii;
		target[i * 6 + 5].ldx = ii;
	}
	vkUnmapMemory(device, vl->vbufc.memory);
}

void vwdlayout_download_output(Vwdlayout *vl, VkCommandBuffer cbuf) {
	VkOffset3D offset = {0, 0, 0};
	uint32_t w = vl->output.image.size[0];
	uint32_t h = vl->output.image.size[1];
	VkExtent3D extent = {w, h, 1};
	VkImageSubresourceLayers layers = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.layerCount = 1,
	};
	VkBufferImageCopy icopy = {
		.bufferOffset = 0,
		.bufferRowLength = w,
		.bufferImageHeight = h,
		.imageSubresource = layers,
		.imageOffset = offset,
		.imageExtent = extent,
	};
	vkhelper2_barrier_src(cbuf, &vl->output.image);
	vkCmdCopyImageToBuffer(cbuf, vl->output.image.image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		vl->output_buffer.buffer,
		1, &icopy);
}

void vwdlayout_build_command(
	Vwdlayout *vl,
	VkDevice device,
	VkCommandBuffer cbuf) {
	uint32_t width = vl->output.image.size[0];
	uint32_t height = vl->output.image.size[1];
	if (vl->rebuild_vbuf) {
		printf("rebuild vbuf %zu\n", vl->layers.len);
		vwdlayout_build_vbuf(vl, device);
		vl->rebuild_vbuf = false;
		VkBufferCopy copy = {
			.size = vl->layers.len * 6 * sizeof(VwdlayoutVertex)
		};
		vkCmdCopyBuffer(cbuf, vl->vbufc.buffer,
			vl->vbufg.buffer, 1, &copy);
	}
	vkhelper2_dynstate_vs(cbuf, width, height);
	vkhelper2_renderpass_begin_clear(cbuf, vl->rp_layer, vl->output_fb,
		width, height);

	VkDeviceSize zero = 0;
	vkCmdBindPipeline(
		cbuf,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		vl->ppl_layer);
	vkCmdBindDescriptorSets(
		cbuf,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		vl->ppll_layer,
		0, 1, &vl->layer.set, 0, NULL);
	vkCmdBindVertexBuffers(cbuf, 0, 1, &vl->vbufg.buffer, &zero);
	vkCmdDraw(cbuf, 6 * (uint32_t)vl->layers.len, 1, 0, 0);
	vkCmdEndRenderPass(cbuf);
}
