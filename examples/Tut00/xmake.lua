target("Tut00_EmptyProject")
	add_packages("entt")
	add_defines("NAZARA_ENTT")
	add_deps("NazaraAudio", "NazaraGraphics", "NazaraNetwork", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraRenderer", "NazaraTextRenderer", "NazaraWidgets")
	add_files("main.cpp")
