# 🏃‍♂️ GTFO-Profiler ⌛

A Google Trace _event_ Format Output Profiler <br>
A simple to use _instrumentation profiler_ to benchmark timings of chunks of your code base.

<img width="413" height="288" alt="t2" src="https://github.com/user-attachments/assets/2ae4def2-8ed4-49a5-b3af-503dac6b5b47" />

## Table of Contents 📑

- [How To Use](#how-to-use)
- [Examples](#examples)
- [Integration](#integration)
- [Dependencies](#dependencies)

## How To Use 🤖

> Macro definitions will not show their implementations for clarity

### Session Macros 💻

```C++
#define GTFO_PROFILE_SESSION_START(name, filePath)
#define GTFO_PROFILE_SESSION_END()
```

Session macros are used to describe **large/conceptual** chunks of code that all profiling <br>
data _within_ the start and end macro calls will be assigned to.

### Scope Macros 🔭

```C++
#define GTFO_PROFILE_SCOPE(scopeName, scopeCategory)
#define GTFO_PROFILE_FUNCTION()
```

Scope macros are used to describe **small** chunks of code such as function calls, class constructors/destructors, etc. <br> 
Scope macros will automatically stop timing when they are destroyed in their current scope. GTFO_PROFILE_FUNCTION() is syntactic <br>
sugar and simply calls GTFO_PROFILE_SCOPE(...) with the expansion of the \__FUNCTION__ variable and "function" as the category. 

### Turn Off Profiling ❌

To disable profiling, simply define GTFO_PROFILER_OFF. You can do this using CMake (and other build systems too) by doing the following.

```cmake
    add_compile_definitions(yourTarget GTFO_PROFILER_OFF)
```

or

```cmake
    target_compile_definitions(yourTarget PUBLIC GTFO_PROFILER_OFF)
```

You can also just #define GTFO_PROFILER_OFF somewhere in your code base.

## Examples 📬

```C++
// app.cpp
#include "window.h"
#include "app.h"

//...

void App::init() {
    GTFO_PROFILE_FUNCTION();

    {
        GTFO_PROFILE_SCOPE("Load Config", "init");
        mConfig = loadConfig();
    }

    {
        GTFO_PROFILE_SCOPE("Window Init", "init");
        mWindow = new Window();
        mWindow->setSize(1920, 1080);
        mWindow->setBorderless(false);
        mWindow->setVSync(true);
    }

    {
        GTFO_PROFILE_SCOPE("Renderer Init", "init");
        mRenderer = new Renderer();
        mRenderer->startBackend();
        mRenderer->giveWindowContext(mWindow);
    }
};


//...

// main.cpp
#include "app.h"
#include "gtfo_profiler.h"
int main() {
   GTFO_PROFILE_SESSION_START("App Init", "AppInitResults.json"); 
   App *app = new App();
   app->init();
   GTFO_PROFILE_SESSION_END(); 


   GTFO_PROFILE_SESSION_START("App Run", "AppRunResults.json"); 
   app->run(); 
   GTFO_PROFILE_SESSION_END(); 


   GTFO_PROFILE_SESSION_START("App Shutdown", "AppShutdownResults.json"); 
   delete app;
   GTFO_PROFILE_SESSION_END(); 
}
```

## Integration 🛠

To integrate GTFOProfiler into you project simply add **both** the _include_ and _src_ directories <br>
and then link it against your project using your build system (CMake, Premake, etc.)
> As of v1.0.0, gtfo_profiler.h is hardcoded into using nlohmann/json.hpp by doing '#include "../include/nlohmann/json.hpp"'.
> If your project _already_ contains nlohmann_json as a dependency, change this #include statement to point to your nlohmann/json.hpp

## Dependencies 🧰

- [nlohmann_json](https://github.com/nlohmann/json)
