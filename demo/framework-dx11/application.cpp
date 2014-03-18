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

#pragma warning(disable:4005) // CEGUI uses some obsolete stuff
#include "CEGUI/RendererModules/Direct3D11/Renderer.h"

namespace framework
{

static const std::string GUI_SKIN = "TaharezLook";

Application::AppInfo::AppInfo() : 
	title("Demo"), 
	windowWidth(1024), 
	windowHeight(768),
	samples(0),
	featureLevel(D3D_FEATURE_LEVEL_11_0)
{
	flags.all = 0;
	flags.fullscreen = 0;
	flags.vsync = 0;
	flags.cursor = 1;
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
	m_guiRenderer(0), 
	m_rootWindow(0), 
	m_fpsLabel(0), 
	m_fpsStorage(0), 
	m_timeSinceLastFpsUpdate(0),
	m_averageFps(0), 
	m_framesCounter(0),
	m_device(0),
	m_debugger(0),
	m_driverType(D3D_DRIVER_TYPE_HARDWARE)
	//m_axisX(0), 
	//m_axisY(0), 
	//m_axisZ(0) 
{
}

Application* Application::Instance()
{
	return m_self;
}

int Application::run(Application* self)
{
	m_self = self;

	if (!utils::Utils::exists("data/gui"))
	{
		utils::Logger::toLog("Error: could not find gui directory. Probably working directory has not been set correctly (especially if you are running from IDE).\n");
		return EXIT_FAILURE;
	}

	init();
	m_isRunning = true;

	// create a window
	if (!m_window.init(m_info.windowWidth, m_info.windowHeight, m_info.title))
	{
		utils::Logger::toLog("Error: could not create window.\n");
		return EXIT_FAILURE;
	}

	HRESULT hr = S_OK;
	AuroreleasePool<IUnknown> autorelease;

	// init D3D device
	if (!initDevice(autorelease))
	{
		return EXIT_FAILURE;
	}

	/*
	// init GL3w
	gl3wInit();

	std::vector<int> multisamplingLevels;
	if (!checkOpenGLVersion() || !checkDeviceCapabilities(multisamplingLevels))
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
		return EXIT_FAILURE;
	}

	#ifdef _DEBUG
	utils::Logger::toLogWithFormat("Video adapter: %s - %s, OpenGL: %s\n", (const char*)glGetString(GL_VENDOR), (const char*)glGetString(GL_RENDERER), (const char*)glGetString(GL_VERSION));
	#endif

	initGui();

	if (!StandardGpuPrograms::init())
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
		return EXIT_FAILURE;
	}
	initAxes();
	startup(m_rootWindow);*/

	do
    {
		m_window.pollEvents();

		if (m_window.shouldClose())
		{
			m_isRunning = false;
		}
		//m_isRunning &= (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);

		/*Texture::beginFrame();
		if (fabs(m_lastTime) < 1e-7)
		{
			render(0);
			Texture::endFrame();
			renderGui(0);

			m_lastTime = glfwGetTime();
		}
		else
		{
			double curTime = glfwGetTime();
			double delta = curTime - m_lastTime;
				
			// fps counter
			measureFps(delta);

			// rendering
			render(delta);
			Texture::endFrame();
			renderGui(delta);

			m_lastTime = curTime;
		}*/
    } 
	while(m_isRunning);

	/*shutdown();
	destroyAllDestroyable();
	destroyGui();

	glfwDestroyWindow(m_window);
	glfwTerminate();*/

	autorelease.perform();
	m_window.destroy();
	
	return EXIT_SUCCESS;
}

bool Application::initDevice(AuroreleasePool<IUnknown>& autorelease)
{
	HRESULT hr = S_OK;

	// dxgi factory
	IDXGIFactory* factory = 0;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory));
	if (hr != S_OK)
	{
		utils::Logger::toLog("Error: could not create DXGI factory.\n");
		return false;
	}
	autorelease.add(factory);

	// adapter
	IDXGIAdapter* adapter = 0;
	if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
	{
		utils::Logger::toLog("Error: could not find a graphics adapter.\n");
		return false;
	}
	autorelease.add(adapter);

	// adapter description
	DXGI_ADAPTER_DESC desc;
	if (adapter->GetDesc(&desc) != S_OK)
	{
		utils::Logger::toLog("Error: could not find a graphics adapter.\n");
		return false;
	}
	utils::Logger::toLog(L"Graphics adapter: " + std::wstring(desc.Description) + L".\n");
}

