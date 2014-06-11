set(SOURCE_LIB  helsing/hmath.hpp
		helsing/HeightMap.hpp
		helsing/math/hmath_misc.hpp
		helsing/math/Mat4.hpp
		helsing/math/Vec2.hpp
		helsing/math/Vec4.hpp
		ptg/Continuous2DSignal.hpp
		ptg/Continuous2DSignalTerrain.hpp
		ptg/DiamondSquareTerrain.hpp
		ptg/FractionalBrownianMotion.hpp
		ptg/GaussianBlurTerrain.hpp
		ptg/HelsingSquareTerrain.hpp
		ptg/HybridMultiFractal.hpp
		ptg/MidpointDisplacementTerrain.hpp
		ptg/PerlinNoise.hpp
		ptg/RidgedMultiFractal.hpp
		ptg/Seedable.hpp

		ptg/Terrain.hpp

		ptg/ThermalErosionTerrain.hpp
		ptg/ValueNoise.hpp
		helsing/HeightMap.cpp
		ptg/Continuous2DSignal.cpp

		ptg/Continuous2DSignalTerrain.cpp

		ptg/DiamondSquareTerrain.cpp

		ptg/FractionalBrownianMotion.cpp

		ptg/GaussianBlurTerrain.cpp

		ptg/HelsingSquareTerrain.cpp
		ptg/HybridMultiFractal.cpp

		ptg/MidpointDisplacementTerrain.cpp

		ptg/PerlinNoise.cpp
		ptg/RidgedMultiFractal.cpp

		ptg/Seedable.cpp

		ptg/ThermalErosionTerrain.cpp

		ptg/ValueNoise.cpp
)

add_library(ptglib STATIC ${SOURCE_LIB})
