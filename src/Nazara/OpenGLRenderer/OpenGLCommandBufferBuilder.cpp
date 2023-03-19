// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - OpenGL renderer"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/OpenGLRenderer/OpenGLCommandBufferBuilder.hpp>
#include <Nazara/OpenGLRenderer/OpenGLCommandBuffer.hpp>
#include <Nazara/OpenGLRenderer/OpenGLComputePipeline.hpp>
#include <Nazara/OpenGLRenderer/OpenGLRenderPass.hpp>
#include <Nazara/OpenGLRenderer/OpenGLRenderPipeline.hpp>
#include <Nazara/OpenGLRenderer/OpenGLRenderPipelineLayout.hpp>
#include <Nazara/OpenGLRenderer/OpenGLShaderBinding.hpp>
#include <Nazara/OpenGLRenderer/OpenGLTexture.hpp>
#include <Nazara/OpenGLRenderer/OpenGLUploadPool.hpp>
#include <NazaraUtils/StackArray.hpp>
#include <stdexcept>
#include <Nazara/OpenGLRenderer/Debug.hpp>

namespace Nz
{
	void OpenGLCommandBufferBuilder::BeginDebugRegion(const std::string_view& regionName, const Color& color)
	{
		m_commandBuffer.BeginDebugRegion(regionName, color);
	}

	void OpenGLCommandBufferBuilder::BeginRenderPass(const Framebuffer& framebuffer, const RenderPass& renderPass, const Recti& /*renderRect*/, const ClearValues* clearValues, std::size_t clearValueCount)
	{
		m_commandBuffer.SetFramebuffer(static_cast<const OpenGLFramebuffer&>(framebuffer), static_cast<const OpenGLRenderPass&>(renderPass), clearValues, clearValueCount);
	}

	void OpenGLCommandBufferBuilder::BindComputePipeline(const ComputePipeline& pipeline)
	{
		const OpenGLComputePipeline& glPipeline = static_cast<const OpenGLComputePipeline&>(pipeline);

		m_commandBuffer.BindComputePipeline(&glPipeline);
	}

	void OpenGLCommandBufferBuilder::BindComputeShaderBinding(UInt32 set, const ShaderBinding& binding)
	{
		const OpenGLShaderBinding& glBinding = static_cast<const OpenGLShaderBinding&>(binding);

		m_commandBuffer.BindComputeShaderBinding(glBinding.GetOwner(), set, &glBinding);
	}

	void OpenGLCommandBufferBuilder::BindComputeShaderBinding(const RenderPipelineLayout& pipelineLayout, UInt32 set, const ShaderBinding& binding)
	{
		const OpenGLRenderPipelineLayout& glPipelineLayout = static_cast<const OpenGLRenderPipelineLayout&>(pipelineLayout);
		const OpenGLShaderBinding& glBinding = static_cast<const OpenGLShaderBinding&>(binding);

		m_commandBuffer.BindComputeShaderBinding(glPipelineLayout, set, &glBinding);
	}

	void OpenGLCommandBufferBuilder::BindIndexBuffer(const RenderBuffer& indexBuffer, IndexType indexType, UInt64 offset)
	{
		const OpenGLBuffer& glBuffer = static_cast<const OpenGLBuffer&>(indexBuffer);

		m_commandBuffer.BindIndexBuffer(glBuffer.GetBuffer().GetObjectId(), indexType, offset);
	}

	void OpenGLCommandBufferBuilder::BindRenderPipeline(const RenderPipeline& pipeline)
	{
		const OpenGLRenderPipeline& glPipeline = static_cast<const OpenGLRenderPipeline&>(pipeline);

		m_commandBuffer.BindRenderPipeline(&glPipeline);
	}

	void OpenGLCommandBufferBuilder::BindRenderShaderBinding(UInt32 set, const ShaderBinding& binding)
	{
		const OpenGLShaderBinding& glBinding = static_cast<const OpenGLShaderBinding&>(binding);

		m_commandBuffer.BindRenderShaderBinding(glBinding.GetOwner(), set, &glBinding);
	}

	void OpenGLCommandBufferBuilder::BindRenderShaderBinding(const RenderPipelineLayout& pipelineLayout, UInt32 set, const ShaderBinding& binding)
	{
		const OpenGLRenderPipelineLayout& glPipelineLayout = static_cast<const OpenGLRenderPipelineLayout&>(pipelineLayout);
		const OpenGLShaderBinding& glBinding = static_cast<const OpenGLShaderBinding&>(binding);

		m_commandBuffer.BindRenderShaderBinding(glPipelineLayout, set, &glBinding);
	}

	void OpenGLCommandBufferBuilder::BindVertexBuffer(UInt32 binding, const RenderBuffer& vertexBuffer, UInt64 offset)
	{
		const OpenGLBuffer& glBuffer = static_cast<const OpenGLBuffer&>(vertexBuffer);

		m_commandBuffer.BindVertexBuffer(binding, glBuffer.GetBuffer().GetObjectId(), offset);
	}