void Application::measureFps(double delta)
{
	m_timeSinceLastFpsUpdate += delta;
	if (m_timeSinceLastFpsUpdate >= 1.0)
	{
		m_averageFps = m_fpsStorage / (m_framesCounter > 0 ? (double)m_framesCounter : 1.0);
		if (m_fpsLabel != 0)
		{
			static char buf[100];
			sprintf(buf, "%.2f fps", (float)m_averageFps);
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

std::string Application::getGuiFullName(const std::string& name)
{
	return GUI_SKIN + name;
}

void Application::renderGui(double elapsedTime)
{
	//GLboolean scissorTestEnable = glIsEnabled(GL_SCISSOR_TEST);
	//GLboolean depthTestEnable = glIsEnabled(GL_DEPTH_TEST);
	//GLboolean blendingEnable = glIsEnabled(GL_BLEND);
	//GLboolean cullfaceEnable = glIsEnabled(GL_CULL_FACE);

	/*CEGUI::System& gui_system(CEGUI::System::getSingleton());
	gui_system.injectTimePulse((float)elapsedTime);
	m_guiRenderer->beginRendering();
	gui_system.getDefaultGUIContext().draw();
	m_guiRenderer->endRendering();
	CEGUI::WindowManager::getSingleton().cleanDeadPool();*/

	//if (scissorTestEnable) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
	//if (depthTestEnable) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	//if (blendingEnable) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	//if (cullfaceEnable) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
}

void Application::renderAxes(const matrix44& viewProjection)
{
	//m_axisX->renderWithStandardGpuProgram(viewProjection, vector4(1, 0, 0, 1), false);
	//m_axisY->renderWithStandardGpuProgram(viewProjection, vector4(0, 1, 0, 1), false);
	//m_axisZ->renderWithStandardGpuProgram(viewProjection, vector4(0, 0, 1, 1), false);
}

/*void Application::registerDestroyable(std::weak_ptr<Destroyable> ptr)
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
}*/

/*void Application::destroyAllDestroyable()
{
	for (auto it = m_destroyableList.begin(); it != m_destroyableList.end(); ++it)
	{
		if (!(*it).expired()) it->lock()->destroy();
	}
	m_destroyableList.clear();
}*/

/*bool Application::checkOpenGLVersion()
{
	int majorVersion = 0;
	int minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	if (majorVersion == 0 && minorVersion == 0)
	{
		utils::Logger::toLog("Error: Probably OpenGL 3+ is not supported in your hardware/drivers.\n");
		return false;
	}
	if (majorVersion < m_info.majorVersion || minorVersion < m_info.minorVersion)
	{
		utils::Logger::toLogWithFormat("Error: Your hardware supports OpenGL %d.%d. You are trying to use OpenGL %d.%d.\n", majorVersion, minorVersion, m_info.majorVersion, m_info.minorVersion);
		return false;
	}
	return true;
}*/

/*bool Application::checkDeviceCapabilities(std::vector<int>& multisamplingLevels)
{
	GLint samplesCount = 0;
	glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, 1, &samplesCount);
	GLint* samples = new GLint[samplesCount];
	glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, samplesCount, samples);
	multisamplingLevels.reserve(samplesCount);
	for (int i = 0; i < samplesCount; i++) 
	{
		if (samples[i] > 0) multisamplingLevels.push_back(samples[i]);
	}
	multisamplingLevels.push_back(0);
	delete [] samples;
	std::sort(multisamplingLevels.begin(), multisamplingLevels.end());

	if (std::find(multisamplingLevels.begin(), multisamplingLevels.end(), m_info.samples) == multisamplingLevels.end())
	{
		utils::Logger::toLogWithFormat("Error: Multisampling level (%d) is unsupported.\n", m_info.samples);
		return false;
	}

	return true;
}*/

void Application::initGui()
{
	/*m_guiRenderer = &CEGUI::Direct3D11Renderer::create();
	static_cast<CEGUI::Direct3D11Renderer*>(m_guiRenderer)->enableExtraStateSettings(true);
	CEGUI::System::create(*m_guiRenderer);
	initialiseResources();

	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	CEGUI::GUIContext* guiContext = &gui_system.getDefaultGUIContext();
	CEGUI::SchemeManager::getSingleton().createFromFile(getGuiFullName(".scheme").c_str());
	guiContext->getMouseCursor().setDefaultImage(getGuiFullName("/MouseArrow").c_str());
	guiContext->getMouseCursor().hide();

	CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
	m_rootWindow = (CEGUI::DefaultWindow*)winMgr.createWindow("DefaultWindow", "Root");
	CEGUI::Font& defaultFont = CEGUI::FontManager::getSingleton().createFromFile("DejaVuSans-12.font");
	guiContext->setDefaultFont(&defaultFont);
	guiContext->setRootWindow(m_rootWindow);

	m_fpsLabel = (CEGUI::Window*)winMgr.createWindow(getGuiFullName("/Label").c_str());
	m_rootWindow->addChild(m_fpsLabel);

	m_fpsLabel->setPosition(CEGUI::UVector2(CEGUI::UDim(1.0f, -150.0f), cegui_reldim(0.0)));
	m_fpsLabel->setSize(CEGUI::USize(cegui_absdim(150.0f), cegui_absdim(25.0f)));
	m_fpsLabel->setProperty("HorzFormatting", "RightAligned");
	m_fpsLabel->setText("0 fps");*/
}

void Application::destroyGui()
{
	//CEGUI::System::destroy();
	//CEGUI::Direct3D11Renderer::destroy(static_cast<CEGUI::Direct3D11Renderer&>(*m_guiRenderer));
}

/*void Application::initAxes()
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
}*/

/*void Application::errorCallback(int error, const char* description)
{
	utils::Logger::toLogWithFormat("Error: %s\n", description);
}*/

/*void APIENTRY Application::debugCallback(GLenum source,
                                    GLenum type,
                                    GLuint id,
                                    GLenum severity,
                                    GLsizei length,
                                    const GLchar* message,
                                    GLvoid* userParam)
{
	utils::Logger::toLogWithFormat("Debug: %s\n", message);
}*/

/*void Application::_setWindowSize(GLFWwindow* window, int width, int height)
{
	glfwMakeContextCurrent(window);
	m_self->m_guiRenderer->setDisplaySize(CEGUI::Sizef((float)width, (float)height));
	m_self->m_info.windowWidth = width;
	m_self->m_info.windowHeight = height;

	m_self->onResize(width, height);
}*/

/*void Application::_onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	CEGUI::Key::Scan ceguiKey = glfwToCeguiKey(key);

	if(action == GLFW_PRESS)
	{
		gui_system.getDefaultGUIContext().injectKeyDown(ceguiKey);
	}
	else if (action == GLFW_RELEASE)
	{
		gui_system.getDefaultGUIContext().injectKeyUp(ceguiKey);
	}

	m_self->onKeyButton(key, scancode, action, mods);
}*/

/*void Application::_onChar(GLFWwindow* window, unsigned int codepoint)
{
	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	gui_system.getDefaultGUIContext().injectChar(codepoint);
}*/

/*void Application::_onMouse(GLFWwindow* window, int button, int action, int mods)
{
	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	CEGUI::MouseButton ceguiMouseButton = glfwToCeguiMouseButton(button);

	if(action == GLFW_PRESS)
	{
		gui_system.getDefaultGUIContext().injectMouseButtonDown(ceguiMouseButton);
	}
	else if (action == GLFW_RELEASE)
	{
		gui_system.getDefaultGUIContext().injectMouseButtonUp(ceguiMouseButton);
	}

	double xpos = 0, ypos = 0;
	glfwGetCursorPos(window, &xpos, &ypos);
	m_self->onMouseButton(xpos, ypos, button, action, mods);
}*/

/*void Application::_onCursor(GLFWwindow* window, double xpos, double ypos)
{
	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	gui_system.getDefaultGUIContext().injectMousePosition((float)xpos, (float)ypos);

	m_self->onMouseMove(xpos, ypos);
}*/

/*void Application::_onScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	gui_system.getDefaultGUIContext().injectMouseWheelChange((float)yoffset);
}*/

/*void Application::initialiseResources()
{
	CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());

	rp->setResourceGroupDirectory("schemes", "data/gui/schemes");
	rp->setResourceGroupDirectory("imagesets", "data/gui/imagesets");
	rp->setResourceGroupDirectory("fonts", "data/gui/fonts");
	rp->setResourceGroupDirectory("layouts", "data/gui/layouts");
	rp->setResourceGroupDirectory("looknfeels", "data/gui/looknfeel");
	rp->setResourceGroupDirectory("lua_scripts", "data/gui/lua_scripts");
	rp->setResourceGroupDirectory("schemas", "data/gui/xml_schemas");   
	rp->setResourceGroupDirectory("animations", "data/gui/animations");

	CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
	CEGUI::Font::setDefaultResourceGroup("fonts");
	CEGUI::Scheme::setDefaultResourceGroup("schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
	CEGUI::WindowManager::setDefaultResourceGroup("layouts");
	CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
	CEGUI::AnimationManager::setDefaultResourceGroup("animations");

	CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
	{
		parser->setProperty("SchemaDefaultResourceGroup", "schemas");
	}
}*/

/*CEGUI::Key::Scan Application::glfwToCeguiKey(int glfwKey)
{
	switch(glfwKey)
	{
		case GLFW_KEY_ESCAPE : return CEGUI::Key::Escape;
		case GLFW_KEY_F1 : return CEGUI::Key::F1;
		case GLFW_KEY_F2 : return CEGUI::Key::F2;
		case GLFW_KEY_F3 : return CEGUI::Key::F3;
		case GLFW_KEY_F4 : return CEGUI::Key::F4;
		case GLFW_KEY_F5 : return CEGUI::Key::F5;
		case GLFW_KEY_F6 : return CEGUI::Key::F6;
		case GLFW_KEY_F7 : return CEGUI::Key::F7;
		case GLFW_KEY_F8 : return CEGUI::Key::F8;
		case GLFW_KEY_F9 : return CEGUI::Key::F9;
		case GLFW_KEY_F10 : return CEGUI::Key::F10;
		case GLFW_KEY_F11 : return CEGUI::Key::F11;
		case GLFW_KEY_F12 : return CEGUI::Key::F12;
		case GLFW_KEY_F13 : return CEGUI::Key::F13;
		case GLFW_KEY_F14 : return CEGUI::Key::F14;
		case GLFW_KEY_F15 : return CEGUI::Key::F15;
		case GLFW_KEY_UP : return CEGUI::Key::ArrowUp;
		case GLFW_KEY_DOWN : return CEGUI::Key::ArrowDown;
		case GLFW_KEY_LEFT : return CEGUI::Key::ArrowLeft;
		case GLFW_KEY_RIGHT : return CEGUI::Key::ArrowRight;
		case GLFW_KEY_LEFT_SHIFT : return CEGUI::Key::LeftShift;
		case GLFW_KEY_RIGHT_SHIFT : return CEGUI::Key::RightShift;
		case GLFW_KEY_LEFT_CONTROL : return CEGUI::Key::LeftControl;
		case GLFW_KEY_RIGHT_CONTROL : return CEGUI::Key::RightControl;
		case GLFW_KEY_LEFT_ALT : return CEGUI::Key::LeftAlt;
		case GLFW_KEY_RIGHT_ALT : return CEGUI::Key::RightAlt;
		case GLFW_KEY_TAB : return CEGUI::Key::Tab;
		case GLFW_KEY_ENTER : return CEGUI::Key::Return;
		case GLFW_KEY_BACKSPACE : return CEGUI::Key::Backspace;
		case GLFW_KEY_INSERT : return CEGUI::Key::Insert;
		case GLFW_KEY_DELETE : return CEGUI::Key::Delete;
		case GLFW_KEY_PAGE_UP : return CEGUI::Key::PageUp;
		case GLFW_KEY_PAGE_DOWN : return CEGUI::Key::PageDown;
		case GLFW_KEY_HOME : return CEGUI::Key::Home;
		case GLFW_KEY_END : return CEGUI::Key::End;
		case GLFW_KEY_KP_ENTER : return CEGUI::Key::NumpadEnter;
		default : return CEGUI::Key::Unknown;
	}
}*/

/*CEGUI::MouseButton Application::glfwToCeguiMouseButton(int glfwButton)
{
	switch(glfwButton)
	{
		case GLFW_MOUSE_BUTTON_LEFT     : return CEGUI::LeftButton;
		case GLFW_MOUSE_BUTTON_RIGHT    : return CEGUI::RightButton;
		case GLFW_MOUSE_BUTTON_MIDDLE   : return CEGUI::MiddleButton;
		default                         : return CEGUI::NoButton;
	}
}*/

}