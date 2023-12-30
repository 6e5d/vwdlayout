#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

#include "../../dmgrect/build/dmgrect.h"
#include "../../vector/build/vector.h"
#include "../../vkhelper2/build/vkhelper2.h"
#include "../../vkstatic/build/vkstatic.h"
#include "../../vwdlayer/build/vwdlayer.h"
#include "../../simpleimg/build/simpleimg.h"

const static size_t vwdlayout(max_layer) = 256;

typedef struct {
	VkRenderPass rp_layer;
	VkPipeline ppl_layer;
	VkPipelineLayout ppll_layer;
	bool rebuild_vbuf;
	Vkhelper2(Buffer) vbufg;
	Vkhelper2(Buffer) vbufc;
	VkSampler sampler;
	Vkhelper2(Desc) layer;
	Vector() layers;
	Vwdlayer() output;
	Vkhelper2(Buffer) output_buffer;
	Simpleimg() output_img;
	VkFramebuffer output_fb;
} Vwdlayout();

void vwdlayout(init)(Vwdlayout() *vl, Vkstatic() *vks, Dmgrect() *dmg);
void vwdlayout(deinit)(Vwdlayout() *vl, VkDevice device);

void vwdlayout(download_output)(Vwdlayout() *vl, VkCommandBuffer cbuf);
void vwdlayout(build_command)(
	Vwdlayout() *vl,
	VkDevice device,
	VkCommandBuffer cbuf);

Vwdlayer() *vwdlayout(ldx)(Vwdlayout() *vl, size_t ldx);
void vwdlayout(descset_init)(Vwdlayout() *vl, VkDevice device);
void vwdlayout(descset_write)(Vwdlayout() *vl, VkDevice device);
void vwdlayout(layer_info)(Vwdlayout() *vl);
void vwdlayout(insert_layer)(Vwdlayout() *vl, Vkstatic() *vks, size_t ldx,
	int32_t ox, int32_t oy, uint32_t sx, uint32_t sy);

typedef struct {
	vec2 pos;
	vec2 uv;
	uint32_t ldx;
} Vwdlayout(Vertex);
