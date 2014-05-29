#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(PSSMAppUniforms)
	ENTITY_DATA,
	ONFRAME_DATA,
	LIGHTS_DATA,
	SHADOW_DATA,
	DIFFUSE_MAP,
	NORMAL_MAP,
	DEFAULT_SAMPLER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<PSSMAppUniforms>::Uniform

// constants
const std::string SHADERS_PATH = "data/shaders/dx11/pssm/";
const int MAX_SPLITS = 8;
#define PROFILING 0

// entity data
#pragma pack (push, 1)
struct EntityDataRaw
{
	matrix44 modelViewProjection;
	matrix44 model;
};
#pragma pack (pop)

// on frame data
#pragma pack (push, 1)
struct OnFrameDataRaw
{
	vector3 viewPosition;
	unsigned int : 32;
};
#pragma pack (pop)

// shadow data
#pragma pack (push, 1)
struct ShadowDataRaw
{
	matrix44 shadowViewProjection[MAX_SPLITS];
};
#pragma pack (pop)

// application
class PSSMApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	PSSMApp()
	{
		m_shadowMapSize = 1024;
		m_splitCount = 4;
		m_splitLambda = 0.5f;

		m_renderDebug = false;
		m_furthestPointInCamera = 0;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Parallel-Split Shadow Mapping (DX11)";

		applyStandardParams(params);
		auto smSize = params.find("shadowmap");
		if (smSize != params.end())
		{
			m_shadowMapSize = smSize->second;
			if (m_shadowMapSize < 0) m_shadowMapSize = 1024;
			if (m_shadowMapSize > 8192) m_shadowMapSize = 8192;
		}
		auto split = params.find("split");
		if (split != params.end())
		{
			m_splitCount = split->second;
			if (m_splitCount < 1) m_splitCount = 1;
			if (m_splitCount > MAX_SPLITS) m_splitCount = MAX_SPLITS;
		}

		m_splitDistances.resize(m_splitCount + 1);

		setLegend("WASD - move camera\nLeft mouse button - rotate camera\nF1 - debug info");

	#if PROFILING
		std::stringstream header;
		header << m_info.title << "\n" << m_info.windowWidth << "x" << m_info.windowHeight 
			<< ", samples = " << m_info.samples << ", fullscreen = " << (m_info.flags.fullscreen ? "yes" : "no") << "\n";
		utils::Profiler::instance().setHeader(header.str());
		utils::Profiler::instance().setFilename("profiler_" + utils::Utils::currentTimeDate(true) + ".txt");
	#endif
	}

	virtual void startup(gui::WidgetPtr_T root)
	{
		// camera
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

		// overlays
		initOverlays(root);

		D3D11_TEXTURE2D_DESC desc = framework::RenderTarget::getDefaultDesc(m_shadowMapSize, m_shadowMapSize, DXGI_FORMAT_R32_FLOAT);
		desc.ArraySize = m_splitCount;
		m_shadowMap.reset(new framework::RenderTarget());
		m_shadowMap->initWithDescription(desc, true);
		if (!m_shadowMap->isValid()) exit();

		// gpu programs
		m_sceneRendering.reset(new framework::GpuProgram());
		m_sceneRendering->addShader(SHADERS_PATH + "scene.vsh.hlsl");
		m_sceneRendering->addShader(SHADERS_PATH + "scene.psh.hlsl");
		if (!m_sceneRendering->init()) exit();
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::ONFRAME_DATA, "onFrameData");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::ENTITY_DATA, "entityData");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::LIGHTS_DATA, "lightsData");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");

		m_shadowMapRendering.reset(new framework::GpuProgram());
		m_shadowMapRendering->addShader(SHADERS_PATH + "shadowmap.vsh.hlsl");
		m_shadowMapRendering->addShader(SHADERS_PATH + "shadowmap.gsh.hlsl");
		m_shadowMapRendering->addShader(SHADERS_PATH + "shadowmap.psh.hlsl");
		if (!m_shadowMapRendering->init()) exit();
		m_shadowMapRendering->bindUniform<PSSMAppUniforms>(UF::SHADOW_DATA, "shadowData");
		m_shadowMapRendering->bindUniform<PSSMAppUniforms>(UF::ENTITY_DATA, "entityData");

		// entity
		m_entity = initEntity("data/media/cube/cube.geom", "data/media/cube/cube_diff.dds", "data/media/cube/cube_normal.dds");
		m_entity.geometry->bindToGpuProgram(m_sceneRendering);
		m_entity.geometry->bindToGpuProgram(m_shadowMapRendering);

		const int ENTITIES_IN_ROW = 6;
		const float HALF_ENTITIES_IN_ROW = float(ENTITIES_IN_ROW) * 0.5f;
		const float AREA_HALFLENGTH = 45.0f;
		m_entitiesData.resize(ENTITIES_IN_ROW * ENTITIES_IN_ROW);
		for (int i = 0; i < ENTITIES_IN_ROW; i++)
		{
			for (int j = 0; j < ENTITIES_IN_ROW; j++)
			{
				int index = i * ENTITIES_IN_ROW + j;
				float x = (float(i) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;
				float z = (float(j) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;

				m_entitiesData[index].model.set_translation(vector3(x * AREA_HALFLENGTH, 5.0f, z * AREA_HALFLENGTH));
			}
		}

		geom::PlaneGenerationInfo planeInfo;
		planeInfo.size = vector2(1000.0f, 1000.0f);
		planeInfo.uvSize = vector2(50.0f, 50.0f);
		m_plane = initEntity(planeInfo, "data/media/textures/grass.dds", "data/media/textures/grass_bump.dds");
		m_plane.geometry->bindToGpuProgram(m_sceneRendering);
		m_planeData.model.ident();

		// rasterizer stage for shadowmap rendering
		m_shadowMapRasterizer.reset(new framework::RasterizerStage());
		D3D11_RASTERIZER_DESC rasterizerDesc = framework::RasterizerStage::getDefault();
		m_shadowMapRasterizer->initWithDescription(rasterizerDesc);
		if (!m_shadowMapRasterizer->isValid()) exit();
		m_shadowMapRasterizer->getViewports().resize(1);
		m_shadowMapRasterizer->getViewports()[0] = framework::RasterizerStage::getDefaultViewport(m_shadowMapSize, m_shadowMapSize);

		// entity's data buffer
		m_entityDataBuffer.reset(new framework::UniformBuffer());
		if (!m_entityDataBuffer->initDefaultConstant<EntityDataRaw>()) exit();

		// on-frame data buffer
		m_onFrameDataBuffer.reset(new framework::UniformBuffer());
		if (!m_onFrameDataBuffer->initDefaultConstant<OnFrameDataRaw>()) exit();

		// shadow buffer
		m_shadowBuffer.reset(new framework::UniformBuffer());
		if (!m_shadowBuffer->initDefaultConstant<ShadowDataRaw>()) exit();

		// lights
		initLights();
	}

	Entity initEntity(const geom::PlaneGenerationInfo& planeInfo, const std::string& diffuseMap, const std::string& normalMap)
	{
		Entity ent;

		ent.geometry.reset(new framework::Geometry3D());
		if (!ent.geometry->initAsPlane(planeInfo)) exit();

		ent.diffuseTexture.reset(new framework::Texture());
		if (!ent.diffuseTexture->initWithDDS(diffuseMap)) exit();

		ent.normalTexture.reset(new framework::Texture());
		if (!ent.normalTexture->initWithDDS(normalMap)) exit();

		return std::move(ent);
	}

	Entity initEntity(const std::string& geometry, const std::string& diffuseMap, const std::string& normalMap)
	{
		Entity ent;

		ent.geometry.reset(new framework::Geometry3D());
		if (!ent.geometry->init(geometry)) exit();

		ent.diffuseTexture.reset(new framework::Texture());
		if (!ent.diffuseTexture->initWithDDS(diffuseMap)) exit();

		ent.normalTexture.reset(new framework::Texture());
		if (!ent.normalTexture->initWithDDS(normalMap)) exit();

		return std::move(ent);
	}

	void initLights()
	{
		// directional light
		framework::LightSource source;
		source.type = framework::LightType::DirectLight;
		source.position = vector3(0, 15, 0);
		vector3 dir(1, -1, 1);
		dir.norm();
		source.orientation.set_from_axes(vector3(0, 0, 1), dir);
		source.diffuseColor = vector3(1.0f, 1.0f, 1.0f);
		source.specularColor = vector3(0.5f, 0.5f, 0.5f);
		source.ambientColor = vector3(0.3f, 0.3f, 0.3f);
		m_lightManager.addLightSource(source);

		// light buffer
		m_lightsBuffer.reset(new framework::UniformBuffer());
		if (!m_lightsBuffer->initDefaultConstant<framework::LightRawData>()) exit();
		m_lightsBuffer->setData(m_lightManager.getRawLightData(0));
		m_lightsBuffer->applyChanges();
	}

	void initOverlays(gui::WidgetPtr_T root)
	{
		m_debugLabel = framework::UIFactory::createLabel(gui::Coords(1.0f, -500.0f, 1.0f, -300.0f),
														 gui::Coords::Absolute(500.0f, 300.0f),
														 gui::RightAligned, gui::BottomAligned);
		root->addChild(m_debugLabel);

		m_debugLabel->setVisible(m_renderDebug);
	}

	virtual void shutdown()
	{
	#if PROFILING
		if (utils::Profiler::instance().isRun())
		{
			utils::Profiler::instance().stop();
		}
		utils::Profiler::instance().saveToFile();
	#endif
	}

	virtual void render(double elapsedTime)
	{
		m_camera.update(elapsedTime);
		update(elapsedTime);

		// render shadow map
		getPipeline().clearRenderTarget(m_shadowMap);
		getPipeline().setRenderTarget(m_shadowMap);
		if (m_shadowMapRendering->use())
		{
			m_shadowMapRendering->setUniform<PSSMAppUniforms>(UF::SHADOW_DATA, m_shadowBuffer);

			m_shadowMapRasterizer->apply();
			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderEntity(true, m_entity, m_entitiesData[i]);
			}
			renderEntity(true, m_plane, m_planeData);
			m_shadowMapRasterizer->cancel();
		}

		// set up on-frame data
		OnFrameDataRaw onFrameData;
		onFrameData.viewPosition = m_camera.getPosition();
		m_onFrameDataBuffer->setData(onFrameData);
		m_onFrameDataBuffer->applyChanges();

		useDefaultRenderTarget();

		// render scene
		if (m_sceneRendering->use())
		{
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::ONFRAME_DATA, m_onFrameDataBuffer);
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, m_entity.diffuseTexture);
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::NORMAL_MAP, m_entity.normalTexture);
			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderEntity(false, m_entity, m_entitiesData[i]);
			}

			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, m_plane.diffuseTexture);
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::NORMAL_MAP, m_plane.normalTexture);
			renderEntity(false, m_plane, m_planeData);
		}

		renderDebug();
	}

	void renderEntity(bool shadowmap, const Entity& entity, const EntityData& entityData)
	{
		EntityDataRaw entityDataRaw;
		entityDataRaw.modelViewProjection = entityData.mvp;
		entityDataRaw.model = entityData.model;
		m_entityDataBuffer->setData(entityDataRaw);
		m_entityDataBuffer->applyChanges();

		m_sceneRendering->setUniform<PSSMAppUniforms>(UF::ENTITY_DATA, m_entityDataBuffer);

		entity.geometry->renderAllMeshes(shadowmap ? m_splitCount : 1);
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		//renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);

		std::wstringstream stream;
		stream.precision(2);
		stream << "Furthest point = " << std::fixed << m_furthestPointInCamera << "\nSplit lambda = " << std::fixed << m_splitLambda << "\nDistances = ";
		for (size_t i = 0; i < m_splitDistances.size(); i++)
		{
			stream << std::fixed << m_splitDistances[i];
			if (i != m_splitDistances.size() - 1) 
			{	
				if (i % 2 == 0 && i != 0) stream << ",\n"; else stream << ", ";
			}
		}

		m_debugLabel->setText(stream.str());
	}

	virtual void onKeyButton(int key, int scancode, bool pressed)
	{
		if (key == InputKeys::F1 && pressed)
		{
			m_renderDebug = !m_renderDebug;
			m_debugLabel->setVisible(m_renderDebug);
			return;
		}
	#if PROFILING
		if (key == InputKeys::P && pressed)
		{
			if (!utils::Profiler::instance().isRun())
				utils::Profiler::instance().run();
			else
				utils::Profiler::instance().stop();
			return;
		}
	#endif
		m_camera.onKeyButton(key, scancode, pressed);
	}

	virtual void onMouseButton(double xpos, double ypos, int button, bool pressed)
	{
		m_camera.onMouseButton(xpos, ypos, button, pressed);
	}

	virtual void onMouseMove(double xpos, double ypos)
	{
		m_camera.onMouseMove(xpos, ypos);
	}

	void update(double elapsedTime)
	{
		matrix44 cameraView = m_camera.getView();
		matrix44 cameraProjection = m_camera.getProjection();
		matrix44 vp = cameraView * cameraProjection;
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			m_entitiesData[i].mvp = m_entitiesData[i].model * vp;
		}

		m_planeData.mvp = m_planeData.model * vp;

		// calculate shadow
		matrix44 shadowView = calculateShadowView();
		
		m_furthestPointInCamera = calculateFurthestPointInCamera(cameraView);
		calculateSplitDistances();

		ShadowDataRaw shadowData;
		m_shadowBuffer->setData(shadowData);
		m_shadowBuffer->applyChanges();
	}

	matrix44 calculateShadowView()
	{
		const float LIGHT_SOURCE_DIST = 50.0f;

		auto lightSource = m_lightManager.getLightSource(0);
		vector3 dir = lightSource.orientation.z_direction();
		
		matrix44 shadowView;
		shadowView.pos_component() = m_camera.getPosition() - dir * LIGHT_SOURCE_DIST;
		shadowView.lookatRh(shadowView.pos_component() + dir, lightSource.orientation.y_direction());
		shadowView.invert_simple();

		return shadowView;
	}

	float calculateFurthestPointInCamera(const matrix44& cameraView)
	{
		bbox3 scenebox;
		scenebox.begin_extend();
		bbox3 b = m_plane.geometry->getBoundingBox();
		b.transform(m_planeData.model);
		scenebox.extend(b);
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			bbox3 b = m_entity.geometry->getBoundingBox();
			b.transform(m_entitiesData[i].model);
			scenebox.extend(b);
		}
		scenebox.end_extend();

		float maxZ = m_camera.getInternalCamera().GetNearPlane();
		for(int i = 0; i < 8; i++)
		{
			vector3 corner = scenebox.corner_point(i);
			float z = -cameraView.transform_coord(corner).z;
			if(z > maxZ) maxZ = z;
		}
		return std::min(maxZ, m_camera.getInternalCamera().GetFarPlane());
	}

	void calculateSplitDistances()
	{
		float nearPlane = m_camera.getInternalCamera().GetNearPlane();
		for(int i = 0; i < m_splitCount; i++)
		{
			float f = (float)i / (float)m_splitCount;
			float l = nearPlane * pow(m_furthestPointInCamera / nearPlane, f);
			float u = nearPlane + (m_furthestPointInCamera - nearPlane) * f;
			m_splitDistances[i] = l * m_splitLambda + u * (1.0f - m_splitLambda);
		}

		m_splitDistances[0] = nearPlane;
		m_splitDistances[m_splitCount] = m_furthestPointInCamera;
	}

