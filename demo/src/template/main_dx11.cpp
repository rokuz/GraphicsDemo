#include "application.h"

DECLARE_UNIFORMS_BEGIN(TestAppUniforms)
	SPACE_DATA,
	TEXTURES_DATA,
	LIGHTS_DATA
DECLARE_UNIFORMS_END()
#define UF framework::UniformBase<TestAppUniforms>::Uniform

const int MAX_LIGHTS_COUNT = 16;

class TestApp : public framework::Application
{
public:
	TestApp(){}
	virtual ~TestApp(){}

	virtual void init()
	{
		m_info.title = "Template application (DX11)";
		m_info.samples = 0;
		m_info.flags.fullscreen = 0;
	}

	virtual void startup(CEGUI::DefaultWindow* root)
	{
		m_rotation = 0.0f;

		m_camera.initWithPositionDirection(m_info.windowWidth, m_info.windowHeight, vector3(0, 50, -100), vector3());

		/*m_geometry.reset(new framework::Geometry3D());
		m_geometry->init("data/media/spaceship/spaceship.geom");

		m_texture.reset(new framework::Texture());
		m_texture->initWithKtx("data/media/spaceship/spaceship_diff.ktx");

		m_specularTexture.reset(new framework::Texture());
		m_specularTexture->initWithKtx("data/media/spaceship/spaceship_specular.ktx");

		m_normalTexture.reset(new framework::Texture());
		m_normalTexture->initWithKtx("data/media/spaceship/spaceship_normal.ktx");*/

		m_program.reset(new framework::GpuProgram());
		//m_program->initWithVFShaders("data/shaders/gl/win32/shader.vsh", "data/shaders/gl/win32/shader.fsh");
		//m_program->bindUniform<TestAppUniforms>(UF::SPACE_DATA, "spaceData");
		//m_program->bindUniform<TestAppUniforms>(UF::TEXTURES_DATA, "texturesData");
		//m_program->bindUniform<TestAppUniforms>(UF::LIGHTS_DATA, "lightsData");

		// lights
		framework::LightSource source;
		source.type = framework::LightType::DirectLight;
		source.position = vector3(0, 15, 0);
		vector3 dir(1, -1, 1);
		dir.norm();
		source.orientation.set_from_axes(vector3(0, 0, 1), dir);
		m_lightManager.addLightSource(getDevice(), source);

		m_lightsBuffer.reset(new framework::UniformBuffer());
		m_lightsBuffer->initDefaultStructured<framework::LightRawData>(getDevice(), (size_t)MAX_LIGHTS_COUNT);

		int lightsCount = std::min((int)m_lightManager.getLightSourcesCount(), MAX_LIGHTS_COUNT);
		for (int i = 0; i < lightsCount; i++)
		{
			m_lightsBuffer->setElement(i, m_lightManager.getRawLightData(i));
		}
	}

	virtual void shutdown()
	{
	}

	virtual void onResize(int width, int height)
	{
		m_camera.updateResolution(width, height);
	}

	virtual void render(double elapsedTime)
	{
		m_camera.update(elapsedTime);

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

		useDefaultRenderTarget();

		if (m_program->use(getDevice()))
		{
			//m_program->setMatrix<TestAppUniforms>(UF::MODELVIEWPROJECTION_MATRIX, m_mvp);
			//m_program->setMatrix<TestAppUniforms>(UF::MODEL_MATRIX, model);
			//m_program->setVector<TestAppUniforms>(UF::VIEW_DIRECTION, m_camera.getOrientation().z_direction());
			m_program->setUniform<TestAppUniforms>(getDevice(), UF::LIGHTS_DATA, m_lightsBuffer);

			//m_texture->setToSampler(m_program->getUniform<TestAppUniforms>(UF::DIFFUSE_MAP));
			//m_normalTexture->setToSampler(m_program->getUniform<TestAppUniforms>(UF::NORMAL_MAP));
			//m_specularTexture->setToSampler(m_program->getUniform<TestAppUniforms>(UF::SPECULAR_MAP));

			//m_geometry->renderAllMeshes();
		}

		//m_geometry->renderBoundingBox(m_mvp);

		matrix44 vp = m_camera.getView() * m_camera.getProjection();
		renderAxes(getDevice(), vp);
		m_lightManager.renderDebugVisualization(getDevice(), vp);
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
	//std::shared_ptr<framework::Geometry3D> m_geometry;
	//std::shared_ptr<framework::Texture> m_texture;
	//std::shared_ptr<framework::Texture> m_normalTexture;
	//std::shared_ptr<framework::Texture> m_specularTexture;
	std::shared_ptr<framework::UniformBuffer> m_lightsBuffer;

	framework::FreeCamera m_camera;
	matrix44 m_mvp;

	float m_rotation;
};

DECLARE_MAIN(TestApp);