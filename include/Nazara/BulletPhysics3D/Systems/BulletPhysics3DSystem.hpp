// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Engine - BulletPhysics3D module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_BULLETPHYSICS3D_SYSTEMS_BULLETPHYSICS3DSYSTEM_HPP
#define NAZARA_BULLETPHYSICS3D_SYSTEMS_BULLETPHYSICS3DSYSTEM_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/BulletPhysics3D/BulletPhysWorld3D.hpp>
#include <Nazara/BulletPhysics3D/Components/BulletRigidBody3DComponent.hpp>
#include <Nazara/Core/Time.hpp>
#include <Nazara/Utils/TypeList.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace Nz
{
	class NAZARA_BULLETPHYSICS3D_API BulletPhysics3DSystem
	{
		public:
			static constexpr Int64 ExecutionOrder = 0;
			using Components = TypeList<BulletRigidBody3DComponent, class NodeComponent>;

			struct RaycastHit;

			BulletPhysics3DSystem(entt::registry& registry);
			BulletPhysics3DSystem(const BulletPhysics3DSystem&) = delete;
			BulletPhysics3DSystem(BulletPhysics3DSystem&&) = delete;
			~BulletPhysics3DSystem();

			template<typename... Args> BulletRigidBody3DComponent CreateRigidBody(Args&&... args);

			inline BulletPhysWorld3D& GetPhysWorld();
			inline const BulletPhysWorld3D& GetPhysWorld() const;

			bool RaycastQueryFirst(const Vector3f& from, const Vector3f& to, RaycastHit* hitInfo = nullptr);

			void Update(Time elapsedTime);

			BulletPhysics3DSystem& operator=(const BulletPhysics3DSystem&) = delete;
			BulletPhysics3DSystem& operator=(BulletPhysics3DSystem&&) = delete;

			struct RaycastHit : BulletPhysWorld3D::RaycastHit
			{
				entt::handle hitEntity;
			};

		private:
			void OnConstruct(entt::registry& registry, entt::entity entity);
			void OnDestruct(entt::registry& registry, entt::entity entity);

			std::vector<entt::entity> m_physicsEntities;
			entt::registry& m_registry;
			entt::observer m_physicsConstructObserver;
			entt::scoped_connection m_constructConnection;
			entt::scoped_connection m_destructConnection;
			BulletPhysWorld3D m_physWorld;
	};
}

#include <Nazara/BulletPhysics3D/Systems/BulletPhysics3DSystem.inl>

#endif // NAZARA_BULLETPHYSICS3D_SYSTEMS_BULLETPHYSICS3DSYSTEM_HPP
