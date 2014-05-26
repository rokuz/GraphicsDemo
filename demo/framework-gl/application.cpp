/*
 * Copyright (c) 2014 Roman Kuznetsov 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "application.h"
#include "uirenderer.h"

namespace framework
{

Application::AppInfo::AppInfo() : 
	title("Demo"), 
	windowWidth(1024), 
	windowHeight(768), 
	samples(0)
{
	flags.all = 0;
	flags.fullscreen = 0;
	flags.vsync = 0;
	flags.cursor = 1;
	flags.useStencil = 0;
	#ifdef _DEBUG
	flags.debug = 1;
	#else
	flags.debug = 0;
	#endif
}

Application* Application::m_self = 0;

Application::Application() : 
	m_isRunning(false), 
	m_lastTime(0)
{
}

Application* Application::instance()
{
	return m_self;
}

int Application::run(Application* self, const std::string& commandLine)
{
	m_self = self;
	if (!m_timer.init())
	{
		utils::Logger::toLog("Error: could not initialize a timer.\n");
		return EXIT_FAILURE;
	}

	if (!utils::Utils::exists("data"))
	{
		utils::Logger::toLog("Error: could not find data directory. Probably working directory has not been set correctly (especially if you are running from IDE).\n");
		return EXIT_FAILURE;
	}

	// initialize app
	m_isRunning = true;
	auto params = utils::Utils::parseCommandLine(commandLine);
	init(params);
	if (!m_isRunning) { return EXIT_SUCCESS; }
	
	// create context
	if (!m_context.init((size_t)m_info.windowWidth, (size_t)m_info.windowHeight, m_info.title, 
						m_info.flags.fullscreen != 0, m_info.samples, 
						m_info.flags.vsync != 0, m_info.flags.useStencil != 0))
	{
		utils::Logger::toLog("Error: could not initialize OpenGL context");
		return EXIT_FAILURE;
	}
	m_context.getWindow().setCursorVisibility(m_info.flags.cursor != 0);

	utils::Logger::toLogWithFormat("Video adapter: %s - %s, OpenGL: %s\n", (const char*)glGetString(GL_VENDOR), (const char*)glGetString(GL_RENDERER), (const char*)glGetString(GL_VERSION));

    if (m_info.flags.debug)
    {
        if (gl3wIsSupported(4, 3))
        {
            glDebugMessageCallback(debugCallback, this);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }
        else if (m_context.isExtensionSupported("GL_ARB_debug_output"))
        {
            glDebugMessageCallbackARB(debugCallback, this);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        }
    }

	Texture::init();
	initInput();
	if (!initGui() || !StandardGpuPrograms::init())
	{
		m_context.destroy();
		return EXIT_FAILURE;
	}
	initAxes();
	startup(gui::UIManager::instance().root());

	mainLoop();

	shutdown();
	destroyAllDestroyable();
	destroyGui();
	Texture::cleanup();
	m_context.destroy();

	return EXIT_SUCCESS;
}

void Application::mainLoop()
{
	while (m_isRunning)
	{
		TRACE_BLOCK("_Frame");
		m_fpsCounter.beginFrame();

		m_context.makeCurrent();

		// process events from the window
		m_context.getWindow().pollEvents();

		// need to close?
		if (m_context.getWindow().shouldClose())
		{
			m_isRunning = false;
		}

		// rendering
		double curTime = m_timer.getTime();
		if (m_lastTime == 0) m_lastTime = curTime;
		double delta = curTime - m_lastTime;
		
		useDefaultRenderTarget();
		clearDefaultRenderTarget();
		render(delta);
		renderGui(delta);
		
		m_lastTime = curTime;

		m_context.present();

		if (m_fpsCounter.endFrame())
		{
			if (m_fpsLabel != 0)
			{
				static wchar_t buf[100];
				swprintf(buf, L"%.2f fps", (float)m_fpsCounter.getFps());
				m_fpsLabel->setText(buf);
			}
		}
	}
}

void Application::exit()
{
	m_isRunning = false;
}

bool Application::isDebugEnabled() const
{
	return m_info.flags.debug != 0;
}

void Application::resize()
{
	gui::UIManager::instance().setScreenSize((size_t)m_info.windowWidth, (size_t)m_info.windowHeight);
	onResize(m_info.windowWidth, m_info.windowHeight);
}

void Application::renderGui(double elapsedTime)
{
	TRACE_FUNCTION;
	gui::UIManager::instance().injectFrameTime(elapsedTime);

	if (gui::UIManager::instance().renderer())
	{
		useDefaultRenderTarget();
		gui::UIManager::instance().renderer()->render();
	}
}

void Application::renderAxes(const matrix44& viewProjection)
{
	m_axisX->renderWithStandardGpuProgram(viewProjection, vector4(1, 0, 0, 1), false);
	m_axisY->renderWithStandardGpuProgram(viewProjection, vector4(0, 1, 0, 1), false);
	m_axisZ->renderWithStandardGpuProgram(viewProjection, vector4(0, 0, 1, 1), false);
}

void Application::renderSkybox(Camera& camera, std::shared_ptr<Texture> texture)
{
	if (!texture) return;

	auto prog = framework::StandardGpuPrograms::getSkyboxRenderer();
	if (prog->use())
	{
		framework::DepthState depthTestDisable(false);
		depthTestDisable.apply();

		matrix44 model;
		model.set_translation(camera.getPosition());
		matrix44 mvp = model * camera.getView() * camera.getProjection();

		prog->setTexture<StandardUniforms>(STD_UF::SKYBOX_MAP, std::move(texture));
		prog->setMatrix<StandardUniforms>(STD_UF::MODELVIEWPROJECTION_MATRIX, mvp);

		glDrawArrays(GL_POINTS, 0, 1);

		depthTestDisable.cancel();
	}
}

void Application::registerDestroyable(std::weak_ptr<Destroyable> ptr)
{
	if (!ptr.expired())
	{
		auto sptr = ptr.lock();
		auto fptr = std::find_if(m_destroyableList.begin(), m_destroyableList.end(), [=](std::weak_ptr<Destroyable> ptr2)
		{
			if (ptr2.expired()) return false;
			return sptr == ptr2.lock();
		});

		if (fptr == m_destroyableList.end())
		{
			m_destroyableList.push_back(ptr);
		}
	}
}

void Application::destroyAllDestroyable()
{
	for (auto it = m_destroyableList.begin(); it != m_destroyableList.end(); ++it)
	{
		if (!(*it).expired()) it->lock()->destroy();
	}
	m_destroyableList.clear();
}

bool Application::initGui()
{
	gui::UIResourcesFactoryPtr_T factory(new UIResourcesFactoryOGL());
	gui::UIRendererPtr_T renderer(new UIRendererOGL());
	if (!gui::UIManager::instance().init((size_t)m_info.windowWidth, (size_t)m_info.windowHeight, factory, renderer))
	{
		return false;
	}

	// create a label to show fps statistics
	m_fpsLabel = UIFactory::createLabel(gui::Coords::Coords(1.0f, -150.0f, 0.0f, 0.0f),
										gui::Coords::Absolute(150.0f, 25.0f),
										gui::RightAligned, gui::TopAligned, L"0 fps");
	gui::UIManager::instance().root()->addChild(m_fpsLabel);

	// create a label to show the legend
	m_legendLabel = framework::UIFactory::createLabel(gui::Coords::Coords(0.0f, 0.0f, 0.0f, 0.0f),
													  gui::Coords::Absolute(500.0f, 500.0f),
													  gui::LeftAligned, gui::TopAligned,
													  utils::Utils::toUnicode(m_legend));
	m_legendLabel->setVisible(!m_legend.empty());
	gui::UIManager::instance().root()->addChild(m_legendLabel);

	return true;
}

void Application::destroyGui()
{
	gui::UIManager::instance().cleanup();
}

void Application::initInput()
{
	m_context.getWindow().setKeyboardHandler([this](int key, int scancode, bool pressed)
	{
		InputKeys::Scan iKey = (InputKeys::Scan)(key);
		if (pressed)
		{
			gui::UIManager::instance().injectKeyDown(iKey);
		}
		else
		{
			gui::UIManager::instance().injectKeyUp(iKey);
		}

		// hint to exit on ESC
		if (iKey == InputKeys::Escape) m_isRunning = false;

		onKeyButton(key, scancode, pressed);
	});

	m_context.getWindow().setCharHandler([this](int codepoint)
	{
		gui::UIManager::instance().injectChar(codepoint);
	});

	m_context.getWindow().setMouseHandler([this](double xpos, double ypos, double zdelta, int button, bool pressed)
	{
		if (button >= 0)
		{
			if (pressed)
			{
				gui::UIManager::instance().injectMouseButtonDown((InputKeys::MouseButton)button);
			}
			else
			{
				gui::UIManager::instance().injectMouseButtonUp((InputKeys::MouseButton)button);
			}

			onMouseButton(xpos, ypos, button, pressed);
		}
		else
		{
			gui::UIManager::instance().injectMousePosition((float)xpos, (float)ypos);
			if (fabs(zdelta) > 1e-4)
			{
				gui::UIManager::instance().injectMouseWheelChange((float)zdelta);
			}

			onMouseMove(xpos, ypos);
		}
	});
}

void Application::initAxes()
{	
	#define INIT_POINTS(name, point) std::vector<vector3> name; name.push_back(vector3()); name.push_back(point);
	INIT_POINTS(points_x, vector3(20, 0, 0));
	INIT_POINTS(points_y, vector3(0, 20, 0));
	INIT_POINTS(points_z, vector3(0, 0, 20));
	#undef INIT_POINTS

	m_axisX.reset(new Line3D());
	m_axisX->initWithArray(points_x);
		
	m_axisY.reset(new Line3D());
	m_axisY->initWithArray(points_y);

	m_axisZ.reset(new Line3D());
	m_axisZ->initWithArray(points_z);
}

void Application::applyStandardParams(const std::map<std::string, int>& params)
{
	auto w = params.find("w");
	auto h = params.find("h");
	if (w != params.end() && h != params.end())
	{
		m_info.windowWidth = w->second;
		m_info.windowHeight = h->second;
	}

	auto msaa = params.find("msaa");
	if (msaa != params.end())
	{
		m_info.samples = msaa->second;
	}
	else
	{
		m_info.samples = 0;
	}

	auto fullscreen = params.find("fullscreen");
	if (fullscreen != params.end())
	{
		m_info.flags.fullscreen = (fullscreen->second != 0 ? 1 : 0);
	}
	else
	{
		m_info.flags.fullscreen = 0;
	}
}

void Application::setLegend(const std::string& legend)
{
	m_legend = legend;
	if (m_legendLabel != 0)
	{
		m_legendLabel->setVisible(!m_legend.empty());
		m_legendLabel->setText(utils::Utils::toUnicode(m_legend.c_str()));
	}
}

vector2 Application::getScreenSize()
{
	return vector2((float)m_info.windowWidth, (float)m_info.windowHeight);
}

void Application::useDefaultRenderTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
}

void Application::clearDefaultRenderTarget(const vector4& color, float depth)
{
	const GLfloat c[] = { color.x, color.y, color.z, color.w };
	glClearBufferfv(GL_COLOR, 0, c);
	glClearBufferfv(GL_DEPTH, 0, &depth);
}

void APIENTRY Application::debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
										 GLsizei length, const GLchar* message, GLvoid* userParam)
{
	utils::Logger::toLogWithFormat("Debug: %s\n", message);
}

}