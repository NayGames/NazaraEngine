// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Nazara Engine - OpenGL Renderer"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_OPENGLRENDERER_VKSAMPLER_HPP
#define NAZARA_OPENGLRENDERER_VKSAMPLER_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/OpenGLRenderer/Wrapper/DeviceObject.hpp>

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

#include <Nazara/OpenGLRenderer/Wrapper/Sampler.inl>

#endif // NAZARA_OPENGLRENDERER_VKSAMPLER_HPP
