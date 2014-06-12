#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(PSSMAppUniforms)
	MODELVIEWPROJECTION_MATRIX,
	MODEL_MATRIX,
	SHADOW_INDICES,
	LIGHTS_DATA,
	VIEW_POSITION,
	SPLIT_COUNT,
	SHADOWVIEWPROJECTION_MATRICES,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	SHADOW_MAP,
	SHADOW_BLUR_STEP
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<PSSMAppUniforms>::Uniform

// constants
const std::string SHADERS_PATH = "data/shaders/gl/win32/pssm/";
const int MAX_SPLITS = 4;
#define PROFILING 0

// application
class PSSMApp : public framework::Application
{
	struct Entity;
	struct EntityData;

public:
	PSSMApp()
	{
		m_shadowMapSize = 1024;
		m_splitCount = MAX_SPLITS;
		m_currentSplitCount = m_splitCount;
		m_splitLambda = 0.5f;
		m_farPlane = 0;
		m_splitShift = 20;
		m_shadowBlurStep = 1.0f / float(m_shadowMapSize);
		m_furthestPointInCamera = 0;

		m_renderDebug = false;
		m_increaseBlurStep = false;
		m_decreaseBlurStep = false;
	}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Parallel-Split Shadow Mapping (OpenGL 4)";

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
		if (m_splitCount > 1) m_maxSplitDistances.resize(m_splitCount - 1);
		m_currentSplitCount = m_splitCount;

