#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(FurAppUniforms)
	MODELVIEWPROJECTION_MATRIX,
	MODEL_MATRIX,
	LIGHTS_DATA,
	VIEW_POSITION,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	FUR_MAP,
	FURLENGTH_MAP
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<FurAppUniforms>::Uniform

// constants
const std::string SHADERS_PATH = "data/shaders/gl/win32/fur/";
const int FUR_LAYERS = 16;
#define PROFILING 0

// application
class FurApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	FurApp()
	{
		m_renderDebug = false;
		m_renderFurShells = true;
		m_renderFurFins = true;
		m_catFinsIndexBuffer = 0;
		m_catFinsIndexBufferSize = 0;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Fur rendering (OpenGL 4)";
		applyStandardParams(params);

		setLegend("WASD - move camera\nLeft mouse button - rotate camera\nF1 - debug info\nF2 - on/off shells\nF3 - on/off fins");

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
		m_camera.setSpeed(10);

		// overlays
		initOverlays(root);

		// gpu programs
		m_solidRendering.reset(new framework::GpuProgram());
		m_solidRendering->addShader(SHADERS_PATH + "solid.vsh.glsl");
		m_solidRendering->addShader(SHADERS_PATH + "solid.fsh.glsl");
		if (!m_solidRendering->init()) exit();
		m_solidRendering->bindUniform<FurAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_solidRendering->bindStorageBuffer<FurAppUniforms>(UF::LIGHTS_DATA, "lightsDataBuffer");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_solidRendering->bindUniform<FurAppUniforms>(UF::SPECULAR_MAP, "specularMap");

		m_furShellsRendering.reset(new framework::GpuProgram());
		m_furShellsRendering->addShader(SHADERS_PATH + "furshells.vsh.glsl");
		m_furShellsRendering->addShader(SHADERS_PATH + "furshells.fsh.glsl");
		if (!m_furShellsRendering->init()) exit();
		m_furShellsRendering->bindUniform<FurAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_furShellsRendering->bindUniform<FurAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_furShellsRendering->bindStorageBuffer<FurAppUniforms>(UF::LIGHTS_DATA, "lightsDataBuffer");
		m_furShellsRendering->bindUniform<FurAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_furShellsRendering->bindUniform<FurAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_furShellsRendering->bindUniform<FurAppUniforms>(UF::FURLENGTH_MAP, "furLengthMap");
		m_furShellsRendering->bindUniform<FurAppUniforms>(UF::FUR_MAP, "furMap");

		m_furFinsRendering.reset(new framework::GpuProgram());
		m_furFinsRendering->addShader(SHADERS_PATH + "furfins.vsh.glsl");
		m_furFinsRendering->addShader(SHADERS_PATH + "furfins.gsh.glsl");
		m_furFinsRendering->addShader(SHADERS_PATH + "furfins.fsh.glsl");
		if (!m_furFinsRendering->init()) exit();
		m_furFinsRendering->bindUniform<FurAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_furFinsRendering->bindUniform<FurAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_furFinsRendering->bindUniform<FurAppUniforms>(UF::FURLENGTH_MAP, "furLengthMap");
		m_furFinsRendering->bindUniform<FurAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_furFinsRendering->bindUniform<FurAppUniforms>(UF::FUR_MAP, "furMap");

		// geometry
		m_catGeometry = initEntity("data/media/cat/cat.geom", true);

		// fins index buffer
		m_catFinsIndexBuffer = initFinsIndexBuffer(m_catGeometry, m_catFinsIndexBufferSize);
		if (m_catFinsIndexBuffer == 0) exit();

		m_furTexture.reset(new framework::Texture());
		std::vector<std::string> furTexturesArray;
		for (int i = 0; i < FUR_LAYERS; i++)
		{
			char buf[128];
			sprintf(buf, "data/media/cat/furlayers/FurLayer%02d.dds", i);
			furTexturesArray.push_back(buf);
		}
		if (!m_furTexture->initAsArray(furTexturesArray)) exit();

		m_furLengthTexture.reset(new framework::Texture());
		if (!m_furLengthTexture->init("data/media/cat/cat_furlen.dds")) exit();

		// entities
		const int ENTITIES_IN_ROW = 1;
		const float HALF_ENTITIES_IN_ROW = float(ENTITIES_IN_ROW) * 0.5f;
		const float AREA_HALFLENGTH = 8.0f;

		if (ENTITIES_IN_ROW != 1)
		{
			for (int i = 0; i < ENTITIES_IN_ROW; i++)
			{
				for (int j = 0; j < ENTITIES_IN_ROW; j++)
				{
					float x = (float(i) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;
					float z = (float(j) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;

					m_entitiesData.push_back(EntityData());
					int index = (int)m_entitiesData.size() - 1;
					m_entitiesData[index].geometry = m_catGeometry;
					m_entitiesData[index].finsIndexBuffer = m_catFinsIndexBuffer;
					m_entitiesData[index].finsIndexBufferSize = m_catFinsIndexBufferSize;

					quaternion quat;	
					quat.set_rotate_y(n_deg2rad(utils::Utils::random(-180.0f, -120.0f).x));
					m_entitiesData[index].model = matrix44(quat);
					m_entitiesData[index].model.set_translation(vector3(x * AREA_HALFLENGTH, 0, z * AREA_HALFLENGTH));
				}
			}
		}
		else
		{
			m_entitiesData.push_back(EntityData());
			int index = (int)m_entitiesData.size() - 1;
			m_entitiesData[index].geometry = m_catGeometry;
			m_entitiesData[index].finsIndexBuffer = m_catFinsIndexBuffer;
			m_entitiesData[index].finsIndexBufferSize = m_catFinsIndexBufferSize;

			quaternion quat;	
			quat.set_rotate_y(n_deg2rad(-180.0));
			m_entitiesData[index].model = matrix44(quat);
		}

		// lights
		initLights();

		// skybox texture
		m_skyboxTexture.reset(new framework::Texture());
		if (!m_skyboxTexture->initAsCubemap("data/media/textures/church_posz.jpg",
											"data/media/textures/church_negz.jpg",
											"data/media/textures/church_negx.jpg",
											"data/media/textures/church_posx.jpg",
											"data/media/textures/church_posy.jpg",
											"data/media/textures/church_negy.jpg", true)) exit();

		framework::DepthState depthTestEnable(true);
		depthTestEnable.setWriteEnable(true);
		depthTestEnable.apply();

		framework::PipelineState cullingEnable(GL_CULL_FACE, true);
		cullingEnable.apply();

		glViewport(0, 0, m_info.windowWidth, m_info.windowHeight);
	}

	std::shared_ptr<framework::Geometry3D> initEntity(const std::string& geometry, bool calculateAdjacency)
	{
		std::shared_ptr<framework::Geometry3D> ent(new framework::Geometry3D());
		if (!ent->init(geometry, calculateAdjacency))
		{
			exit();
		}
		else
		{
			framework::MaterialManager::instance().initializeMaterial(ent);
		}

		return ent;
	}

	unsigned int initFinsIndexBuffer(const std::shared_ptr<framework::Geometry3D>& geometry, size_t& size)
	{
		auto adjacency = geometry->getAdjacency();
		if (!adjacency.empty())
		{
			// calculate fins data
			std::vector<int> finsData;
			finsData.reserve(adjacency.size() * 3 * 4);
			for (size_t i = 0; i < adjacency.size(); i++)
			{
				if (adjacency[i].adjacentPoints[0] != -1)
				{
					finsData.push_back(adjacency[i].adjacentPoints[0]);
					finsData.push_back(adjacency[i].points[0]);
					finsData.push_back(adjacency[i].points[1]);
					finsData.push_back(adjacency[i].points[2]);
				}
				if (adjacency[i].adjacentPoints[1] != -1)
				{
					finsData.push_back(adjacency[i].adjacentPoints[1]);
					finsData.push_back(adjacency[i].points[1]);
					finsData.push_back(adjacency[i].points[2]);
					finsData.push_back(adjacency[i].points[0]);
				}
				if (adjacency[i].adjacentPoints[2] != -1)
				{
					finsData.push_back(adjacency[i].adjacentPoints[2]);
					finsData.push_back(adjacency[i].points[2]);
					finsData.push_back(adjacency[i].points[0]);
					finsData.push_back(adjacency[i].points[1]);
				}
			}

			// create buffer
			unsigned int indexBuffer = 0;
			glGenBuffers(1, &indexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, finsData.size() * sizeof(int), finsData.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			size = finsData.size();

			if (!CHECK_GL_ERROR) return indexBuffer;
		}
		
		size = 0;
		return 0;
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
		if (!m_lightsBuffer->initStorage<framework::LightRawData>(1)) exit();
		m_lightsBuffer->setElement(0, m_lightManager.getRawLightData(0));
	}

	void initOverlays(gui::WidgetPtr_T root)
	{
		m_debugLabel = framework::UIFactory::createLabel(gui::Coords(1.0f, -500.0f, 1.0f, -300.0f),
														 gui::Coords::Absolute(500.0f, 300.0f),
														 gui::RightAligned, gui::BottomAligned);
		root->addChild(m_debugLabel);
		m_debugLabel->setVisible(m_renderDebug);

		m_fpsLabel->setColor(vector4(1.0f, 1.0f, 0.0f, 1.0f));
		m_legendLabel->setColor(vector4(1.0f, 1.0f, 0.0f, 1.0f));
		m_debugLabel->setColor(vector4(1.0f, 1.0f, 0.0f, 1.0f));
	}

	virtual void shutdown()
	{
		if (m_catFinsIndexBuffer != 0)
		{
			glDeleteBuffers(1, &m_catFinsIndexBuffer);
			m_catFinsIndexBuffer = 0;
		}

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

		glViewport(0, 0, m_info.windowWidth, m_info.windowHeight);
		useDefaultRenderTarget();

		// render skybox
		renderSkybox(m_camera, m_skyboxTexture);

		// parts without fur
		if (m_solidRendering->use())
		{
			m_solidRendering->setVector<FurAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());
			m_solidRendering->setStorageBuffer<FurAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer, 0);

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderGeometry(m_entitiesData[i].geometry.lock(), m_entitiesData[i]);
			}
		}

		framework::BlendState blendingEnable(true);
		blendingEnable.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		blendingEnable.apply();
		framework::DepthState disableDepthWriting(true);
		disableDepthWriting.setWriteEnable(false);
		disableDepthWriting.apply();
	
		// fur (fins)
		if (m_renderFurFins && m_furFinsRendering->use())
		{
			m_furFinsRendering->setVector<FurAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderFurFins(m_entitiesData[i].geometry.lock(), m_entitiesData[i]);
			}
		}

		// fur (shells)
		if (m_renderFurShells && m_furShellsRendering->use())
		{
			m_furShellsRendering->setVector<FurAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());
			m_furShellsRendering->setStorageBuffer<FurAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer, 0);

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderFurShells(m_entitiesData[i].geometry.lock(), m_entitiesData[i]);
			}
		}

		blendingEnable.cancel();
		disableDepthWriting.cancel();

		renderDebug();
	}

	void renderGeometry(const std::shared_ptr<framework::Geometry3D>& geometry, const EntityData& entityData)
	{
		m_solidRendering->setMatrix<FurAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, entityData.mvp);
		m_solidRendering->setMatrix<FurAppUniforms>(UF::MODEL_MATRIX, entityData.model);

		for (size_t i = 0; i < geometry->getMeshesCount(); i++)
		{
			auto diffMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_DIFFUSE_MAP);
			auto normMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_NORMAL_MAP);
			auto specMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_SPECULAR_MAP);
			m_solidRendering->setTexture<FurAppUniforms>(UF::DIFFUSE_MAP, diffMap, 0);
			m_solidRendering->setTexture<FurAppUniforms>(UF::NORMAL_MAP, normMap, 1);
			m_solidRendering->setTexture<FurAppUniforms>(UF::SPECULAR_MAP, specMap, 2);

			geometry->renderMesh(i);
		}
	}

	void renderFurShells(const std::shared_ptr<framework::Geometry3D>& geometry, const EntityData& entityData)
	{
		const int index = 1;
		if (index >= geometry->getMeshesCount()) return;

		m_furShellsRendering->setMatrix<FurAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, entityData.mvp);
		m_furShellsRendering->setMatrix<FurAppUniforms>(UF::MODEL_MATRIX, entityData.model);

		auto diffMap = framework::MaterialManager::instance().getTexture(geometry, index, framework::MAT_DIFFUSE_MAP);
		m_furShellsRendering->setTexture<FurAppUniforms>(UF::FURLENGTH_MAP, m_furLengthTexture);
		m_furShellsRendering->setTexture<FurAppUniforms>(UF::DIFFUSE_MAP, diffMap);
		m_furShellsRendering->setTexture<FurAppUniforms>(UF::FUR_MAP, m_furTexture);

		geometry->renderMesh(index, FUR_LAYERS);
	}

	void renderFurFins(const std::shared_ptr<framework::Geometry3D>& geometry, const EntityData& entityData)
	{
		const int index = 1;
		if (index >= geometry->getMeshesCount()) return;

		m_furFinsRendering->setMatrix<FurAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, entityData.mvp);

		auto diffMap = framework::MaterialManager::instance().getTexture(geometry, index, framework::MAT_DIFFUSE_MAP);
		m_furFinsRendering->setTexture<FurAppUniforms>(UF::FURLENGTH_MAP, m_furLengthTexture);
		m_furFinsRendering->setTexture<FurAppUniforms>(UF::DIFFUSE_MAP, diffMap);
		m_furFinsRendering->setTexture<FurAppUniforms>(UF::FUR_MAP, m_furTexture);

		glBindVertexArray(geometry->getVertexArray());
		glBindBuffer(GL_ARRAY_BUFFER, geometry->getVertexBuffer());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entityData.finsIndexBuffer);
		glDrawElements(GL_LINES_ADJACENCY, entityData.finsIndexBufferSize, GL_UNSIGNED_INT, 0);
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
		if (key == InputKeys::F2 && pressed)
		{
			m_renderFurShells = !m_renderFurShells;
			return;
		}
		if (key == InputKeys::F3 && pressed)
		{
			m_renderFurFins = !m_renderFurFins;
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
	// gpu program to render shells
	std::shared_ptr<framework::GpuProgram> m_furShellsRendering;
	// gpu program to render fins
	std::shared_ptr<framework::GpuProgram> m_furFinsRendering;

	std::shared_ptr<framework::Geometry3D> m_catGeometry;
	unsigned int m_catFinsIndexBuffer;
	size_t m_catFinsIndexBufferSize;

	struct EntityData
	{
		std::weak_ptr<framework::Geometry3D> geometry;
		unsigned int finsIndexBuffer;
		size_t finsIndexBufferSize;
		matrix44 model;
		matrix44 mvp;

		EntityData() : finsIndexBuffer(0), finsIndexBufferSize(0) {}
	};

	std::vector<EntityData> m_entitiesData;

	std::shared_ptr<framework::UniformBuffer> m_entityDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_onFrameDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;

	std::shared_ptr<framework::Texture> m_furLengthTexture;
	std::shared_ptr<framework::Texture> m_furTexture;

	framework::FreeCamera m_camera;

	bool m_renderDebug;
	gui::LabelPtr_T m_debugLabel;
	bool m_renderFurShells;
	bool m_renderFurFins;

	std::shared_ptr<framework::Texture> m_skyboxTexture;
};

DECLARE_MAIN(FurApp);