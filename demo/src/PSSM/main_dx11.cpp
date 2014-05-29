#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(PSSMAppUniforms)
	ENTITY_DATA,
	ONFRAME_DATA,
	LIGHTS_DATA,
	DIFFUSE_MAP,
	NORMAL_MAP,
	DEFAULT_SAMPLER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<PSSMAppUniforms>::Uniform

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

// constants
const std::string SHADERS_PATH = "data/shaders/dx11/pssm/";
#define PROFILING 0

// application
class PSSMApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	PSSMApp()
	{
		m_renderDebug = false;
		m_shadowMapSize = 1024;
		m_splitCount = 4;
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
			if (m_splitCount > 16) m_splitCount = 16;
		}

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

		// entity
		m_entity = initEntity("data/media/cube/cube.geom", "data/media/cube/cube_diff.dds", "data/media/cube/cube_normal.dds");
		m_entity.geometry->bindToGpuProgram(m_sceneRendering);

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

		// entity's data buffer
		m_entityDataBuffer.reset(new framework::UniformBuffer());
		if (!m_entityDataBuffer->initDefaultConstant<EntityDataRaw>()) exit();

		// on-frame data buffer
		m_onFrameDataBuffer.reset(new framework::UniformBuffer());
		if (!m_onFrameDataBuffer->initDefaultConstant<OnFrameDataRaw>()) exit();

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
		m_debugLabel = framework::UIFactory::createLabel(gui::Coords(1.0f, -300.0f, 1.0f, -150.0f),
														 gui::Coords::Absolute(300.0f, 150.0f),
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

		if (shadowmap)
		{
			// TODO
		}
		else
		{
			m_sceneRendering->setUniform<PSSMAppUniforms>(UF::ENTITY_DATA, m_entityDataBuffer);
		}

		entity.geometry->renderAllMeshes();
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		//renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);

		//static wchar_t buf[100];
		//swprintf(buf, L"Fragments buffer usage = %d%%\nLost fragments = %d", (int)usage, lostFragments);
		//m_debugLabel->setText(buf);
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
		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			m_entitiesData[i].mvp = m_entitiesData[i].model * vp;
		}

		m_planeData.mvp = m_planeData.model * vp;
	}

private:
	// gpu program to render scene
	std::shared_ptr<framework::GpuProgram> m_sceneRendering;
	// gpu program to render shadow map
	std::shared_ptr<framework::GpuProgram> m_shadowMapRendering;

	// shadow map
	std::shared_ptr<framework::RenderTarget> m_shadowMap;

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

	framework::FreeCamera m_camera;

	int m_shadowMapSize;
	int m_splitCount;

	bool m_renderDebug;
	gui::LabelPtr_T m_debugLabel;
};

DECLARE_MAIN(PSSMApp);