private:
	// gpu program to render scene
	std::shared_ptr<framework::GpuProgram> m_sceneRendering;
	// gpu program to render shadow map
	std::shared_ptr<framework::GpuProgram> m_shadowMapRendering;

	// shadow map
	std::shared_ptr<framework::RenderTarget> m_shadowMap;

	std::shared_ptr<framework::RasterizerStage> m_shadowMapRasterizer;

	// entity
	struct Entity
	{
		std::shared_ptr<framework::Geometry3D> geometry;
		std::shared_ptr<framework::Texture> diffuseTexture;
		std::shared_ptr<framework::Texture> normalTexture;
	};

	struct EntityData
	{
		matrix44 model;
		matrix44 mvp;
	};
	Entity m_entity;
	std::vector<EntityData> m_entitiesData;

	Entity m_plane;
	EntityData m_planeData;

	std::shared_ptr<framework::UniformBuffer> m_entityDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_onFrameDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;
	std::shared_ptr<framework::UniformBuffer> m_shadowBuffer;

	framework::FreeCamera m_camera;

	int m_shadowMapSize;
	int m_splitCount;
	float m_splitLambda;
	std::vector<float> m_splitDistances;

	bool m_renderDebug;
	gui::LabelPtr_T m_debugLabel;
	float m_furthestPointInCamera;
};

DECLARE_MAIN(PSSMApp);