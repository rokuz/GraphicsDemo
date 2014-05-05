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

#include "application.h"
#include <algorithm>

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
	m_lastTime(0), 
	m_fpsStorage(0), 
	m_timeSinceLastFpsUpdate(0),
	m_averageFps(0), 
	m_framesCounter(0),
	m_axisX(0), 
	m_axisY(0), 
	m_axisZ(0) 
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

	initInput();
	initGui();
	if (!StandardGpuPrograms::init())
	{
		m_context.destroy();
		return EXIT_FAILURE;
	}
	initAxes();
	startup(m_rootWindow);

	mainLoop();

	shutdown();
	destroyAllDestroyable();
	destroyGui();
	m_context.destroy();

	return EXIT_SUCCESS;
}

void Application::mainLoop()
{
	while (m_isRunning)
	{
		// process events from the window
		m_context.getWindow().pollEvents();

		// need to close?
		if (m_context.getWindow().shouldClose())
		{
			m_isRunning = false;
		}

		Texture::beginFrame();
		if (fabs(m_lastTime) < 1e-7)
		{
			render(0);
			Texture::endFrame();
			renderGui(0);

			m_lastTime = m_timer.getTime();
		}
		else
		{
			double curTime = m_timer.getTime();
			double delta = curTime - m_lastTime;

			// fps counter
			measureFps(delta);

			// rendering
			render(delta);
			Texture::endFrame();
			renderGui(delta);

			m_lastTime = curTime;
		}

		m_context.present();
	}
}

void Application::measureFps(double delta)
{
	m_timeSinceLastFpsUpdate += delta;
	if (m_timeSinceLastFpsUpdate >= 1.0)
	{
		m_averageFps = m_fpsStorage / (m_framesCounter > 0 ? (double)m_framesCounter : 1.0);
		if (m_fpsLabel != 0)
		{
			static wchar_t buf[100];
			swprintf(buf, L"%.2f fps", (float)m_averageFps);
			m_fpsLabel->setText(buf);
		}

		m_timeSinceLastFpsUpdate -= 1.0;
		m_framesCounter = 0;
		m_fpsStorage = 0;
	}
	else
	{
		m_framesCounter++;
		m_fpsStorage += (1.0 / delta);
	}
}

void Application::exit()
{
	m_isRunning = false;
}

void Application::resize()
{
	gui::UIManager::instance().setScreenSize((size_t)m_info.windowWidth, (size_t)m_info.windowHeight);
	onResize(m_info.windowWidth, m_info.windowHeight);
}

void Application::renderGui(double elapsedTime)
{
	//TODO

	gui::UIManager::instance().injectFrameTime(elapsedTime);
}

void Application::renderAxes(const matrix44& viewProjection)
{
	m_axisX->renderWithStandardGpuProgram(viewProjection, vector4(1, 0, 0, 1), false);
	m_axisY->renderWithStandardGpuProgram(viewProjection, vector4(0, 1, 0, 1), false);
	m_axisZ->renderWithStandardGpuProgram(viewProjection, vector4(0, 0, 1, 1), false);
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

void Application::initGui()
{
	gui::UIManager::instance().init((size_t)m_info.windowWidth, (size_t)m_info.windowHeight);
	m_rootWindow = gui::UIManager::instance().root();

	// create a label to show fps statistics
	m_fpsLabel = gui::UIManager::instance().createLabel(gui::Coords::Coords(1.0f, -150.0f, 0.0f, 0.0f),
														gui::Coords::Absolute(150.0f, 25.0f),
														gui::RightAligned, gui::TopAligned, L"0 fps");
	m_rootWindow->addChild(m_fpsLabel);
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

void APIENTRY Application::debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
										 GLsizei length, const GLchar* message, GLvoid* userParam)
{
	utils::Logger::toLogWithFormat("Debug: %s\n", message);
}

}