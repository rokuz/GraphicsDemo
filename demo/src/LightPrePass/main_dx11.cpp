#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(LPPAppUniforms)
	ENTITY_DATA,
	ONFRAME_DATA,
	LIGHTS_DATA,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	DEFAULT_SAMPLER,
	GBUFFER_MAP,
	LBUFFER_MAP,
	SBUFFER_MAP
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<LPPAppUniforms>::Uniform

// entity data
#pragma pack (push, 1)
struct EntityDataRaw
{
	matrix44 modelViewProjection;
	matrix44 model;
	matrix44 modelView;
	unsigned int materialId;
	unsigned int : 32;
	unsigned int : 32;
	unsigned int : 32;
};
#pragma pack (pop)

// on frame data
#pragma pack (push, 1)
struct OnFrameDataRaw
{
	vector3 viewPosition;
	unsigned int lightsCount;
	matrix44 viewInverse;
	matrix44 projectionInverse;
	unsigned int samplesCount;
	unsigned int : 32;
	unsigned int : 32;
	unsigned int : 32;
};
#pragma pack (pop)

// constants
const int MAX_LIGHTS_COUNT = 64;
const std::string SHADERS_PATH = "data/shaders/dx11/lpp/";

// application
class LightPrePassApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	LightPrePassApp()
	{
		m_lightsCount = 0;
		m_pause = false;
		m_renderDebug = false;
		m_additionalLights = 50;
		m_visibleObjects = 0;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Light-Prepass Deferred shading (DX11)";

		applyStandardParams(params);

		auto lights = params.find("lights");
		if (lights != params.end())
		{
			m_additionalLights = lights->second;
			if (m_additionalLights < 0) m_additionalLights = 0;
			if (m_additionalLights > MAX_LIGHTS_COUNT - 1) m_additionalLights = MAX_LIGHTS_COUNT - 1;
		}

		setLegend("WASD - move camera\nLeft mouse button - rotate camera\nF1 - debug info");
	}

	virtual void startup(gui::WidgetPtr_T root)
	{
		// camera
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

		// overlays
		initOverlays(root);

		// buffers
		D3D11_TEXTURE2D_DESC desc = framework::RenderTarget::getDefaultDesc(m_info.windowWidth, m_info.windowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
		auto defDesc = defaultRenderTarget()->getDesc(0);
		desc.SampleDesc.Count = defDesc.SampleDesc.Count;
		desc.SampleDesc.Quality = defDesc.SampleDesc.Quality;
		m_gbuffer.reset(new framework::RenderTarget());
		m_gbuffer->initWithDescription(desc, false);
		if (!m_gbuffer->isValid()) exit();

		std::vector<D3D11_TEXTURE2D_DESC> descs;
		descs.push_back(desc);
		descs.push_back(desc);
		m_lbuffer.reset(new framework::RenderTarget());
		m_lbuffer->initWithDescriptions(descs, false);
		if (!m_lbuffer->isValid()) exit();

		// gpu programs
		m_gbufferRendering.reset(new framework::GpuProgram());
		m_gbufferRendering->addShader(SHADERS_PATH + "gbuffer.vsh.hlsl");
		m_gbufferRendering->addShader(SHADERS_PATH + "gbuffer.psh.hlsl");
		if (!m_gbufferRendering->init()) exit();
		m_gbufferRendering->bindUniform<LPPAppUniforms>(UF::ENTITY_DATA, "entityData");
		m_gbufferRendering->bindUniform<LPPAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_gbufferRendering->bindUniform<LPPAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");

		m_lightPrePass.reset(new framework::GpuProgram());
		m_lightPrePass->addShader(SHADERS_PATH + "lightprepass.vsh.hlsl");
		m_lightPrePass->addShader(SHADERS_PATH + "lightprepass.gsh.hlsl");
		if (m_info.samples == 0)
		{
			m_lightPrePass->addShader(SHADERS_PATH + "lightprepass.psh.hlsl");
		}
		else
		{
			m_lightPrePass->addShader(SHADERS_PATH + "lightprepass_msaa.psh.hlsl");
		}
		if (!m_lightPrePass->init(true)) exit();
		m_lightPrePass->bindUniform<LPPAppUniforms>(UF::LIGHTS_DATA, "lightsData");
		m_lightPrePass->bindUniform<LPPAppUniforms>(UF::ONFRAME_DATA, "onFrameData");
		m_lightPrePass->bindUniform<LPPAppUniforms>(UF::GBUFFER_MAP, "gbufferMap");

		m_sceneRendering.reset(new framework::GpuProgram());
		m_sceneRendering->addShader(SHADERS_PATH + "scene.vsh.hlsl");
		if (m_info.samples == 0)
		{
			m_sceneRendering->addShader(SHADERS_PATH + "scene.psh.hlsl");
		}
		else
		{
			m_sceneRendering->addShader(SHADERS_PATH + "scene_msaa.psh.hlsl");
		}
		if (!m_sceneRendering->init()) exit();
		m_sceneRendering->bindUniform<LPPAppUniforms>(UF::ENTITY_DATA, "entityData");
		m_sceneRendering->bindUniform<LPPAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_sceneRendering->bindUniform<LPPAppUniforms>(UF::SPECULAR_MAP, "specularMap");
		m_sceneRendering->bindUniform<LPPAppUniforms>(UF::LBUFFER_MAP, "lbufferMap");
		m_sceneRendering->bindUniform<LPPAppUniforms>(UF::SBUFFER_MAP, "sbufferMap");
		m_sceneRendering->bindUniform<LPPAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");

		m_depthPrePass.reset(new framework::DepthStencilStage());
		D3D11_DEPTH_STENCIL_DESC depthDesc = framework::DepthStencilStage::getDisableDepthWriting();
		depthDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
		m_depthPrePass->initWithDescription(depthDesc);
		if (!m_depthPrePass->isValid()) exit();

		// entity
		m_entity = initEntity("data/media/cube/cube.geom",
							  "data/media/cube/cube_diff.dds",
							  "data/media/cube/cube_normal.dds",
							  "data/media/textures/full_specular.dds");
		m_entity.geometry->bindToGpuProgram(m_gbufferRendering);
		m_entity.geometry->bindToGpuProgram(m_sceneRendering);

		m_entitiesData.resize(25);
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			m_entitiesData[i].model.set_translation(utils::Utils::random(-70.0f, 70.0f));
		}

		// skybox texture
		m_skyboxTexture.reset(new framework::Texture());
		if (!m_skyboxTexture->initWithDDS("data/media/textures/nightsky2.dds")) exit();

		// entity's data buffer
		m_entityDataBuffer.reset(new framework::UniformBuffer());
		if (!m_entityDataBuffer->initDefaultConstant<EntityDataRaw>()) exit();

		// on-frame data buffer
		m_onFrameDataBuffer.reset(new framework::UniformBuffer());
		if (!m_onFrameDataBuffer->initDefaultConstant<OnFrameDataRaw>()) exit();

		// lights
		initLights();
	}

	Entity initEntity(const std::string& geometry, 
					  const std::string& texture, 
					  const std::string& normalTexture, 
					  const std::string& specularTexture)
	{
		Entity ent;

		// geometry
		ent.geometry.reset(new framework::Geometry3D());
		if (!ent.geometry->init(geometry)) exit();

		// textures
		if (!texture.empty())
		{
			ent.texture.reset(new framework::Texture());
			ent.texture->initWithDDS(texture);
		}

		if (!normalTexture.empty())
		{
			ent.normalTexture.reset(new framework::Texture());
			ent.normalTexture->initWithDDS(normalTexture);
		}

		if (!specularTexture.empty())
		{
			ent.specularTexture.reset(new framework::Texture());
			ent.specularTexture->initWithDDS(specularTexture);
		}

		return std::move(ent);
	}

	void initLights()
	{
		// directional light
		framework::LightSource source;
		source.type = framework::LightType::DirectLight;
		source.position = vector3(-11, 11, -1);
		vector3 dir(1, -1, 1);
		dir.norm();
		source.orientation.set_from_axes(vector3(0, 0, 1), dir);
		source.diffuseColor = vector3(1.0f, 1.0f, 1.0f);
		source.specularColor = vector3(0.5f, 0.5f, 0.5f);
		source.ambientColor = vector3(0.3f, 0.3f, 0.3f);
		m_lightManager.addLightSource(source);

		// omni light
		framework::LightSource source2;
		source2.type = framework::LightType::OmniLight;
		source2.position = vector3(15, 15, 0);
		source2.diffuseColor = vector3(0.96f, 0.81f, 0.59f);
		source2.specularColor = vector3(0.1f, 0.1f, 0.1f);
		source2.ambientColor = vector3(0.0f, 0.0f, 0.0f);
		source2.falloff = 40.0f;

		// additional lights
		for (int i = 0; i < m_additionalLights; i++)
		{
			framework::LightSource sourceN = source2;
			sourceN.position = utils::Utils::random(-70.0f, 70.0f);
			m_lightManager.addLightSource(sourceN);
		}

		// light buffer
		m_lightsBuffer.reset(new framework::UniformBuffer());
		if (!m_lightsBuffer->initDefaultStructured<framework::LightRawData>((size_t)MAX_LIGHTS_COUNT)) exit();

		m_lightsCount = std::min((int)m_lightManager.getLightSourcesCount(), MAX_LIGHTS_COUNT);
		for (unsigned int i = 0; i < m_lightsCount; i++)
		{
			m_lightsBuffer->setElement((int)i, m_lightManager.getRawLightData(i));
		}
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
		if (utils::Profiler::instance().isRun())
		{
			utils::Profiler::instance().stop();
		}
		utils::Profiler::instance().saveToFile();
	}

	virtual void render(double elapsedTime)
	{
		TRACE_FUNCTION
		m_camera.update(elapsedTime);
		update(elapsedTime);

		// set up on-frame data
		OnFrameDataRaw onFrameData;
		onFrameData.viewPosition = m_camera.getPosition();
		onFrameData.lightsCount = m_lightsCount;
		matrix44 view = m_camera.getView();
		view.invert();
		matrix44 proj = m_camera.getProjection();
		proj.invert();
		onFrameData.viewInverse = view;
		onFrameData.projectionInverse = proj;
		onFrameData.samplesCount = m_info.samples;
		m_onFrameDataBuffer->setData(onFrameData);
		m_onFrameDataBuffer->applyChanges();

		getPipeline().clearRenderTarget(m_gbuffer);
		getPipeline().setRenderTarget(m_gbuffer, defaultRenderTarget());

		// render to g-buffer
		if (m_gbufferRendering->use())
		{
			TRACE_BLOCK("_GBuffer")

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderEntity(true, m_entity, m_entitiesData[i]);
			}
		}

		getPipeline().clearRenderTarget(m_lbuffer);
		getPipeline().setRenderTarget(m_lbuffer);

		// light pre-pass
		if (m_lightPrePass->use())
		{
			TRACE_BLOCK("_LightPrePass")
			m_lightPrePass->setUniform<LPPAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);
			m_lightPrePass->setUniform<LPPAppUniforms>(UF::ONFRAME_DATA, m_onFrameDataBuffer);
			m_lightPrePass->setUniform<LPPAppUniforms>(UF::GBUFFER_MAP, m_gbuffer);

			disableDepthTest()->apply();
			getPipeline().drawPoints(1);
			disableDepthTest()->cancel();
		}

		useDefaultRenderTarget();

		// render skybox
		renderSkybox(m_camera, m_skyboxTexture);

		// render scene
		if (m_sceneRendering->use())
		{
			TRACE_BLOCK("_SceneRendering")

			m_sceneRendering->setUniform<LPPAppUniforms>(UF::LBUFFER_MAP, m_lbuffer, 0);
			m_sceneRendering->setUniform<LPPAppUniforms>(UF::SBUFFER_MAP, m_lbuffer, 1);

			m_depthPrePass->apply();
			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderEntity(false, m_entity, m_entitiesData[i]);
			}
			m_depthPrePass->cancel();
		}

		// debug rendering
		renderDebug();
	}

	void renderEntity(bool gbuffer, const Entity& entity, const EntityData& entityData)
	{
		if (!entityData.isVisible) return;

		EntityDataRaw entityDataRaw;
		entityDataRaw.modelViewProjection = entityData.mvp;
		entityDataRaw.model = entityData.model;
		entityDataRaw.modelView = entityData.mv;
		entityDataRaw.materialId = entityData.materialId;
		m_entityDataBuffer->setData(entityDataRaw);
		m_entityDataBuffer->applyChanges();

		if (gbuffer)
		{
			m_gbufferRendering->setUniform<LPPAppUniforms>(UF::ENTITY_DATA, m_entityDataBuffer);
			m_gbufferRendering->setUniform<LPPAppUniforms>(UF::NORMAL_MAP, entity.normalTexture);
			m_gbufferRendering->setUniform<LPPAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());
		}
		else
		{
			m_sceneRendering->setUniform<LPPAppUniforms>(UF::ENTITY_DATA, m_entityDataBuffer);
			m_sceneRendering->setUniform<LPPAppUniforms>(UF::DIFFUSE_MAP, entity.texture);
			m_sceneRendering->setUniform<LPPAppUniforms>(UF::SPECULAR_MAP, entity.specularTexture);
			m_sceneRendering->setUniform<LPPAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

		}

		entity.geometry->renderAllMeshes();
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		static wchar_t buf[100];
		swprintf(buf, L"MSAA = %dx\nVisible lights = %d\nVisible objects = %d", m_info.samples, m_lightsCount, m_visibleObjects);
		m_debugLabel->setText(buf);

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);
	}

	virtual void onKeyButton(int key, int scancode, bool pressed)
	{
		if (key == InputKeys::Space && pressed)
		{
			m_pause = !m_pause;
			return;
		}
		if (key == InputKeys::F1 && pressed)
		{
			m_renderDebug = !m_renderDebug;
			m_debugLabel->setVisible(m_renderDebug);
			return;
		}
		if (key == InputKeys::P && pressed)
		{
			if (!utils::Profiler::instance().isRun())
				utils::Profiler::instance().run();
			else
				utils::Profiler::instance().stop();
			return;
		}
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
		matrix44 view = m_camera.getView();
		matrix44 vp = view * m_camera.getProjection();

		// clip objects by frustum
		m_visibleObjects = 0;
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			bbox3 box = m_entity.geometry->getBoundingBox();
			box.transform(m_entitiesData[i].model);
			m_entitiesData[i].isVisible = (box.clipstatus(vp) != bbox3::Outside);
			if (m_entitiesData[i].isVisible)
			{
				m_entitiesData[i].mvp = m_entitiesData[i].model * vp;
				m_entitiesData[i].mv = m_entitiesData[i].model * view;
				m_visibleObjects++;
			}
		}

		// clip lights by frustum
		m_lightsCount = 0;
		for (size_t i = 0; i < m_lightManager.getLightSourcesCount(); i++)
		{
			auto lightSource = m_lightManager.getLightSource(i);
			if (lightSource.type != framework::DirectLight)
			{
				bbox3 box(lightSource.position, vector3(lightSource.falloff, lightSource.falloff, lightSource.falloff));
				if (box.clipstatus(vp) != bbox3::Outside)
				{
					m_lightsBuffer->setElement(m_lightsCount, m_lightManager.getRawLightData(i));
					m_lightsCount++;
					if (m_lightsCount == MAX_LIGHTS_COUNT) break;
				}
			}
			else
			{
				m_lightsBuffer->setElement(m_lightsCount, m_lightManager.getRawLightData(i));
				m_lightsCount++;
				if (m_lightsCount == MAX_LIGHTS_COUNT) break;
			}
		}
		m_lightsBuffer->applyChanges();
	}