	void OpenGLCommandBufferBuilder::BlitTexture(const Texture& fromTexture, const Boxui& fromBox, TextureLayout /*fromLayout*/, const Texture& toTexture, const Boxui& toBox, TextureLayout /*toLayout*/, SamplerFilter filter)
	{
		const OpenGLTexture& sourceTexture = static_cast<const OpenGLTexture&>(fromTexture);
		const OpenGLTexture& targetTexture = static_cast<const OpenGLTexture&>(toTexture);

		m_commandBuffer.BlitTexture(sourceTexture, fromBox, targetTexture, toBox, filter);
	}

	void OpenGLCommandBufferBuilder::CopyBuffer(const RenderBufferView& source, const RenderBufferView& target, UInt64 size, UInt64 sourceOffset, UInt64 targetOffset)
	{
		OpenGLBuffer& sourceBuffer = *static_cast<OpenGLBuffer*>(source.GetBuffer());
		OpenGLBuffer& targetBuffer = *static_cast<OpenGLBuffer*>(target.GetBuffer());

		m_commandBuffer.CopyBuffer(sourceBuffer.GetBuffer().GetObjectId(), targetBuffer.GetBuffer().GetObjectId(), size, sourceOffset + source.GetOffset(), targetOffset + target.GetOffset());
	}

	void OpenGLCommandBufferBuilder::CopyBuffer(const UploadPool::Allocation& allocation, const RenderBufferView& target, UInt64 size, UInt64 sourceOffset, UInt64 targetOffset)
	{
		OpenGLBuffer& targetBuffer = *static_cast<OpenGLBuffer*>(target.GetBuffer());

		m_commandBuffer.CopyBuffer(allocation, targetBuffer.GetBuffer().GetObjectId(), size, sourceOffset, target.GetOffset() + targetOffset);
	}

	void OpenGLCommandBufferBuilder::CopyTexture(const Texture& fromTexture, const Boxui& fromBox, TextureLayout /*fromLayout*/, const Texture& toTexture, const Vector3ui& toPos, TextureLayout /*toLayout*/)
	{
		const OpenGLTexture& sourceTexture = static_cast<const OpenGLTexture&>(fromTexture);
		const OpenGLTexture& targetTexture = static_cast<const OpenGLTexture&>(toTexture);

		m_commandBuffer.CopyTexture(sourceTexture, fromBox, targetTexture, toPos);
	}

	void OpenGLCommandBufferBuilder::Dispatch(UInt32 workgroupX, UInt32 workgroupY, UInt32 workgroupZ)
	{
		m_commandBuffer.Dispatch(workgroupX, workgroupY, workgroupZ);
	}

	void OpenGLCommandBufferBuilder::Draw(UInt32 vertexCount, UInt32 instanceCount, UInt32 firstVertex, UInt32 firstInstance)
	{
		m_commandBuffer.Draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void OpenGLCommandBufferBuilder::DrawIndexed(UInt32 indexCount, UInt32 instanceCount, UInt32 firstIndex, UInt32 firstInstance)
	{
		m_commandBuffer.DrawIndexed(indexCount, instanceCount, firstIndex, firstInstance);
	}

	void OpenGLCommandBufferBuilder::EndDebugRegion()
	{
		m_commandBuffer.EndDebugRegion();
	}

	void OpenGLCommandBufferBuilder::EndRenderPass()
	{
		/* nothing to do */
	}

	void OpenGLCommandBufferBuilder::NextSubpass()
	{
		/* nothing to do */
	}

	void OpenGLCommandBufferBuilder::PreTransferBarrier()
	{
		/* nothing to do */
	}

	void OpenGLCommandBufferBuilder::PostTransferBarrier()
	{
		/* nothing to do */
	}

	void OpenGLCommandBufferBuilder::SetScissor(const Recti& scissorRegion)
	{
		m_commandBuffer.SetScissor(scissorRegion);
	}

	void OpenGLCommandBufferBuilder::SetViewport(const Recti& viewportRegion)
	{
		m_commandBuffer.SetViewport(viewportRegion);
	}

	void OpenGLCommandBufferBuilder::TextureBarrier(PipelineStageFlags /*srcStageMask*/, PipelineStageFlags /*dstStageMask*/, MemoryAccessFlags srcAccessMask, MemoryAccessFlags dstAccessMask, TextureLayout /*oldLayout*/, TextureLayout /*newLayout*/, const Texture& /*texture*/)
	{
		if (srcAccessMask.Test(MemoryAccess::ShaderWrite))
		{
			GLbitfield barriers = 0;

			if (dstAccessMask.Test(MemoryAccess::ColorRead))
				barriers |= GL_TEXTURE_FETCH_BARRIER_BIT;

			if (dstAccessMask.Test(MemoryAccess::ShaderRead))
				barriers |= GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;

			if (dstAccessMask.Test(MemoryAccess::ShaderWrite))
				barriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;

			if (barriers != 0)
				m_commandBuffer.InsertMemoryBarrier(barriers);
		}
	}
}
