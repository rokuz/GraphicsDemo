/*
 * Copyright (c) 2014 Roman Kuznetsov 
 *
 * Thanks to Graham Sellers (and his book "OpenGL SuperBible, 6th Edition") for inspiration and some code
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

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#ifdef WIN32
    #pragma once

    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>
#endif

#include <d3d11.h>
#include <string>
#include <list>
#include "CEGUI/CEGUI.h"

#include "window.h"
#include "autoreleasepool.h"

#include "matrix.h"

//#include "destroyable.h"
#include "logger.h"
#include "utils.h"
//#include "geometry3D.h"
//#include "line3D.h"
//#include "texture.h"
//#include "gpuprogram.h"
//#include "standardGpuPrograms.h"
//#include "freeCamera.h"
//#include "lightManager.h"
//#include "uniformBuffer.h"

namespace framework
{

class Application
{
	//friend class Destroyable;

public:
	Application();
	virtual ~Application(){}

	virtual void init(){}
	virtual void startup(CEGUI::DefaultWindow* root){}
	virtual void render(double elapsedTime){}
	virtual void shutdown(){}
	virtual void onResize(int width, int height){}
	virtual void onKeyButton(int key, int scancode, int action, int mods){}
	virtual void onMouseButton(double xpos, double ypos, int button, int action, int mods){}
	virtual void onMouseMove(double xpos, double ypos){}

	static Application* Instance();

	int run(Application* self);

protected:
	struct AppInfo
    {
        std::string title;
        int windowWidth;
        int windowHeight;
        int samples;
		D3D_FEATURE_LEVEL featureLevel;
        union
        {
            struct
            {
                unsigned int    fullscreen  : 1;
                unsigned int    vsync       : 1;
                unsigned int    cursor      : 1;
                unsigned int    debug       : 1;
            };
            unsigned int        all;
        } flags;
		
		AppInfo();
    };

	AppInfo m_info;
	//LightManager m_lightManager;

	static std::string getGuiFullName(const std::string& name);
	void renderGui(double elapsedTime);
	void renderAxes(const matrix44& viewProjection);

private:
	static Application* m_self;
	Window m_window;
	bool m_isRunning;
	double m_lastTime;
	double m_fpsStorage;
	CEGUI::Renderer* m_guiRenderer;
	CEGUI::DefaultWindow* m_rootWindow;
	CEGUI::Window* m_fpsLabel;
	double m_timeSinceLastFpsUpdate;
	double m_averageFps;
	size_t m_framesCounter;

	ID3D11Device* m_device;
	ID3D11Debug* m_debugger;
	D3D_DRIVER_TYPE m_driverType;

	//std::list<std::weak_ptr<Destroyable> > m_destroyableList;
	//std::shared_ptr<Line3D> m_axisX;
	//std::shared_ptr<Line3D> m_axisY;
	//std::shared_ptr<Line3D> m_axisZ;

	//void registerDestroyable(std::weak_ptr<Destroyable> ptr);
	//void destroyAllDestroyable();

	//bool checkOpenGLVersion();
	//bool checkDeviceCapabilities(std::vector<int>& multisamplingLevels);

	bool initDevice(AuroreleasePool<IUnknown>& autorelease);

	void initGui();
	void destroyGui();
	void initialiseResources();

	//void initAxes();

	void measureFps(double delta);

	//static void errorCallback(int error, const char* description);
	//static void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
	//static void _setWindowSize(GLFWwindow* window, int width, int height);
	//static void _onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
	//static void _onChar(GLFWwindow* window, unsigned int codepoint);
	//static void _onMouse(GLFWwindow* window, int button, int action, int mods);
	//static void _onCursor(GLFWwindow* window, double xpos, double ypos);
	//static void _onScroll(GLFWwindow* window, double xoffset, double yoffset);
	//static CEGUI::Key::Scan glfwToCeguiKey(int glfwKey);
	//static CEGUI::MouseButton glfwToCeguiMouseButton(int glfwButton);
};

}
#if defined _WIN32
#define DECLARE_MAIN(A)                             \
int CALLBACK WinMain(HINSTANCE hInstance,           \
                     HINSTANCE hPrevInstance,       \
                     LPSTR lpCmdLine,               \
                     int nCmdShow)                  \
{                                                   \
    A *app = new A();                               \
    int result = app->run(app);                     \
    delete app;                                     \
    return result;                                  \
}
#elif defined __APPLE__
#define DECLARE_MAIN(a)                             \
int main(int argc, const char ** argv)              \
{                                                   \
    A *app = new A();                               \
    int result = app->run(app);                     \
    delete app;                                     \
    return result;                                  \
}
#else
#error Undefined platform!
#endif

#endif // __APPLICATION_H__