#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(OITAppUniforms)
	SPATIAL_DATA,
	LIGHTS_DATA,
	LIGHTS_COUNT,
	ENVIRONMENT_MAP,
	DEFAULT_SAMPLER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<OITAppUniforms>::Uniform

// spatial data
#pragma pack (push, 1)
struct SpatialData
{
	matrix44 modelViewProjection;
	matrix44 model;
	vector3 viewPosition;
	unsigned int lightsCount;
	vector4 color;
};
#pragma pack (pop)

// constants
const int MAX_LIGHTS_COUNT = 16;
const std::string SHADERS_PATH = "data/shaders/dx11/oit/";
#define PROFILING 0

// application
class OITApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	OITApp()
	{
		m_lightsCount = 0;
		m_renderDebug = false;
		m_fragmentsBufferSize = 0;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Order Independent Transparency (DX11)";
		
		applyStandardParams(params);

		m_info.flags.debug = 1;

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
		//m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(-30, 30, -90), vector3());
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(-30.96, 4.15, -50.85), vector3(-30.96, 4.15, -49.85));

		// overlays
		initOverlays(root);

		// head buffer
		m_headBuffer.reset(new framework::RenderTarget());
		auto headBufferDesc = framework::RenderTarget::getDefaultDesc(m_info.windowWidth, 
																	  m_info.windowHeight, 
																	  DXGI_FORMAT_R32_UINT);
		headBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		m_headBuffer->initWithDescription(headBufferDesc, false);
		if (!m_headBuffer->isValid()) exit();

		// fragments buffer (we use buffer 8 time more than screen size)
		m_fragmentsBufferSize = (unsigned int)m_info.windowWidth * m_info.windowHeight * 8;
		unsigned int fragmentSize = 4 + 4 + 4; // color + depth + next
		unsigned int fragmentsBufferFlags = D3D11_BUFFER_UAV_FLAG::D3D11_BUFFER_UAV_FLAG_COUNTER;
		m_fragmentsBuffer.reset(new framework::UnorderedAccessBuffer());
		if (!m_fragmentsBuffer->initDefaultUnorderedAccess(m_fragmentsBufferSize, fragmentSize, fragmentsBufferFlags)) exit();

		// gpu programs
		m_opaqueRendering.reset(new framework::GpuProgram());
		m_opaqueRendering->addShader(SHADERS_PATH + "opaque.vsh.hlsl");
		m_opaqueRendering->addShader(SHADERS_PATH + "opaque.psh.hlsl");
		if (!m_opaqueRendering->init()) exit();
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::SPATIAL_DATA, "spatialData");
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::LIGHTS_DATA, "lightsData");
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::ENVIRONMENT_MAP, "environmentMap");
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");

		m_fragmentsListCreation.reset(new framework::GpuProgram());
		m_fragmentsListCreation->addShader(SHADERS_PATH + "opaque.vsh.hlsl");
		m_fragmentsListCreation->addShader(SHADERS_PATH + "fragmentslist.psh.hlsl");
		if (!m_fragmentsListCreation->init()) exit();
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::SPATIAL_DATA, "spatialData");
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::LIGHTS_DATA, "lightsData");
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::ENVIRONMENT_MAP, "environmentMap");
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::DEFAULT_SAMPLER, "defaultSampler");
		
		m_transparentRendering.reset(new framework::GpuProgram());
		m_transparentRendering->addShader(SHADERS_PATH + "screenquad.vsh.hlsl");
		m_transparentRendering->addShader(SHADERS_PATH + "screenquad.gsh.hlsl");
		if (m_info.samples == 0)
		{
			m_transparentRendering->addShader(SHADERS_PATH + "transparent.psh.hlsl");
		}
		else
		{
			m_transparentRendering->addShader(SHADERS_PATH + "transparent_msaa.psh.hlsl");
		}
		if (!m_transparentRendering->init(true)) exit();

		// entity
		m_entity = initEntity("data/media/models/teapot.geom");
		m_entity.geometry->bindToGpuProgram(m_opaqueRendering);
		m_entity.geometry->bindToGpuProgram(m_fragmentsListCreation);

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

				m_entitiesData[index].transparent = !(i % 2 == 0 && j % 2 == 0);
				m_entitiesData[index].color = utils::Utils::random();
				m_entitiesData[index].color.w = utils::Utils::random(0.3f, 0.7f).x;
				m_entitiesData[index].model.set_translation(vector3(x * AREA_HALFLENGTH, 0.0f, z * AREA_HALFLENGTH));
			}
		}

		// skybox texture
		m_skyboxTexture.reset(new framework::Texture());
		if (!m_skyboxTexture->initWithDDS("data/media/textures/meadow.dds")) exit();

		// a blend state to disable color writing
		m_disableColorWriting.reset(new framework::BlendStage());
		D3D11_BLEND_DESC blendDesc = framework::BlendStage::getDisableColorWriting();
		m_disableColorWriting->initWithDescription(blendDesc);
		if (!m_disableColorWriting->isValid()) exit();

		// a depth-stencil state to disable depth writing
		m_disableDepthWriting.reset(new framework::DepthStencilStage());
		D3D11_DEPTH_STENCIL_DESC depthDesc = framework::DepthStencilStage::getDisableDepthWriting();
		m_disableDepthWriting->initWithDescription(depthDesc);
		if (!m_disableDepthWriting->isValid()) exit();

		// a rasterizer to render without culling
		m_cullingOff.reset(new framework::RasterizerStage());
		D3D11_RASTERIZER_DESC rasterizerDesc = defaultRasterizer()->getDesc();
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		m_cullingOff->initWithDescription(rasterizerDesc);
		if (!m_cullingOff->isValid()) exit();
		auto viewports = defaultRasterizer()->getViewports();
		m_cullingOff->getViewports().reserve(viewports.size());
		m_cullingOff->getViewports() = viewports;

		// a blend state to enable alpha-blending
		m_alphaBlending.reset(new framework::BlendStage());
		blendDesc = framework::BlendStage::getAlphaBlending();
		for (int i = 0; i < 8; i++)
		{
			blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_SRC_ALPHA;
		}
		m_alphaBlending->initWithDescription(blendDesc);
		if (!m_alphaBlending->isValid()) exit();

		// spatial info buffer
		m_spatialBuffer.reset(new framework::UniformBuffer());
		if (!m_spatialBuffer->initDefaultConstant<SpatialData>()) exit();

		// lights
		initLights();
	}

	Entity initEntity(const std::string& geometry)
	{
		Entity ent;

		// geometry
		ent.geometry.reset(new framework::Geometry3D());
		if (!ent.geometry->init(geometry)) exit();

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

		// directional light 2
		framework::LightSource source2;
		source2.type = framework::LightType::DirectLight;
		source2.position = vector3(15, 15, 0);
		dir = vector3(0, -1, 1);
		dir.norm();
		source2.diffuseColor = vector3(0.96f, 0.81f, 0.59f) * 0.5f;
		source2.specularColor = vector3(0.1f, 0.1f, 0.1f);
		source2.ambientColor = vector3(0.0f, 0.0f, 0.0f);
		source2.orientation.set_from_axes(vector3(0, 0, 1), dir);
		m_lightManager.addLightSource(source2);

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
														 gui::RightAligned, gui::BottomAligned,
														 L"Fragments buffer usage = 0 %\nLost fragments = 0");
		m_debugLabel->setVisible(m_renderDebug);
		root->addChild(m_debugLabel);

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

		// clear only head buffer
		framework::UnorderedAccessibleBatch clearbatch;
		clearbatch.add(m_headBuffer);
		getPipeline().clearUnorderedAccessBatch(clearbatch);
		
		// set render target and head/fragments buffers
		framework::UnorderedAccessibleBatch batch;
		batch.add(m_headBuffer);
		batch.add(m_fragmentsBuffer, 0);
		getPipeline().setRenderTarget(defaultRenderTarget(), batch);

		// render skybox
		renderSkybox(m_camera, m_skyboxTexture);
		
		// render opaque objects
		if (m_opaqueRendering->use())
		{
			m_opaqueRendering->setUniform<OITAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);
			m_opaqueRendering->setUniform<OITAppUniforms>(UF::ENVIRONMENT_MAP, m_skyboxTexture);
			m_opaqueRendering->setUniform<OITAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				if (!m_entitiesData[i].transparent) renderEntity(m_opaqueRendering, m_entity, m_entitiesData[i]);
			}
		}

		// build lists of fragments for transparent objects
		if (m_fragmentsListCreation->use())
		{
			m_fragmentsListCreation->setUniform<OITAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer);
			m_fragmentsListCreation->setUniform<OITAppUniforms>(UF::ENVIRONMENT_MAP, m_skyboxTexture);
			m_fragmentsListCreation->setUniform<OITAppUniforms>(UF::DEFAULT_SAMPLER, anisotropicSampler());

			m_cullingOff->apply();
			m_disableColorWriting->apply();
			m_disableDepthWriting->apply();
			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				if (m_entitiesData[i].transparent) renderEntity(m_fragmentsListCreation, m_entity, m_entitiesData[i]);
			}
			m_disableDepthWriting->cancel();
			m_disableColorWriting->cancel();
			m_cullingOff->cancel();
		}

		// render transparent objects
		if (m_transparentRendering->use())
		{
			disableDepthTest()->apply();
			m_alphaBlending->apply();
			getPipeline().drawPoints(1);
			disableDepthTest()->cancel();
			m_alphaBlending->cancel();
		}

		// debug rendering
		renderDebug();
	}

	void renderEntity(const std::shared_ptr<framework::GpuProgram>& program, const Entity& entity, const EntityData& entityData)
	{
		SpatialData spatialData;
		spatialData.modelViewProjection = entityData.mvp;
		spatialData.model = entityData.model;
		spatialData.viewPosition = m_camera.getPosition();
		spatialData.lightsCount = m_lightsCount;
		spatialData.color = entityData.color;
		m_spatialBuffer->setData(spatialData);
		m_spatialBuffer->applyChanges();

		program->setUniform<OITAppUniforms>(UF::SPATIAL_DATA, m_spatialBuffer);

		entity.geometry->renderAllMeshes();
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		unsigned int bufferUsage = m_fragmentsBuffer->getActualSize();
		unsigned int lostFragments = 0;
		double usage = ((double)bufferUsage / (double)m_fragmentsBufferSize) * 100.0;
		if (usage > 100.0)
		{
			usage = 100.0;
			lostFragments = bufferUsage - m_fragmentsBufferSize;
		}
		static wchar_t buf[100];
		swprintf(buf, L"Fragments buffer usage = %d%%\nLost fragments = %d", (int)usage, lostFragments);
		m_debugLabel->setText(buf);
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
	}

