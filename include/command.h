#pragma once

#include <stdint.h>
#include <vulkan/vulkan.h>

#include "../../vkstatic/include/vkstatic.h"
#include "../include/vwdlayout.h"

void vwdlayout_build_command(
	Vwdlayout *vb2,
	VkDevice device,
	VkCommandBuffer cbuf);
