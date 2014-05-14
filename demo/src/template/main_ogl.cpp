#include "framework.h"

// uniforms
DECLARE_UNIFORMS_BEGIN(TestAppUniforms)
	MODELVIEWPROJECTION_MATRIX,
	MODEL_MATRIX,
	DIFFUSE_MAP,
	NORMAL_MAP,
	SPECULAR_MAP,
	VIEW_POSITION,
	LIGHTS_DATA_BUFFER
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<TestAppUniforms>::Uniform

// constants
const int MAX_LIGHTS_COUNT = 16;
const std::string SHADERS_PATH = "data/shaders/gl/win32/template/";

// application
class TestApp : public framework::Application
{
public:
	TestApp(){}
	virtual ~TestApp(){}

	virtual void init(const std::map<std::string, int>& params)
	{
		m_info.title = "Test application (OpenGL 4)";
		m_info.flags.fullscreen = 0;
		m_info.samples = 4;
	}

	virtual void startup(gui::WidgetPtr_T root)
	{
		m_rotation = 0.0f;

		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

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
		m_program->bindUniform<TestAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, "modelViewProjectionMatrix");
		m_program->bindUniform<TestAppUniforms>(UF::MODEL_MATRIX, "modelMatrix");
		m_program->bindUniform<TestAppUniforms>(UF::DIFFUSE_MAP, "diffuseSampler");
		m_program->bindUniform<TestAppUniforms>(UF::NORMAL_MAP, "normalSampler");
		m_program->bindUniform<TestAppUniforms>(UF::SPECULAR_MAP, "specularSampler");
		m_program->bindUniform<TestAppUniforms>(UF::VIEW_POSITION, "viewPosition");
		m_program->bindUniformBuffer<TestAppUniforms>(UF::LIGHTS_DATA_BUFFER, "lightsDataBuffer");

		// lights
		framework::LightSource source;
		source.type = framework::LightType::DirectLight;
		source.position = vector3(0, 15, 0);
		vector3 dir(1, -1, 1);
		dir.norm();
		source.orientation.set_from_axes(vector3(0, 0, 1), dir);
		source.diffuseColor = vector3(1.0f, 1.0f, 1.0f);
		source.specularColor = vector3(0.3f, 0.3f, 0.3f);
		source.ambientColor = vector3(0.1f, 0.1f, 0.1f);
		m_lightManager.addLightSource(source);

		m_lightsBuffer.reset(new framework::UniformBuffer());
		if (!m_lightsBuffer->init<framework::LightRawData>((size_t)MAX_LIGHTS_COUNT)) exit();

		int lightsCount = std::min((int)m_lightManager.getLightSourcesCount(), MAX_LIGHTS_COUNT);
		for (int i = 0; i < lightsCount; i++)
		{
			m_lightsBuffer->setElement(i, m_lightManager.getRawLightData(i));
		}

		framework::PipelineState depthTestEnable(GL_DEPTH_TEST, true);
		depthTestEnable.apply();

		framework::PipelineState cullingEnable(GL_CULL_FACE, true);
		cullingEnable.apply();
	}

	virtual void shutdown()
	{
	}

	virtual void onResize(int width, int height)
	{
		glViewport(0, 0, width, height);
		m_camera.updateResolution(width, height);
	}

	virtual void render(double elapsedTime)
	{
		m_camera.update(elapsedTime);

		float aspect = fabsf((float)m_info.windowWidth / (float)m_info.windowHeight);

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
			m_program->setMatrix<TestAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, m_mvp);
			m_program->setMatrix<TestAppUniforms>(UF::MODEL_MATRIX, model);
			m_program->setVector<TestAppUniforms>(UF::VIEW_POSITION, m_camera.getPosition());
			m_program->setUniformBuffer<TestAppUniforms>(UF::LIGHTS_DATA_BUFFER, *m_lightsBuffer, 0);

			m_texture->setToSampler(m_program->getUniform<TestAppUniforms>(UF::DIFFUSE_MAP));
			m_normalTexture->setToSampler(m_program->getUniform<TestAppUniforms>(UF::NORMAL_MAP));
			m_specularTexture->setToSampler(m_program->getUniform<TestAppUniforms>(UF::SPECULAR_MAP));

			m_geometry->renderAllMeshes();
		}

		//m_geometry->renderBoundingBox(m_mvp);

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		renderAxes(vp);
		m_lightManager.renderDebugVisualization(vp);
	}

	virtual void onKeyButton(int key, int scancode, bool pressed)
	{
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

private:
	std::shared_ptr<framework::GpuProgram> m_program;
	std::shared_ptr<framework::Geometry3D> m_geometry;
	std::shared_ptr<framework::Texture> m_texture;
	std::shared_ptr<framework::Texture> m_normalTexture;
	std::shared_ptr<framework::Texture> m_specularTexture;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;

	framework::FreeCamera m_camera;
	matrix44 m_mvp;

	float m_rotation;
};

DECLARE_MAIN(TestApp);