		m_shadowBlurStep = 1.0f / float(m_shadowMapSize);

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
		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(158.0f, 22.0f, -161.0f), vector3(157.27f, 22.0f, -160.32f));
		m_camera.getInternalCamera().SetFarPlane(2500.0f);

		// overlays
		initOverlays(root);

		// shadow map (depth only)
		m_shadowMap.reset(new framework::RenderTarget());
		if (!m_shadowMap->initArray(m_splitCount, m_shadowMapSize, m_shadowMapSize, -1, 0, GL_DEPTH_COMPONENT32F)) exit();
		const float BORDER_COLOR[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glBindTexture(m_shadowMap->getTargetType(), m_shadowMap->getDepthBuffer());
		glTexParameteri(m_shadowMap->getTargetType(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(m_shadowMap->getTargetType(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(m_shadowMap->getTargetType(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(m_shadowMap->getTargetType(), GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		glTexParameteri(m_shadowMap->getTargetType(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(m_shadowMap->getTargetType(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(m_shadowMap->getTargetType(), GL_TEXTURE_BORDER_COLOR, BORDER_COLOR);
		glBindTexture(m_shadowMap->getTargetType(), 0);

		// gpu programs
		m_sceneRendering.reset(new framework::GpuProgram());
		m_sceneRendering->addShader(SHADERS_PATH + "scene.vsh.glsl");
		m_sceneRendering->addShader(SHADERS_PATH + "scene.fsh.glsl");
		if (!m_sceneRendering->init()) exit();
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_sceneRendering->bindStorageBuffer<PSSMAppUniforms>(UF::LIGHTS_DATA, "lightsDataBuffer");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::SPLIT_COUNT, "splitsCount");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::SHADOWVIEWPROJECTION_MATRICES, "shadowViewProjection");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::DIFFUSE_MAP, "diffuseMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::NORMAL_MAP, "normalMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::SPECULAR_MAP, "specularMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::SHADOW_MAP, "shadowMap");
		m_sceneRendering->bindUniform<PSSMAppUniforms>(UF::SHADOW_BLUR_STEP, "shadowBlurStep");

		m_shadowMapRendering.reset(new framework::GpuProgram());
		m_shadowMapRendering->addShader(SHADERS_PATH + "shadowmap.vsh.glsl");
		m_shadowMapRendering->addShader(SHADERS_PATH + "shadowmap.gsh.glsl");
		if (!m_shadowMapRendering->init()) exit();
		m_shadowMapRendering->bindUniform<PSSMAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_shadowMapRendering->bindUniform<PSSMAppUniforms>(UF::SHADOWVIEWPROJECTION_MATRICES, "shadowViewProjection");
		m_shadowMapRendering->bindUniform<PSSMAppUniforms>(UF::SHADOW_INDICES, "shadowIndices");

		// geometry
		m_windmillGeometry = initEntity("data/media/windmill/windmill.geom");
		m_houseGeometry = initEntity("data/media/house/house.geom");

		geom::TerrainGenerationInfo terrainInfo;
		terrainInfo.heightmap = framework::LoadHeightmapData("data/media/textures/heightmap1.png", terrainInfo.heightmapWidth, terrainInfo.heightmapHeight);
		terrainInfo.size = vector3(5000.0f, 5000.0f, 500.0f);
		terrainInfo.uvSize = vector2(200.0f, 200.0f);
		m_terrainGeometry = initEntity(terrainInfo, "data/media/textures/grass.dds", "data/media/textures/grass_bump.dds", "data/media/textures/no_specular.png");

		// entities
		vector3 GROUP_OFFSET[] = { vector3(0, 0, 0), vector3(-1000, 130, 600), vector3(0, -4, 900) };
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
		}

		EntityData terrainData;
		terrainData.geometry = m_terrainGeometry;
		terrainData.model.ident();
		terrainData.isShadowCaster = false;
		m_entitiesData.push_back(terrainData);
		
		// lights
		initLights();

		// skybox texture
		m_skyboxTexture.reset(new framework::Texture());
		m_skyboxTexture.reset(new framework::Texture());
		if (!m_skyboxTexture->initAsCubemap("data/media/textures/meadow_front.jpg",
											"data/media/textures/meadow_back.jpg",
											"data/media/textures/meadow_left.jpg",
											"data/media/textures/meadow_right.jpg",
											"data/media/textures/meadow_top.jpg",
											"data/media/textures/meadow_bottom.jpg", true)) exit();

		framework::DepthState depthTestEnable(true);
		depthTestEnable.setWriteEnable(true);
		depthTestEnable.apply();

		framework::PipelineState cullingEnable(GL_CULL_FACE, true);
		cullingEnable.apply();

		glViewport(0, 0, m_info.windowWidth, m_info.windowHeight);
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

		// set viewport for shadow map rendering
		glViewport(0, 0, m_shadowMapSize, m_shadowMapSize);

		// render shadow map
		m_shadowMap->set();
		m_shadowMap->clearDepth();
		if (m_shadowMapRendering->use())
		{
			m_shadowMapRendering->setMatrixArray<PSSMAppUniforms>(UF::SHADOWVIEWPROJECTION_MATRICES, m_shadowViewProjection, MAX_SPLITS);

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderGeometry(true, m_entitiesData[i].geometry.lock(), m_entitiesData[i]);
			}
		}

		// render scene
		glViewport(0, 0, m_info.windowWidth, m_info.windowHeight);
		useDefaultRenderTarget();

		// render skybox
		renderSkybox(m_camera, m_skyboxTexture);

		// render scene
		if (m_sceneRendering->use())
		{
			m_sceneRendering->setVector<PSSMAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());	
			m_sceneRendering->setInt<PSSMAppUniforms>(UF::SPLIT_COUNT, m_currentSplitCount);
			m_sceneRendering->setStorageBuffer<PSSMAppUniforms>(UF::LIGHTS_DATA, m_lightsBuffer, 0);
			m_sceneRendering->setDepth<PSSMAppUniforms>(UF::SHADOW_MAP, m_shadowMap, 0);
			m_sceneRendering->setMatrixArray<PSSMAppUniforms>(UF::SHADOWVIEWPROJECTION_MATRICES, m_shadowViewProjection, MAX_SPLITS);
			m_sceneRendering->setFloat<PSSMAppUniforms>(UF::SHADOW_BLUR_STEP, m_shadowBlurStep);

			for (size_t i = 0; i < m_entitiesData.size(); i++)
			{
				renderGeometry(false, m_entitiesData[i].geometry.lock(), m_entitiesData[i]);
			}
		}

		renderDebug();

		//CHECK_GL_ERROR;
	}

	void renderGeometry(bool shadowmap, const std::shared_ptr<framework::Geometry3D>& geometry, const EntityData& entityData)
	{
		if (shadowmap && (!entityData.isShadowCaster || entityData.shadowInstancesCount == 0)) return;

		if (shadowmap)
		{
			m_shadowMapRendering->setMatrix<PSSMAppUniforms>(UF::MODEL_MATRIX, entityData.model);
			m_shadowMapRendering->setIntArray<PSSMAppUniforms>(UF::SHADOW_INDICES, (int*)&entityData.shadowIndices[0], MAX_SPLITS);
		}
		else
		{
			m_sceneRendering->setMatrix<PSSMAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, entityData.mvp);
			m_sceneRendering->setMatrix<PSSMAppUniforms>(UF::MODEL_MATRIX, entityData.model);
		}
		
		for (size_t i = 0; i < geometry->getMeshesCount(); i++)
		{
			if (!shadowmap)
			{
				auto diffMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_DIFFUSE_MAP);
				auto normMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_NORMAL_MAP);
				auto specMap = framework::MaterialManager::instance().getTexture(geometry, i, framework::MAT_SPECULAR_MAP);
				m_sceneRendering->setTexture<PSSMAppUniforms>(UF::DIFFUSE_MAP, diffMap, 1);
				m_sceneRendering->setTexture<PSSMAppUniforms>(UF::NORMAL_MAP, normMap, 2);
				m_sceneRendering->setTexture<PSSMAppUniforms>(UF::SPECULAR_MAP, specMap, 3);
			}

			geometry->renderMesh(i, shadowmap ? entityData.shadowInstancesCount : 1);
		}
	}

	void renderDebug()
	{
		if (!m_renderDebug) return;

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		//renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);

		std::wstringstream stream;
		stream.precision(2);
		stream << "Furthest point = " << std::fixed << m_furthestPointInCamera << "\nDistances = ";
		for (int i = 0; i < m_currentSplitCount + 1; i++)
		{
			stream << std::fixed << m_splitDistances[i];
			if (i != m_currentSplitCount)
			{	
				if (i % 2 == 0 && i != 0) stream << ",\n"; else stream << ", ";
			}
		}

		int objectsCount = 0;
		int instancesCount = 0;
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			if (m_entitiesData[i].shadowInstancesCount > 0)
			{
				objectsCount++;
				instancesCount += (m_entitiesData[i].shadowInstancesCount - 1);
			}
		}
		stream << "\nRendered to SM objects = " << objectsCount << "\nRendered to SM instances = " << instancesCount;

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
		if (key == InputKeys::Equals)
		{
			m_increaseBlurStep = pressed;
			return;
		}
		else if (key == InputKeys::Minus)
		{
			m_decreaseBlurStep = pressed;
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
			m_entitiesData[i].shadowInstancesCount = 0;
		}

		// calculate shadow
		calculateMaxSplitDistances();
		m_furthestPointInCamera = calculateFurthestPointInCamera(cameraView);
		calculateSplitDistances();
		for (int i = 0; i < m_currentSplitCount; i++)
		{
			float n = m_splitDistances[i];
			float f = m_splitDistances[i + 1];
			bbox3 box = calculateFrustumBox(n, f);
			m_shadowViewProjection[i] = calculateShadowViewProjection(box);
			updateShadowVisibilityMask(box, i);
		}

		if (m_increaseBlurStep)
		{
			m_shadowBlurStep += 2.0f * float(elapsedTime) / float(m_shadowMapSize);
			const float MAX_SHADOW_BLUR = 10.0f / float(m_shadowMapSize);
			if (m_shadowBlurStep > MAX_SHADOW_BLUR) m_shadowBlurStep = MAX_SHADOW_BLUR;
		}
		else if (m_decreaseBlurStep)
		{
			m_shadowBlurStep -= 2.0f * float(elapsedTime) / float(m_shadowMapSize);
			const float MIN_SHADOW_BLUR = 1.0f / float(m_shadowMapSize);
			if (m_shadowBlurStep < MIN_SHADOW_BLUR) m_shadowBlurStep = MIN_SHADOW_BLUR;
		}
	}

	void calculateMaxSplitDistances()
	{
		float nearPlane = m_camera.getInternalCamera().GetNearPlane();
		float farPlane = m_camera.getInternalCamera().GetFarPlane();
		for (int i = 1; i < m_splitCount; i++)
		{
			float f = (float)i / (float)m_splitCount;
			float l = nearPlane * pow(farPlane / nearPlane, f);
			float u = nearPlane + (farPlane - nearPlane) * f;
			m_maxSplitDistances[i - 1] = l * m_splitLambda + u * (1.0f - m_splitLambda);
		}

		m_farPlane = farPlane + m_splitShift;
	}

	float calculateFurthestPointInCamera(const matrix44& cameraView)
	{
		bbox3 scenebox;
		scenebox.begin_extend();
		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			if (m_entitiesData[i].isShadowCaster)
			{
				bbox3 b = m_entitiesData[i].geometry.lock()->getBoundingBox();
				b.transform(m_entitiesData[i].model);
				scenebox.extend(b);
			}
		}
		scenebox.end_extend();

		float maxZ = m_camera.getInternalCamera().GetNearPlane();
		for(int i = 0; i < 8; i++)
		{
			vector3 corner = scenebox.corner_point(i);
			float z = -cameraView.transform_coord(corner).z;
			if(z > maxZ) maxZ = z;
		}
		return std::min(maxZ, m_farPlane);
	}

	void calculateSplitDistances()
	{
		// calculate how many shadow maps do we really need
		m_currentSplitCount = 1;
		if (!m_maxSplitDistances.empty())
		{
			for (size_t i = 0; i < m_maxSplitDistances.size(); i++)
			{
				float d = m_maxSplitDistances[i] - m_splitShift;
				if (m_furthestPointInCamera >= d) m_currentSplitCount++;
			}
		}

		float nearPlane = m_camera.getInternalCamera().GetNearPlane();
		for (int i = 0; i < m_currentSplitCount; i++)
		{
			float f = (float)i / (float)m_currentSplitCount;
			float l = nearPlane * pow(m_furthestPointInCamera / nearPlane, f);
			float u = nearPlane + (m_furthestPointInCamera - nearPlane) * f;
			m_splitDistances[i] = l * m_splitLambda + u * (1.0f - m_splitLambda);
		}

		m_splitDistances[0] = nearPlane;
		m_splitDistances[m_currentSplitCount] = m_furthestPointInCamera;
	}

	matrix44 calculateShadowViewProjection(const bbox3& frustumBox)
	{
		const float LIGHT_SOURCE_HEIGHT = 500.0f;

		vector3 viewDir = m_camera.getOrientation().z_direction();
		vector3 size = frustumBox.size();
		vector3 center = frustumBox.center() - viewDir * m_splitShift;
		center.y = 0;

		auto lightSource = m_lightManager.getLightSource(0);
		vector3 lightDir = lightSource.orientation.z_direction();

		matrix44 shadowView;
		shadowView.pos_component() = center - lightDir * LIGHT_SOURCE_HEIGHT;
		shadowView.lookatRh(shadowView.pos_component() + lightDir, lightSource.orientation.y_direction());
		shadowView.invert_simple();

		matrix44 shadowProj;	
		float d = std::max(size.x, size.z);
		shadowProj.orthoRh(d, d, 0.1f, 2000.0f);

		return shadowView * shadowProj;
	}

	bbox3 calculateFrustumBox(float nearPlane, float farPlane)
	{
		vector3 eye = m_camera.getPosition();
		vector3 vZ = m_camera.getOrientation().z_direction();
		vector3 vX = m_camera.getOrientation().x_direction();
		vector3 vY = m_camera.getOrientation().y_direction();
		float fov = n_deg2rad(m_camera.getInternalCamera().GetAngleOfView());
		float aspect = m_camera.getInternalCamera().GetAspectRatio();

		float nearPlaneHeight = n_tan(fov * 0.5f) * nearPlane;
		float nearPlaneWidth = nearPlaneHeight * aspect;
		float farPlaneHeight = n_tan(fov * 0.5f) * farPlane;
		float farPlaneWidth = farPlaneHeight * aspect;
		vector3 nearPlaneCenter = eye + vZ * nearPlane;
		vector3 farPlaneCenter = eye + vZ * farPlane;

		bbox3 box;
		box.begin_extend();
		box.extend(vector3(nearPlaneCenter - vX * nearPlaneWidth - vY * nearPlaneHeight));
		box.extend(vector3(nearPlaneCenter - vX * nearPlaneWidth + vY * nearPlaneHeight));
		box.extend(vector3(nearPlaneCenter + vX * nearPlaneWidth + vY * nearPlaneHeight));
		box.extend(vector3(nearPlaneCenter + vX * nearPlaneWidth - vY * nearPlaneHeight));
		box.extend(vector3(farPlaneCenter - vX * farPlaneWidth - vY * farPlaneHeight));
		box.extend(vector3(farPlaneCenter - vX * farPlaneWidth + vY * farPlaneHeight));
		box.extend(vector3(farPlaneCenter + vX * farPlaneWidth + vY * farPlaneHeight));
		box.extend(vector3(farPlaneCenter + vX * farPlaneWidth - vY * farPlaneHeight));
		box.end_extend();
		return box;
	}

	void updateShadowVisibilityMask(const bbox3& frustumBox, int splitIndex)
	{
		vector3 viewDir = m_camera.getOrientation().z_direction();
		matrix44 boxOffset;
		boxOffset.set_translation(-viewDir * m_splitShift);
		bbox3 box = frustumBox;
		box.transform(boxOffset);

		for (size_t i = 0; i < m_entitiesData.size(); i++)
		{
			if (m_entitiesData[i].isShadowCaster)
			{
				updateShadowVisibilityMask(box, m_entitiesData[i].geometry.lock(), m_entitiesData[i], splitIndex);
			}
		}
	}

	void updateShadowVisibilityMask(const bbox3& frustumBox, const std::shared_ptr<framework::Geometry3D>& entity, EntityData& entityData, int splitIndex)
	{
		bbox3 b = entity->getBoundingBox();
		b.transform(entityData.model);

		// shadow box computation
		auto lightSource = m_lightManager.getLightSource(0);
		vector3 lightDir = lightSource.orientation.z_direction();
		float shadowBoxL = fabs(lightDir.z) < 1e-5 ? 1000.0f : (b.size().y / -lightDir.z);
		bbox3 shadowBox;
		shadowBox.begin_extend();
		for (int i = 0; i < 8; i++)
		{
			shadowBox.extend(b.corner_point(i));
			shadowBox.extend(b.corner_point(i) + lightDir * shadowBoxL);
		}
		shadowBox.end_extend();
	
		if (frustumBox.clipstatus(shadowBox) != bbox3::Outside)
		{
			int i = entityData.shadowInstancesCount;
			entityData.shadowIndices[i] = splitIndex;
			entityData.shadowInstancesCount++;
		}
	}

private:
	// gpu program to render scene
	std::shared_ptr<framework::GpuProgram> m_sceneRendering;
	// gpu program to render shadow map
	std::shared_ptr<framework::GpuProgram> m_shadowMapRendering;

	// shadow map
	std::shared_ptr<framework::RenderTarget> m_shadowMap;

	std::shared_ptr<framework::Geometry3D> m_houseGeometry;
	std::shared_ptr<framework::Geometry3D> m_windmillGeometry;
	std::shared_ptr<framework::Geometry3D> m_terrainGeometry;

	struct EntityData
	{
		std::weak_ptr<framework::Geometry3D> geometry;
		matrix44 model;
		matrix44 mvp;
		unsigned int shadowInstancesCount;
		int shadowIndices[MAX_SPLITS];
		bool isShadowCaster;
		EntityData() : isShadowCaster(true), shadowInstancesCount(0)
		{
			memset(shadowIndices, 0, sizeof(int) * MAX_SPLITS);
		}
	};
	
	std::vector<EntityData> m_entitiesData;

	std::shared_ptr<framework::UniformBuffer> m_entityDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_onFrameDataBuffer;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;
	std::shared_ptr<framework::UniformBuffer> m_shadowBuffer;

	framework::FreeCamera m_camera;

	int m_shadowMapSize;
	std::vector<float> m_maxSplitDistances;
	float m_farPlane;
	float m_splitShift;
	int m_splitCount;
	int m_currentSplitCount;
	float m_splitLambda;
	std::vector<float> m_splitDistances;
	float m_furthestPointInCamera;
	matrix44 m_shadowViewProjection[MAX_SPLITS];
	float m_shadowBlurStep;

	bool m_renderDebug;
	gui::LabelPtr_T m_debugLabel;
	bool m_increaseBlurStep;
	bool m_decreaseBlurStep;

	std::shared_ptr<framework::Texture> m_skyboxTexture;
};

DECLARE_MAIN(PSSMApp);