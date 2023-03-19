// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Vulkan renderer"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_VULKANRENDERER_WRAPPER_SAMPLER_HPP
#define NAZARA_VULKANRENDERER_WRAPPER_SAMPLER_HPP

#include <NazaraUtils/Prerequisites.hpp>
#include <Nazara/VulkanRenderer/Wrapper/DeviceObject.hpp>

namespace Nz
{
	namespace Vk
	{
		class Sampler : public DeviceObject<Sampler, VkSampler, VkSamplerCreateInfo, VK_OBJECT_TYPE_SAMPLER>
		{
			friend DeviceObject;

			public:
				Sampler() = default;
				Sampler(const Sampler&) = delete;
				Sampler(Sampler&&) = default;
				~Sampler() = default;

				Sampler& operator=(const Sampler&) = delete;
				Sampler& operator=(Sampler&&) = delete;

			private:
				static inline VkResult CreateHelper(Device& device, const VkSamplerCreateInfo* createInfo, const VkAllocationCallbacks* allocator, VkSampler* handle);
				static inline void DestroyHelper(Device& device, VkSampler handle, const VkAllocationCallbacks* allocator);
		};
	}
}

#include <Nazara/VulkanRenderer/Wrapper/Sampler.inl>

#endif // NAZARA_VULKANRENDERER_WRAPPER_SAMPLER_HPP
