#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(DSAppUniforms)
	MODELVIEWPROJECTION_MATRIX,
	MODEL_MATRIX,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	VIEW_POSITION,
	LIGHTS_DATA_BUFFER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<DSAppUniforms>::Uniform

// constants
const int MAX_LIGHTS_COUNT = 64;
const std::string SHADERS_PATH = "data/shaders/gl/win32/template/";//deferredshading/";

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

		m_rotation = 0.0f;

		m_geometry.reset(new framework::Geometry3D());
		if (!m_geometry->init("data/media/spaceship/spaceship.geom")) exit();
    
		m_texture.reset(new framework::Texture());
		if (!m_texture->initWithKtx("data/media/spaceship/spaceship_diff.ktx")) exit();

		m_specularTexture.reset(new framework::Texture());
		if (!m_specularTexture->initWithKtx("data/media/spaceship/spaceship_specular.ktx")) exit();

		m_normalTexture.reset(new framework::Texture());
		if (!m_normalTexture->initWithKtx("data/media/spaceship/spaceship_normal.ktx")) exit();

		m_program.reset(new framework::GpuProgram());
		m_program->addShader(SHADERS_PATH + "shader.vsh.glsl");
		m_program->addShader(SHADERS_PATH + "shader.fsh.glsl");
		if (!m_program->init()) exit();
		m_program->bindUniform<DSAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_program->bindUniform<DSAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_program->bindUniform<DSAppUniforms>(UF::DIFFUSE_MAP, "diffuseSampler");
		m_program->bindUniform<DSAppUniforms>(UF::NORMAL_MAP, "normalSampler");
		m_program->bindUniform<DSAppUniforms>(UF::SPECULAR_MAP, "specularSampler");
		m_program->bindUniform<DSAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_program->bindUniformBuffer<DSAppUniforms>(UF::LIGHTS_DATA_BUFFER, "lightsDataBuffer");

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
			//ent.texture->initWithDDS(texture);
		}

		if (!normalTexture.empty())
		{
			ent.normalTexture.reset(new framework::Texture());
			//ent.normalTexture->initWithDDS(normalTexture);
		}

		if (!specularTexture.empty())
		{
			ent.specularTexture.reset(new framework::Texture());
			//ent.specularTexture->initWithDDS(specularTexture);
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

		quaternion quat;
		quat.set_rotate_x(n_deg2rad(-90.0f));
		quaternion quat2;
		quat2.set_rotate_z(-n_deg2rad(m_rotation));
		matrix44 model(quat * quat2);
		quaternion quat3;
		quat3.set_rotate_y(-n_deg2rad(m_rotation));
		model.set_translation(quat3.z_direction() * 30.0f);

		m_mvp = (model * m_camera.getView()) * m_camera.getProjection();

		m_rotation += (float)elapsedTime * 70.0f;

		const GLfloat color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat depth = 1.0f;
		glClearBufferfv(GL_COLOR, 0, color);
		glClearBufferfv(GL_DEPTH, 0, &depth);
    
		if (m_program->use())
		{
			m_program->setMatrix<DSAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, m_mvp);
			m_program->setMatrix<DSAppUniforms>(UF::MODEL_MATRIX, model);
			m_program->setVector<DSAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());
			m_program->setUniformBuffer<DSAppUniforms>(UF::LIGHTS_DATA_BUFFER, *m_lightsBuffer, 0);

			m_texture->setToSampler(m_program->getUniform<DSAppUniforms>(UF::DIFFUSE_MAP));
			m_normalTexture->setToSampler(m_program->getUniform<DSAppUniforms>(UF::NORMAL_MAP));
			m_specularTexture->setToSampler(m_program->getUniform<DSAppUniforms>(UF::SPECULAR_MAP));

			m_geometry->renderAllMeshes();
		}

		// debug rendering
		renderDebug();
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

	// -----
	std::shared_ptr<framework::GpuProgram> m_program;
	std::shared_ptr<framework::Geometry3D> m_geometry;
	std::shared_ptr<framework::Texture> m_texture;
	std::shared_ptr<framework::Texture> m_normalTexture;
	std::shared_ptr<framework::Texture> m_specularTexture;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;
	matrix44 m_mvp;
	float m_rotation;
	// -----

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