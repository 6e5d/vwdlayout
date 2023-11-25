#pragma once

#include <stdint.h>
#include <vulkan/vulkan.h>

#include "../../vkhelper/include/image.h"
#include "../../vkstatic/include/vkstatic.h"
#include "../../vwdlayer/include/layer.h"
#include "layer.h"
#include "vwdlayout.h"

Vwdlayer *vwdlayout_ldx(Vwdlayout *vb2, size_t ldx);
void vwdlayout_descset_init(Vwdlayout *vb2, VkDevice device);
void vwdlayout_descset_write(Vwdlayout *vb2, VkDevice device);
void vwdlayout_layer_info(Vwdlayout *vb2);
void vwdlayout_insert_layer(Vwdlayout *vb2, Vkstatic *vks, size_t ldx,
	int32_t ox, int32_t oy, uint32_t sx, uint32_t sy);
