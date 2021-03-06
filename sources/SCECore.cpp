/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCECore.cpp *********/
/**************************************/

#include "../headers/SCECore.hpp"
#include "../headers/SCE_GLDebug.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCERender.hpp"
#include "../headers/SCEDebug.hpp"
#include "../headers/SCEInput.hpp"

#include <time.h>
#include <glfw3.h>

using namespace SCE;
using namespace std;

GLFWwindow *    SCECore::s_window       = nullptr;
int             SCECore::s_windowWidth  = 0;
int             SCECore::s_windowHeight = 0;

SCECore::~SCECore()
{
    CleanUpEngine();
}

void SCECore::InitEngine(const std::string &windowName)
{
    Internal::Log("Initializing engine");

    // Initialise GLFW
    if( !glfwInit() )
    {
       Debug::RaiseError("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_SAMPLES, 0); // disable antialiasing, which doesn't work  on my computeranyway
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 4.0+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    //set framebuffer to suppoer 32 depth bits and 8 stencil bits
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    glfwWindowHint(GLFW_DECORATED, 0);//0 for full screen
    glfwWindowHint(GLFW_REFRESH_RATE, 60);

#ifdef SCE_DEBUG
    // Create a debug OpenGL context or tell your OpenGL library (GLFW, SDL) to do so.
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    // Open a window and create its OpenGL context
    s_window = glfwCreateWindow( 1920, 1080, windowName.c_str(), glfwGetPrimaryMonitor(), NULL);

//    s_window = glfwCreateWindow( 1920, 1080, windowName.c_str(), NULL, NULL);

    Internal::Log("Window created");

    if( s_window == NULL ){
        glfwTerminate();
        Debug::RaiseError("Failed to open GLFW window.");
    }
    glfwMakeContextCurrent(s_window);
    glewExperimental=true; // Needed in core profile

#ifdef SCE_DEBUG
        //no vsync
       glfwSwapInterval(0);
#else
    //enable v-sync
//    glfwSwapInterval(1);
    //adaptative v-sync
//    glfwSwapInterval(-1);
    glfwSwapInterval(0);
#endif


    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        Debug::RaiseError("Failed to initialize GLEW.");
    }

    glfwSetCursorPos(s_window, 0.0, 0.0);

    UpdateWindow();

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(s_window, GLFW_STICKY_KEYS, GL_TRUE);
//    glfwSetInputMode(s_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

#ifdef SCE_DEBUG

    if(glDebugMessageCallbackAMD) {
        Internal::Log("Linking GL debug with AMD callback");
        glDebugMessageCallbackAMD(DebugCallbackAMD, NULL);
    }
    else if(glDebugMessageCallbackARB) {
        Internal::Log("Linking GL debug with ARB callback");
        glDebugMessageCallbackARB(DebugCallback, NULL);
    }
    else {
        Internal::Log("Linking GL debug with standard callback");
        glDebugMessageCallback(DebugCallback, NULL);
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

#endif
    //read and empty the error queue
    GLenum errorCode;
    while((errorCode = glGetError()) != GL_NO_ERROR)
    {
        Debug::Log("Warning : OpenGL error found : " + std::to_string(errorCode));
    }

#ifdef SCE_DEBUG
    SCE::Math::SeedRandomGenerator(0);
#else
    time_t ctime = time(0);
    tm* calendarTime = localtime(&ctime);
    SCE::Math::SeedRandomGenerator(calendarTime->tm_sec);
#endif

    //Init Engine subcomponents in order
    SCE::Time::Init();
    //Rendering
    SCE::Render::Init();
}

void SCECore::RunEngine()
{
    int escPressCount = 0;

    do
    {        
        SCE::Time::Update();
        SCE::Input::UpdateKeyStates(s_window);
        SCE::Debug::UpdateDebugMenu();
        SCEScene::Run();

        // Swap buffers
        glfwSwapBuffers(s_window);
        glfwPollEvents();

        if(SCE::Input::GetKeyAction( GLFW_KEY_ESCAPE ) == SCE::Input::KeyAction::Press)
        {
            ++escPressCount;
        }        

    } while( escPressCount < 2);

}

void SCECore::CleanUpEngine()
{
    Internal::Log("Cleaning up engine");
    SCEScene::DestroyScene();

    //clean engine subcomponents
    SCE::Render::CleanUp();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

GLFWwindow *SCECore::GetWindow()
{
    return s_window;
}

int SCECore::GetWindowWidth()
{
    return s_windowWidth;
}

int SCECore::GetWindowHeight()
{
    return s_windowHeight;
}

void SCECore::UpdateWindow()
{
    int width, height;
    glfwGetWindowSize(s_window, &width, &height);
    s_windowWidth = width;
    s_windowHeight = height;
}


