#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "../../vkhelper/include/barrier.h"
#include "../../vkhelper/include/dynstate.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../include/command.h"
#include "../include/vertex.h"
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

static void vwdlayout_build_vbuf(Vwdlayout *vb2, VkDevice device) {
	VwdlayoutVertex* target;
	assert(0 == vkMapMemory(device, vb2->vbufc.memory, 0,
		vb2->vbufc.size, 0, (void**)&target));
	float size[2] = {(float)vb2->size[0], (float)vb2->size[1]};
	float offset[2] = {0.0, 0.0};
	for (size_t i = 1; i < vb2->layers.len; i += 1) {
		Vwdlayer *layer = vector_offset(&vb2->layers, i);
		float x0 = (float)layer->offset[0];
		float y0 = (float)layer->offset[1];
		float x1 = (float)layer->offset[0] + (float)layer->size[0];
		float y1 = (float)layer->offset[1] + (float)layer->size[1];
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
	vkUnmapMemory(device, vb2->vbufc.memory);
}

void vwdlayout_build_command(
	Vwdlayout *vb2,
	VkDevice device,
	VkCommandBuffer cbuf) {
	uint32_t width = vb2->size[0];
	uint32_t height = vb2->size[1];
	if (vb2->rebuild_vbuf) {
		printf("rebuild vbuf %zu\n", vb2->layers.len);
		vwdlayout_build_vbuf(vb2, device);
		vb2->rebuild_vbuf = false;
		VkBufferCopy copy = {
			.size = vb2->layers.len * 6 * sizeof(VwdlayoutVertex)
		};
		vkCmdCopyBuffer(cbuf, vb2->vbufc.buffer,
			vb2->vbufg.buffer, 1, &copy);
	}
	vkhelper_viewport_scissor(cbuf, width, height);
	static const VkClearValue clear_color = {
		.color.float32 = {0.0f, 0.0f, 0.0f, 0.0f},
	};
	static const VkClearValue clear_depthstencil = {
		.depthStencil.depth = 1.0f,
		.depthStencil.stencil = 0,
	};
	VkClearValue clear_values[2] = {clear_color, clear_depthstencil};
	VkRenderPassBeginInfo rp_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = vb2->rp_layer,
		.framebuffer = vb2->output,
		.renderArea.offset.x = 0,
		.renderArea.offset.y = 0,
		.renderArea.extent.width = width,
		.renderArea.extent.height = height,
		.clearValueCount = 2,
		.pClearValues = clear_values,
	};
	vkCmdBeginRenderPass(cbuf, &rp_info, VK_SUBPASS_CONTENTS_INLINE);

	VkDeviceSize zero = 0;
	vkCmdBindPipeline(
		cbuf,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		vb2->ppl_layer);
	vkCmdBindDescriptorSets(
		cbuf,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		vb2->ppll_layer,
		0, 1, &vb2->layer.set, 0, NULL);
	vkCmdBindVertexBuffers(cbuf, 0, 1, &vb2->vbufg.buffer, &zero);
	vkCmdDraw(cbuf, 6 * (uint32_t)vb2->layers.len, 1, 0, 0);
	vkCmdEndRenderPass(cbuf);
}
