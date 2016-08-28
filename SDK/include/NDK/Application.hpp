// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Development Kit"
// For conditions of distribution and use, see copyright notice in Prerequesites.hpp

#pragma once

#ifndef NDK_APPLICATION_HPP
#define NDK_APPLICATION_HPP

#include <NDK/Prerequesites.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <Nazara/Core/Clock.hpp>
#include <list>
#include <set>
#include <vector>

#ifndef NDK_SERVER
#include <NDK/Console.hpp>
#include <Nazara/Core/Log.hpp>
#include <Nazara/Lua/LuaInstance.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <Nazara/Utility/Window.hpp>
#endif

namespace Ndk
{
	class NDK_API Application
	{
		public:
			#ifndef NDK_SERVER
			struct ConsoleOverlay;
			struct FPSCounterOverlay;
			#endif

			inline Application();
			inline Application(int argc, const char* argv[]);
			Application(const Application&) = delete;
			Application(Application&&) = delete;
			inline ~Application();

			#ifndef NDK_SERVER
			template<typename T, typename... Args> T& AddWindow(Args&&... args);
			#endif
			template<typename... Args> World& AddWorld(Args&&... args);

			#ifndef NDK_SERVER
			inline void EnableConsole(bool enable);
			inline void EnableFPSCounter(bool enable);

			inline ConsoleOverlay& GetConsoleOverlay(std::size_t windowIndex = 0U);
			inline FPSCounterOverlay& GetFPSCounterOverlay(std::size_t windowIndex = 0U);
			#endif

			inline float GetUpdateTime() const;

			#ifndef NDK_SERVER
			inline bool IsConsoleEnabled() const;
			inline bool IsFPSCounterEnabled() const;
			#endif

			bool Run();

			#ifndef NDK_SERVER
			inline void MakeExitOnLastWindowClosed(bool exitOnClosedWindows);
			#endif

			inline void Quit();

			Application& operator=(const Application&) = delete;
			Application& operator=(Application&&) = delete;

			inline static Application* Instance();

			#ifndef NDK_SERVER
			struct ConsoleOverlay
			{
				std::unique_ptr<Console> console;
				Nz::LuaInstance lua;

				NazaraSlot(Nz::EventHandler, OnEvent, eventSlot);
				NazaraSlot(Nz::EventHandler, OnKeyPressed, keyPressedSlot);
				NazaraSlot(Nz::EventHandler, OnResized, resizedSlot);
				NazaraSlot(Nz::Log, OnLogWrite, logSlot);
			};

			struct FPSCounterOverlay
			{
				Nz::TextSpriteRef sprite;
				EntityOwner entity;
				float elapsedTime = 0.f;
				unsigned int frameCount = 0;
			};
			#endif

		private:
			#ifndef NDK_SERVER
			enum OverlayFlags
			{
				OverlayFlags_Console    = 0x1,
				OverlayFlags_FPSCounter = 0x2
			};

			struct WindowInfo
			{
				inline WindowInfo(std::unique_ptr<Nz::Window>&& window);

				Nz::RenderTarget* renderTarget;
				std::unique_ptr<Nz::Window> window;
				std::unique_ptr<ConsoleOverlay> console;
				std::unique_ptr<FPSCounterOverlay> fpsCounter;
				std::unique_ptr<World> overlayWorld;
			};
			
			void SetupConsole(WindowInfo& info);
			void SetupFPSCounter(WindowInfo& info);
			void SetupOverlay(WindowInfo& info);

			std::vector<WindowInfo> m_windows;
			#endif
			std::list<World> m_worlds;
			Nz::Clock m_updateClock;
			#ifndef NDK_SERVER
			Nz::UInt32 m_overlayFlags;
			bool m_exitOnClosedWindows;
			#endif
			bool m_shouldQuit;
			float m_updateTime;

			static Application* s_application;
	};
}

#include <NDK/Application.inl>

#endif // NDK_APPLICATION_HPP
