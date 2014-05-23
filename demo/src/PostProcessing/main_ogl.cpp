#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(PPAppUniforms)
	MODELVIEWPROJECTION_MATRIX,
	MODEL_MATRIX,
	LIGHTS_DATA,
	LIGHTS_COUNT,
	SPECULAR_POWER,
	VIEW_POSITION,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	SCREENQUAD_MAP,
	SAMPLES_COUNT
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<PPAppUniforms>::Uniform

// constants
const int MAX_LIGHTS_COUNT = 8;
const std::string SHADERS_PATH = "data/shaders/gl/win32/postprocessing/";

// application
class PostProcessingApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	PostProcessingApp()
	{
		m_pause = false;
		m_renderDebug = false;
		m_visibleObjects = 0;
		m_lightsCount = 0;
		m_samples = 0;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Post processing (OpenGL 4)";
		applyStandardParams(params);
		m_samples = (int)m_info.samples;
		m_info.samples = 0;

		setLegend("WASD - move camera\nLeft mouse button - rotate camera\nF1 - debug info");
	}

	virtual void startup(gui::WidgetPtr_T root)
	{
		// camera
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

		// overlays
		initOverlays(root);

		// scene buffer
		m_sceneBuffer.reset(new framework::RenderTarget());
		std::vector<int> formats;
		formats.push_back(GL_RGBA32F);
		if (!m_sceneBuffer->init(m_info.windowWidth, m_info.windowHeight, formats, m_samples, GL_DEPTH_COMPONENT32F)) exit();

		// gpu programs
		m_sceneRendering.reset(new framework::GpuProgram());
		m_sceneRendering->addShader(SHADERS_PATH + "scene.vsh.glsl");
		m_sceneRendering->addShader(SHADERS_PATH + "scene.fsh.glsl");
		if (!m_sceneRendering->init()) exit();
		m_sceneRendering->bindUniform<PPAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_sceneRendering->bindUniform<PPAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_sceneRendering->bindUniform<PPAppUniforms>(UF::SPECULAR_POWER, "specularPower");
		m_sceneRendering->bindUniform<PPAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_sceneRendering->bindUniform<PPAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_sceneRendering->bindUniform<PPAppUniforms>(UF::SPECULAR_MAP, "specularMap");
		m_sceneRendering->bindStorageBuffer<PPAppUniforms>(UF::LIGHTS_DATA, "lightsDataBuffer");
		m_sceneRendering->bindUniform<PPAppUniforms>(UF::LIGHTS_COUNT, "lightsCount");
		m_sceneRendering->bindUniform<PPAppUniforms>(UF::VIEW_POSITION, "viewPosition");

		m_quadRendering.reset(new framework::GpuProgram());
		m_quadRendering->addShader(SHADERS_PATH + "screenquad.vsh.glsl");
		m_quadRendering->addShader(SHADERS_PATH + "screenquad.gsh.glsl");
		if (m_samples == 0)
		{
			m_quadRendering->addShader(SHADERS_PATH + "screenquad.fsh.glsl");
		}
		else
		{
			m_quadRendering->addShader(SHADERS_PATH + "screenquad_msaa.fsh.glsl");
		}
		if (!m_quadRendering->init()) exit();
		m_quadRendering->bindUniform<PPAppUniforms>(UF::SCREENQUAD_MAP, "screenquadMap");
		if (m_samples > 0)
		{
			m_quadRendering->bindUniform<PPAppUniforms>(UF::SAMPLES_COUNT, "samplesCount");
		}

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

		framework::DepthState depthTestEnable(true);
		depthTestEnable.setWriteEnable(true);
		depthTestEnable.apply();

		framework::PipelineState cullingEnable(GL_CULL_FACE, true);
		cullingEnable.apply();

		framework::PipelineState msaaEnable(GL_MULTISAMPLE, m_samples > 0);
		msaaEnable.apply();

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
		/*framework::LightSource source2;
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
		}*/

		// light buffer
		m_lightsBuffer.reset(new framework::UniformBuffer());
		if (!m_lightsBuffer->init<framework::LightRawData>((size_t)MAX_LIGHTS_COUNT, true)) exit();

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

		// clear only depth (skybox will clear color)
		m_sceneBuffer->set();
		m_sceneBuffer->clearDepth();

		// render skybox
		renderSkybox(m_camera, m_skyboxTexture);

		// render scene
		if (m_sceneRendering->use())
		{
			m_sceneRendering->setStorageBuffer<PPAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer, 0);
			m_sceneRendering->setUint<PPAppUniforms>(UF::LIGHTS_COUNT, m_lightsCount);
			m_sceneRendering->setVector<PPAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderEntity(m_entity, m_entitiesData[i]);
			}
		}
		
		useDefaultRenderTarget();
		m_sceneBuffer->copyDepthToCurrentDepthBuffer(m_samples);

		// render quad
		if (m_quadRendering->use())
		{
			m_quadRendering->setTexture<PPAppUniforms>(UF::SCREENQUAD_MAP, m_sceneBuffer, 0);
			if (m_samples > 0)
			{
				m_quadRendering->setInt<PPAppUniforms>(UF::SAMPLES_COUNT, m_samples);
			}

			framework::DepthState depthTestDisable(false);
			depthTestDisable.apply();
			glDrawArrays(GL_POINTS, 0, 1);
			depthTestDisable.cancel();
		}

		// debug rendering
		renderDebug();

		CHECK_GL_ERROR;
	}

	void renderEntity(const Entity& entity, const EntityData& entityData)
	{
		if (!entityData.isVisible) return;

		m_sceneRendering->setMatrix<PPAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, entityData.mvp);
		m_sceneRendering->setMatrix<PPAppUniforms>(UF::MODEL_MATRIX, entityData.model);
		m_sceneRendering->setFloat<PPAppUniforms>(UF::SPECULAR_POWER, entityData.specularPower);
		m_sceneRendering->setTexture<PPAppUniforms>(UF::DIFFUSE_MAP, entity.texture);
		m_sceneRendering->setTexture<PPAppUniforms>(UF::NORMAL_MAP, entity.normalTexture);
		m_sceneRendering->setTexture<PPAppUniforms>(UF::SPECULAR_MAP, entity.specularTexture);

		entity.geometry->renderAllMeshes();
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		static wchar_t buf[100];
		swprintf(buf, L"MSAA = %dx", m_samples);
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
	std::shared_ptr<framework::RenderTarget> m_sceneBuffer;

	// gpu program to render scene
	std::shared_ptr<framework::GpuProgram> m_sceneRendering;
	// gpu program to render screen quad
	std::shared_ptr<framework::GpuProgram> m_quadRendering;

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
		bool isVisible;

		EntityData() : specularPower(30.0f), isVisible(true){}
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
	int m_visibleObjects;

	int m_samples;

	gui::LabelPtr_T m_debugLabel;
};

DECLARE_MAIN(PostProcessingApp);