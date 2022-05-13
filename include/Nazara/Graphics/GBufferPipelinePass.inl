// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - Graphics module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Graphics/GBufferPipelinePass.hpp>
#include <Nazara/Graphics/Debug.hpp>

namespace Nz
{
	inline void GBufferPipelinePass::InvalidateCommandBuffers()
	{
		m_rebuildCommandBuffer = true;
	}

	inline void GBufferPipelinePass::InvalidateElements()
	{
		m_rebuildElements = true;
	}
}

#include <Nazara/Graphics/DebugOff.hpp>