private:
	// texture to store heads of dynamic lists
	std::shared_ptr<framework::RenderTarget> m_headBuffer;
	// buffer to store dynamic lists
	std::shared_ptr<framework::UnorderedAccessBuffer> m_fragmentsBuffer;
	
	// gpu program to render opaque geometry
	std::shared_ptr<framework::GpuProgram> m_opaqueRendering;
	// gpu program to create fragments list
	std::shared_ptr<framework::GpuProgram> m_fragmentsListCreation;
	// gpu program to render transparent geometry by fragments list
	std::shared_ptr<framework::GpuProgram> m_transparentRendering;

	// pipeline stages
	std::shared_ptr<framework::BlendStage> m_disableColorWriting;
	std::shared_ptr<framework::DepthStencilStage> m_disableDepthWriting;
	std::shared_ptr<framework::RasterizerStage> m_cullingOff;
	std::shared_ptr<framework::BlendStage> m_alphaBlending;

	// entity
	struct Entity
	{
		std::shared_ptr<framework::Geometry3D> geometry;
	};
	Entity m_entity;

	struct EntityData
	{
		matrix44 model;
		matrix44 mvp;
		vector4 color;
		bool transparent;
		EntityData() : color(vector4(1, 1, 1, 1)), transparent(true) {}
	};
	std::vector<EntityData> m_entitiesData;

	std::shared_ptr<framework::Texture> m_skyboxTexture;
	std::shared_ptr<framework::UniformBuffer> m_spatialBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;
	unsigned int m_lightsCount;
	framework::FreeCamera m_camera;

	bool m_renderDebug;
	unsigned int m_fragmentsBufferSize;
	gui::LabelPtr_T m_debugLabel;
};

DECLARE_MAIN(OITApp);