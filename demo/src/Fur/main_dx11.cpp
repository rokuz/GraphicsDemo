#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(FurAppUniforms)
	ENTITY_DATA,
	ONFRAME_DATA,
	LIGHTS_DATA,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	DEFAULT_SAMPLER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<FurAppUniforms>::Uniform

// constants
const std::string SHADERS_PATH = "data/shaders/dx11/fur/";
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

// application
class FurApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	FurApp()
	{
		m_renderDebug = false;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Fur rendering (DX11)";
		applyStandardParams(params);

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
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(-1, 2, -3), vector3());

		// overlays
		initOverlays(root);

		// gpu programs
		m_solidRendering.reset(new framework::GpuProgram());
		m_solidRendering->addShader(SHADERS_PATH + "solid.vsh.hlsl");
		m_solidRendering->addShader(SHADERS_PATH + "solid.psh.hlsl");
		if (!m_solidRendering->init()) exit();
		m_solidRendering->bindUniform<FurAppUniforms>(UF::ONFRAME_DATA, "onFrameData");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::ENTITY_DATA, "entityData");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::LIGHTS_DATA, "lightsData");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::SPECULAR_MAP, "specularMap");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");

		// geometry
		m_catGeometry = initEntity("data/media/cat/cat.geom");
		m_catGeometry->bindToGpuProgram(m_solidRendering);

		// entities
		/*vector3 GROUP_OFFSET[] = { vector3(0, 0, 0), vector3(-1000, 130, 600), vector3(0, -4, 900) };
		for (int k = 0; k < sizeof(GROUP_OFFSET) / sizeof(GROUP_OFFSET[0]); k++)
		{
			const int ENTITIES_IN_ROW = 2;
			const float HALF_ENTITIES_IN_ROW = float(ENTITIES_IN_ROW) * 0.5f;
			const float AREA_HALFLENGTH = 150.0f;

			for (int i = 0; i < ENTITIES_IN_ROW; i++)
			{
				for (int j = 0; j < ENTITIES_IN_ROW; j++)
				{
					float x = (float(i) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;
					float z = (float(j) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;

					m_entitiesData.push_back(EntityData());
					int index = (int)m_entitiesData.size() - 1;
					m_entitiesData[index].geometry = ((i == 1 && j == 1) ? m_windmillGeometry : m_houseGeometry);
					m_entitiesData[index].model.set_translation(vector3(x * AREA_HALFLENGTH, 5.0f, z * AREA_HALFLENGTH));
					m_entitiesData[index].model.pos_component() += GROUP_OFFSET[k];
				}
			}
		}*/

		EntityData terrainData;
		terrainData.geometry = m_catGeometry;
		quaternion quat;
		quat.set_rotate_y(n_deg2rad(180.0f));
		terrainData.model = matrix44(quat);
		m_entitiesData.push_back(terrainData);

		// entity's data buffer
		m_entityDataBuffer.reset(new framework::UniformBuffer());
		if (!m_entityDataBuffer->initDefaultConstant<EntityDataRaw>()) exit();

		// on-frame data buffer
		m_onFrameDataBuffer.reset(new framework::UniformBuffer());
		if (!m_onFrameDataBuffer->initDefaultConstant<OnFrameDataRaw>()) exit();

		// lights
		initLights();

		// skybox texture
		m_skyboxTexture.reset(new framework::Texture());
		if (!m_skyboxTexture->initWithDDS("data/media/textures/meadow.dds")) exit();
	}

	std::shared_ptr<framework::Geometry3D> initEntity(const geom::TerrainGenerationInfo& terrainInfo, const std::string& diffuseMap, const std::string& normalMap, const std::string& specularMap)
	{
		std::shared_ptr<framework::Geometry3D> ent(new framework::Geometry3D());
		if (!ent->initAsTerrain(terrainInfo))
		{
			exit();
		}
		else
		{
			framework::MaterialManager::instance().initializeMaterial(ent, diffuseMap, normalMap, specularMap);
		}

		return std::move(ent);
	}

	std::shared_ptr<framework::Geometry3D> initEntity(const std::string& geometry)
	{
		std::shared_ptr<framework::Geometry3D> ent(new framework::Geometry3D());
		if (!ent->init(geometry))
		{
			exit();
		}
		else
		{
			framework::MaterialManager::instance().initializeMaterial(ent);
		}

		return std::move(ent);
	}

	void initLights()
	{
		// directional light
		framework::LightSource source;
		source.type = framework::LightType::DirectLight;
		source.position = vector3(0, 5, 0);
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

		m_fpsLabel->setColor(vector4(0.0f, 0.5f, 1.0f, 1.0f));
		m_legendLabel->setColor(vector4(0.0f, 0.5f, 1.0f, 1.0f));
		m_debugLabel->setColor(vector4(0.0f, 0.5f, 1.0f, 1.0f));
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

		// render skybox
		renderSkybox(m_camera, m_skyboxTexture);

		// render scene
		if (m_solidRendering->use())
		{
			m_solidRendering->setUniform<FurAppUniforms>(UF::ONFRAME_DATA, m_onFrameDataBuffer);
			m_solidRendering->setUniform<FurAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderGeometry(m_entitiesData[i].geometry.lock(), m_entitiesData[i]);
			}
		}

		renderDebug();
	}

	void renderGeometry(const std::shared_ptr<framework::Geometry3D>& geometry, const EntityData& entityData)
	{
		EntityDataRaw entityDataRaw;
		entityDataRaw.modelViewProjection = entityData.mvp;
		entityDataRaw.model = entityData.model;
		m_entityDataBuffer->setData(entityDataRaw);
		m_entityDataBuffer->applyChanges();

		for (size_t i = 0; i < geometry->getMeshesCount(); i++)
		{
			auto diffMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_DIFFUSE_MAP);
			auto normMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_NORMAL_MAP);
			auto specMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_SPECULAR_MAP);
			m_solidRendering->setUniform<FurAppUniforms>(UF::DIFFUSE_MAP, diffMap);
			m_solidRendering->setUniform<FurAppUniforms>(UF::NORMAL_MAP, normMap);
			m_solidRendering->setUniform<FurAppUniforms>(UF::SPECULAR_MAP, specMap);
			m_solidRendering->setUniform<FurAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

			m_solidRendering->setUniform<FurAppUniforms>(UF::ENTITY_DATA, m_entityDataBuffer);
			
			geometry->renderMesh(i);
		}	
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);

		std::wstringstream stream;
		stream.precision(2);
		stream << "Debug info";
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
	}

private:
	// gpu program to render solid objects in scene
	std::shared_ptr<framework::GpuProgram> m_solidRendering;

	std::shared_ptr<framework::Geometry3D> m_catGeometry;

	struct EntityData
	{
		std::weak_ptr<framework::Geometry3D> geometry;
		matrix44 model;
		matrix44 mvp;

		EntityData(){}
	};

	std::vector<EntityData> m_entitiesData;

	std::shared_ptr<framework::UniformBuffer> m_entityDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_onFrameDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;

	framework::FreeCamera m_camera;

	bool m_renderDebug;
	gui::LabelPtr_T m_debugLabel;

	std::shared_ptr<framework::Texture> m_skyboxTexture;
};

DECLARE_MAIN(FurApp);