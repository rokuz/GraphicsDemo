#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(DSAppUniforms)
	MODELVIEWPROJECTION_MATRIX,
	MODELVIEW_MATRIX,
	MODEL_MATRIX,
	PROJECTIONINVERSE_MATRIX,
	VIEWINVERSE_MATRIX,
	LIGHTS_DATA,
	LIGHTS_COUNT,
	SPECULAR_POWER,
	MATERIAL_ID,
	VIEW_POSITION,
	SAMPLES_COUNT,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	SKYBOX_MAP,
	DATABLOCK_MAP1,
	DATABLOCK_MAP2
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<DSAppUniforms>::Uniform

// constants
const int MAX_LIGHTS_COUNT = 64;
const std::string SHADERS_PATH = "data/shaders/gl/win32/deferredshading/";

// application
class DeferredShadingApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	DeferredShadingApp()
	{
		m_lightsCount = 0;
		m_pause = false;
		m_renderDebug = false;
		m_additionalLights = 50;
		m_visibleObjects = 0;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Deferred shading (OpenGL 4)";
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

		// g-buffer
		m_gbuffer.reset(new framework::RenderTarget());
		std::vector<int> formats;
		formats.push_back(GL_RGBA32F);
		formats.push_back(GL_RGBA32UI);
		if (!m_gbuffer->init(m_info.windowWidth, m_info.windowHeight, formats, m_info.samples, GL_DEPTH_COMPONENT32F)) exit();

		// gpu programs
		m_gbufferRendering.reset(new framework::GpuProgram());
		m_gbufferRendering->addShader(SHADERS_PATH + "gbuffer.vsh.glsl");
		m_gbufferRendering->addShader(SHADERS_PATH + "gbuffer.fsh.glsl");
		if (!m_gbufferRendering->init()) exit();
		m_gbufferRendering->bindUniform<DSAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_gbufferRendering->bindUniform<DSAppUniforms>(UF::MODELVIEW_MATRIX, "modelViewMatrix");
		m_gbufferRendering->bindUniform<DSAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_gbufferRendering->bindUniform<DSAppUniforms>(UF::SPECULAR_POWER, "specularPower");
		m_gbufferRendering->bindUniform<DSAppUniforms>(UF::MATERIAL_ID, "materialId");
		m_gbufferRendering->bindUniform<DSAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_gbufferRendering->bindUniform<DSAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_gbufferRendering->bindUniform<DSAppUniforms>(UF::SPECULAR_MAP, "specularMap");

		m_deferredShading.reset(new framework::GpuProgram());
		m_deferredShading->addShader(SHADERS_PATH + "screenquad.vsh.glsl");
		m_deferredShading->addShader(SHADERS_PATH + "deferredshading.gsh.glsl");
		if (m_info.samples == 0)
		{
			m_deferredShading->addShader(SHADERS_PATH + "deferredshading.fsh.glsl");
		}
		else
		{
			m_deferredShading->addShader(SHADERS_PATH + "deferredshading_msaa.fsh.glsl");
		}
		if (!m_deferredShading->init()) exit();
		m_deferredShading->bindUniformBuffer<DSAppUniforms>(UF::LIGHTS_DATA, "lightsDataBuffer");
		m_deferredShading->bindUniform<DSAppUniforms>(UF::LIGHTS_COUNT, "lightsCount");
		m_deferredShading->bindUniform<DSAppUniforms>(UF::PROJECTIONINVERSE_MATRIX, "projectionInverseMatrix");
		m_deferredShading->bindUniform<DSAppUniforms>(UF::VIEWINVERSE_MATRIX, "viewInverseMatrix");
		m_deferredShading->bindUniform<DSAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_deferredShading->bindUniform<DSAppUniforms>(UF::DATABLOCK_MAP1, "dataBlockMap1");
		m_deferredShading->bindUniform<DSAppUniforms>(UF::DATABLOCK_MAP2, "dataBlockMap2");
		if (m_info.samples > 0)
		{
			m_deferredShading->bindUniform<DSAppUniforms>(UF::SAMPLES_COUNT, "samplesCount");
		}
		
		m_skyboxRendering.reset(new framework::GpuProgram());
		m_skyboxRendering->addShader(SHADERS_PATH + "screenquad.vsh.glsl");
		m_skyboxRendering->addShader(SHADERS_PATH + "skybox.gsh.glsl");
		m_skyboxRendering->addShader(SHADERS_PATH + "skybox.fsh.glsl");
		if (!m_skyboxRendering->init()) exit();
		m_skyboxRendering->bindUniform<DSAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_skyboxRendering->bindUniform<DSAppUniforms>(UF::SKYBOX_MAP, "skyboxMap");

		// entity
		m_entity = initEntity("data/media/cube/cube.geom",
							  "data/media/cube/cube_diff.ktx",
							  "data/media/cube/cube_normal.ktx",
							  "data/media/textures/full_specular.png");

		m_entitiesData.resize(25);
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			m_entitiesData[i].model.set_translation(utils::Utils::random(-70.0f, 70.0f));
		}

		// skybox texture
		m_skyboxTexture.reset(new framework::Texture());
		if (!m_skyboxTexture->initAsCubemap("data/media/textures/nightsky_front.jpg",
											"data/media/textures/nightsky_back.jpg",
											"data/media/textures/nightsky_left.jpg",
											"data/media/textures/nightsky_right.jpg",
											"data/media/textures/nightsky_top.jpg",
											"data/media/textures/nightsky_bottom.jpg")) exit();

		// lights
		initLights();

		framework::PipelineState depthTestEnable(GL_DEPTH_TEST, true);
		depthTestEnable.apply();

		framework::PipelineState cullingEnable(GL_CULL_FACE, true);
		cullingEnable.apply();

		glViewport(0, 0, m_info.windowWidth, m_info.windowHeight);
	}

	void initOverlays(gui::WidgetPtr_T root)
	{
		m_debugLabel = framework::UIFactory::createLabel(gui::Coords(1.0f, -300.0f, 1.0f, -150.0f),
														 gui::Coords::Absolute(300.0f, 150.0f),
														 gui::RightAligned, gui::BottomAligned);
		root->addChild(m_debugLabel);

		m_debugLabel->setVisible(m_renderDebug);
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
			ent.texture->init(texture);
		}

		if (!normalTexture.empty())
		{
			ent.normalTexture.reset(new framework::Texture());
			ent.normalTexture->init(normalTexture);
		}

		if (!specularTexture.empty())
		{
			ent.specularTexture.reset(new framework::Texture());
			ent.specularTexture->init(specularTexture);
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
		if (!m_lightsBuffer->init<framework::LightRawData>((size_t)MAX_LIGHTS_COUNT)) exit();

		int lightsCount = std::min((int)m_lightManager.getLightSourcesCount(), MAX_LIGHTS_COUNT);
		for (int i = 0; i < lightsCount; i++)
		{
			m_lightsBuffer->setElement(i, m_lightManager.getRawLightData(i));
		}
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

		matrix44 invview = m_camera.getView();
		invview.invert();
		matrix44 invproj = m_camera.getProjection();
		invproj.invert();

		m_gbuffer->set();
		m_gbuffer->clearColorAsFloat(0);
		m_gbuffer->clearColorAsUint(1);
		m_gbuffer->clearDepth();

		// render to g-buffer
		if (m_gbufferRendering->use())
		{
			TRACE_BLOCK("_GBuffer")
			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderEntity(m_entity, m_entitiesData[i]);
			}
		}

		useDefaultRenderTarget();

		m_gbuffer->copyDepthToCurrentDepthBuffer();
		
		// skybox
		renderSkybox();

		// deferred shading pass
		if (m_deferredShading->use())
		{
			TRACE_BLOCK("_DeferredShading")
			m_deferredShading->setUniformBuffer<DSAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer, 0);
			m_deferredShading->setUint<DSAppUniforms>(UF::LIGHTS_COUNT, m_lightsCount);
			m_deferredShading->setVector<DSAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());
			m_deferredShading->setMatrix<DSAppUniforms>(UF::VIEWINVERSE_MATRIX, invview);
			m_deferredShading->setMatrix<DSAppUniforms>(UF::PROJECTIONINVERSE_MATRIX, invproj);
			if (m_info.samples > 0)
			{
				m_deferredShading->setUint<DSAppUniforms>(UF::SAMPLES_COUNT, m_info.samples);
			}
			m_deferredShading->setTexture<DSAppUniforms>(UF::DATABLOCK_MAP1, m_gbuffer, 0);
			m_deferredShading->setTexture<DSAppUniforms>(UF::DATABLOCK_MAP2, m_gbuffer, 1);

			framework::PipelineState depthTestDisable(GL_DEPTH_TEST, false);
			framework::PipelineState blendingEnable(GL_BLEND, true);
			blendingEnable.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			depthTestDisable.apply();
			blendingEnable.apply();
			glDrawArrays(GL_POINTS, 0, 1);
			depthTestDisable.cancel();
			blendingEnable.cancel();
		}
    
		// debug rendering
		renderDebug();

		CHECK_GL_ERROR;
	}

	void renderEntity(const Entity& entity, const EntityData& entityData)
	{
		if (!entityData.isVisible) return;

		m_gbufferRendering->setMatrix<DSAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, entityData.mvp);
		m_gbufferRendering->setMatrix<DSAppUniforms>(UF::MODEL_MATRIX, entityData.model);
		m_gbufferRendering->setMatrix<DSAppUniforms>(UF::MODELVIEW_MATRIX, entityData.mv);
		m_gbufferRendering->setFloat<DSAppUniforms>(UF::SPECULAR_POWER, entityData.specularPower);
		m_gbufferRendering->setUint<DSAppUniforms>(UF::MATERIAL_ID, entityData.materialId);
		m_gbufferRendering->setTexture<DSAppUniforms>(UF::DIFFUSE_MAP, entity.texture);
		m_gbufferRendering->setTexture<DSAppUniforms>(UF::NORMAL_MAP, entity.normalTexture);
		m_gbufferRendering->setTexture<DSAppUniforms>(UF::SPECULAR_MAP, entity.specularTexture);

		entity.geometry->renderAllMeshes();
	}

	void renderSkybox()
	{
		if (m_skyboxRendering->use())
		{
			framework::PipelineState depthTestDisable(GL_DEPTH_TEST, false);
			depthTestDisable.apply();

			matrix44 model;
			model.set_translation(m_camera.getPosition());
			matrix44 mvp = model * m_camera.getView() * m_camera.getProjection();

			m_skyboxRendering->setTexture<DSAppUniforms>(UF::SKYBOX_MAP, m_skyboxTexture);
			m_skyboxRendering->setMatrix<DSAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, mvp);

			glDrawArrays(GL_POINTS, 0, 1);

			depthTestDisable.cancel();
		}
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
	}

private:
	std::shared_ptr<framework::RenderTarget> m_gbuffer;

	// gpu program to render in g-buffer
	std::shared_ptr<framework::GpuProgram> m_gbufferRendering;
	// gpu program to render skybox
	std::shared_ptr<framework::GpuProgram> m_skyboxRendering;
	// gpu program to deferred shading
	std::shared_ptr<framework::GpuProgram> m_deferredShading;

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
		float specularPower;
		unsigned int materialId;
		bool isVisible;

		EntityData() : specularPower(30.0f), materialId(1), isVisible(true){}
	};

	// opaque entity
	Entity m_entity;
	std::vector<EntityData> m_entitiesData;

	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;
	
	std::shared_ptr<framework::Texture> m_skyboxTexture;
	framework::FreeCamera m_camera;

	unsigned int m_lightsCount;
	bool m_pause;
	bool m_renderDebug;
	int m_additionalLights;
	int m_visibleObjects;

	gui::LabelPtr_T m_debugLabel;
};

DECLARE_MAIN(DeferredShadingApp);