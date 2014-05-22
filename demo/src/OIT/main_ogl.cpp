#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(OITAppUniforms)
	MODELVIEWPROJECTION_MATRIX,
	MODEL_MATRIX,
	LIGHTS_DATA,
	LIGHTS_COUNT,
	VIEW_POSITION,
	ENVIRONMENT_MAP,
	HEAD_BUFFER
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
const std::string SHADERS_PATH = "data/shaders/gl/win32/oit/";

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
		m_info.title = "Order Independent Transparency (OpenGL 4)";
		
		applyStandardParams(params);
		
		setLegend("WASD - move camera\nLeft mouse button - rotate camera\nF1 - debug info");
	}

	virtual void startup(gui::WidgetPtr_T root)
	{
		// camera
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(-30, 30, -90), vector3());

		// overlays
		initOverlays(root);

		// head buffer
		m_headBuffer.reset(new framework::RenderTarget());
		std::vector<int> formats;
		formats.push_back(GL_R32UI);
		if (!m_headBuffer->init(m_info.windowWidth, m_info.windowHeight, formats)) exit();

		// fragments counter
		m_fragmentsCounter.reset(new framework::AtomicCounter());
		if (!m_fragmentsCounter->init()) exit();

		// fragments buffer (we use buffer 8 time more than screen size)
		m_fragmentsBufferSize = (unsigned int)m_info.windowWidth * m_info.windowHeight * 8;
		unsigned int fragmentSize = 4 + 4 + 4; // color + depth + next
		m_fragmentsBuffer.reset(new framework::StorageBuffer());
		if (!m_fragmentsBuffer->init(fragmentSize, m_fragmentsBufferSize)) exit();

		// gpu programs
		m_opaqueRendering.reset(new framework::GpuProgram());
		m_opaqueRendering->addShader(SHADERS_PATH + "opaque.vsh.glsl");
		m_opaqueRendering->addShader(SHADERS_PATH + "opaque.fsh.glsl");
		if (!m_opaqueRendering->init()) exit();
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_opaqueRendering->bindUniformBuffer<OITAppUniforms>(UF::LIGHTS_DATA, "lightsDataBuffer");
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::LIGHTS_COUNT, "lightsCount");
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_opaqueRendering->bindUniform<OITAppUniforms>(UF::ENVIRONMENT_MAP, "environmentMap");
	
		m_fragmentsListCreation.reset(new framework::GpuProgram());
		m_fragmentsListCreation->addShader(SHADERS_PATH + "opaque.vsh.glsl");
		m_fragmentsListCreation->addShader(SHADERS_PATH + "fragmentslist.fsh.glsl");
		if (!m_fragmentsListCreation->init()) exit();
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_fragmentsListCreation->bindUniformBuffer<OITAppUniforms>(UF::LIGHTS_DATA, "lightsDataBuffer");
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::LIGHTS_COUNT, "lightsCount");
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::ENVIRONMENT_MAP, "environmentMap");
		m_fragmentsListCreation->bindUniform<OITAppUniforms>(UF::HEAD_BUFFER, "headBuffer");
		
		m_transparentRendering.reset(new framework::GpuProgram());
		m_transparentRendering->addShader(SHADERS_PATH + "screenquad.vsh.glsl");
		m_transparentRendering->addShader(SHADERS_PATH + "screenquad.gsh.glsl");
		m_transparentRendering->addShader(SHADERS_PATH + "transparent.fsh.glsl");
		if (!m_transparentRendering->init()) exit();
		m_transparentRendering->bindUniform<OITAppUniforms>(UF::HEAD_BUFFER, "headBuffer");

		// entity
		m_entity = initEntity("data/media/models/teapot.geom");

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
				m_entitiesData[index].model.set_translation(vector3(x * AREA_HALFLENGTH, 0.0f, z * AREA_HALFLENGTH));
			}
		}

		// skybox texture
		m_skyboxTexture.reset(new framework::Texture());
		if (!m_skyboxTexture->initAsCubemap("data/media/textures/meadow_front.jpg",
											"data/media/textures/meadow_back.jpg",
											"data/media/textures/meadow_left.jpg",
											"data/media/textures/meadow_right.jpg",
											"data/media/textures/meadow_top.jpg",
											"data/media/textures/meadow_bottom.jpg")) exit();

		// lights
		initLights();

		framework::DepthState depthTestEnable(true);
		depthTestEnable.setWriteEnable(true);
		depthTestEnable.apply();

		framework::PipelineState cullingEnable(GL_CULL_FACE, true);
		cullingEnable.apply();

		glViewport(0, 0, m_info.windowWidth, m_info.windowHeight);
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
		if (!m_lightsBuffer->init<framework::LightRawData>((size_t)MAX_LIGHTS_COUNT)) exit();

		m_lightsCount = std::min(m_lightManager.getLightSourcesCount(), (size_t)MAX_LIGHTS_COUNT);
		for (size_t i = 0; i < m_lightsCount; i++)
		{
			m_lightsBuffer->setElement(i, m_lightManager.getRawLightData(i));
		}
	}

	void initOverlays(gui::WidgetPtr_T root)
	{
		m_debugLabel = framework::UIFactory::createLabel(gui::Coords(1.0f, -300.0f, 1.0f, -150.0f),
														 gui::Coords::Absolute(300.0f, 150.0f),
														 gui::RightAligned, gui::BottomAligned,
														 L"Fragments buffer usage = 0%\nLost fragments = 0");
		m_debugLabel->setVisible(m_renderDebug);
		root->addChild(m_debugLabel);

		m_fpsLabel->setColor(vector4(0.0f, 0.5f, 1.0f, 1.0f));
		m_legendLabel->setColor(vector4(0.0f, 0.5f, 1.0f, 1.0f));
		m_debugLabel->setColor(vector4(0.0f, 0.5f, 1.0f, 1.0f));
	}

	virtual void render(double elapsedTime)
	{
		m_camera.update(elapsedTime);
		update(elapsedTime);

		// clear head buffer and fragments counter
		m_fragmentsCounter->clear();
		m_headBuffer->clearColorAsUint(0);

		// render skybox
		renderSkybox(m_camera, m_skyboxTexture);
		
		// render opaque objects
		if (m_opaqueRendering->use())
		{
			m_opaqueRendering->setUniformBuffer<OITAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer, 0);
			m_opaqueRendering->setUint<OITAppUniforms>(UF::LIGHTS_COUNT, m_lightsCount);
			m_opaqueRendering->setVector<OITAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());
			m_opaqueRendering->setTexture<OITAppUniforms>(UF::ENVIRONMENT_MAP, m_skyboxTexture);

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				if (!m_entitiesData[i].transparent) renderEntity(m_opaqueRendering, m_entity, m_entitiesData[i]);
			}
		}

		// build lists of fragments for transparent objects
		if (m_fragmentsListCreation->use())
		{
			m_fragmentsListCreation->setUniformBuffer<OITAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer, 0);
			m_fragmentsListCreation->setUint<OITAppUniforms>(UF::LIGHTS_COUNT, m_lightsCount);
			m_fragmentsListCreation->setVector<OITAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());
			m_fragmentsListCreation->setTexture<OITAppUniforms>(UF::ENVIRONMENT_MAP, m_skyboxTexture);
			m_fragmentsListCreation->setImage<OITAppUniforms>(UF::HEAD_BUFFER, m_headBuffer, 0);
			
			m_fragmentsCounter->bind(0);
			m_fragmentsBuffer->bind(1);

			framework::PipelineState cullingDisable(GL_CULL_FACE, false);
			cullingDisable.apply();
			framework::DepthState disableDepthWriting(true);
			disableDepthWriting.setWriteEnable(false);
			disableDepthWriting.apply();

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				if (m_entitiesData[i].transparent) renderEntity(m_fragmentsListCreation, m_entity, m_entitiesData[i]);
			}

			disableDepthWriting.cancel();
			cullingDisable.cancel();
		}

		// render transparent objects
		if (m_transparentRendering->use())
		{
			m_transparentRendering->setImage<OITAppUniforms>(UF::HEAD_BUFFER, m_headBuffer, 0);
			m_fragmentsBuffer->bind(0);

			framework::DepthState disableDepth(false);
			disableDepth.apply();
			framework::BlendState blendingEnable(true);
			blendingEnable.setBlending(GL_ONE, GL_SRC_ALPHA);
			blendingEnable.apply();

			glDrawArrays(GL_POINTS, 0, 1);
			
			disableDepth.cancel();
			blendingEnable.cancel();
		}

		// debug rendering
		renderDebug();

		CHECK_GL_ERROR;
	}

	void renderEntity(const std::shared_ptr<framework::GpuProgram>& program, const Entity& entity, const EntityData& entityData)
	{
		program->setMatrix<OITAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, entityData.mvp);
		program->setMatrix<OITAppUniforms>(UF::MODEL_MATRIX, entityData.model);

		entity.geometry->renderAllMeshes();
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		unsigned int bufferUsage = 0;//m_fragmentsBuffer->getActualSize();
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
	// atomic counter for fragments buffer
	std::shared_ptr<framework::AtomicCounter> m_fragmentsCounter;
	// buffer to store dynamic lists
	std::shared_ptr<framework::StorageBuffer> m_fragmentsBuffer;
	
	// gpu program to render opaque geometry
	std::shared_ptr<framework::GpuProgram> m_opaqueRendering;
	// gpu program to create fragments list
	std::shared_ptr<framework::GpuProgram> m_fragmentsListCreation;
	// gpu program to render transparent geometry by fragments list
	std::shared_ptr<framework::GpuProgram> m_transparentRendering;

	// pipeline stages
	//std::shared_ptr<framework::BlendStage> m_disableColorWriting;
	//std::shared_ptr<framework::DepthStencilStage> m_disableDepthWriting;
	//std::shared_ptr<framework::RasterizerStage> m_cullingOff;
	//std::shared_ptr<framework::BlendStage> m_alphaBlending;

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
		bool transparent;
		EntityData() : transparent(true) {}
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