private:
	// g-buffer
	std::shared_ptr<framework::RenderTarget> m_gbuffer;
	// light buffer
	std::shared_ptr<framework::RenderTarget> m_lbuffer;

	// gpu program to render in g-buffer
	std::shared_ptr<framework::GpuProgram> m_gbufferRendering;
	// gpu program to perform light pre-pass
	std::shared_ptr<framework::GpuProgram> m_lightPrePass;
	// gpu program to render scene
	std::shared_ptr<framework::GpuProgram> m_sceneRendering;

	// entity
	struct Entity
	{
		std::shared_ptr<framework::Geometry3D> geometry;
		std::shared_ptr<framework::Texture> texture;
		std::shared_ptr<framework::Texture> normalTexture;
		std::shared_ptr<framework::Texture> specularTexture;
	};

	struct EntityData
	{
		matrix44 model;
		matrix44 mvp;
		matrix44 mv;
		unsigned int materialId;
		bool isVisible;

		EntityData() : materialId(1), isVisible(true){}
	};

	Entity m_entity;
	std::vector<EntityData> m_entitiesData;

	std::shared_ptr<framework::UniformBuffer> m_entityDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_onFrameDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;
	unsigned int m_lightsCount;

	std::shared_ptr<framework::DepthStencilStage> m_depthPrePass;

	std::shared_ptr<framework::Texture> m_skyboxTexture;
	framework::FreeCamera m_camera;

	bool m_pause;
	bool m_renderDebug;
	int m_additionalLights;
	int m_visibleObjects;

	gui::LabelPtr_T m_debugLabel;
};

DECLARE_MAIN(LightPrePassApp);