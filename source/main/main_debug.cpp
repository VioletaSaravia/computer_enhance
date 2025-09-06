#include "io.hpp"
#include "os.cpp"
#include "runtime.hpp"
#include <vector>

#ifdef _WIN32
#define DLL_EXT ".dll"
#elif defined(__linux__)
#define DLL_EXT ".so"
#else
#error "Unsupported platform"
#endif

#define DLL_DIR       "build/debug/"
#define DLL_GAME_PATH "game" DLL_EXT

struct GameApi {
    u64   modTime, version;
    void* library;

    void (*Init)();
    void (*Update)();
    void (*Shutdown)();
    bool (*ShouldClose)();
    void* (*GetMemory)();
    u64 (*MemorySize)();
    void (*HotReloaded)(void*);
    bool (*ForceReload)();
    bool (*ForceRestart)();

    GameApi(u64 _version) : version{_version} {}

    ~GameApi() {}
};

i32 main(i32 argc, cstr argv[]) {
    u64     apiVersion = 0;
    GameApi api(apiVersion);
    if (!api.library) return 1;

    apiVersion += 1;

    api.Init();

    std::vector<GameApi> oldApis{};

    while (!api.ShouldClose) {
        api.Update();
        bool forceReload  = api.ForceReload();
        bool forceRestart = api.ForceRestart();
        bool reload       = forceReload || forceRestart;

        u64 dllModTime = 0; // get last write time of GAME_DLL_PATH

        if (dllModTime != 0 && api.modTime != dllModTime) {
            reload = true;
        }

        if (reload) {
            GameApi newApi(apiVersion);
            if (!newApi.library) continue;

            forceRestart |= api.MemorySize() != newApi.MemorySize();

            if (!forceRestart) {
                oldApis.push_back(api);
                void* memory = api.GetMemory();
                api          = newApi;
                api.HotReloaded(memory);
            } else {
                api.Shutdown();
                oldApis.clear();
                api = newApi;
                api.Init();
            }

            apiVersion += 1;
        }
    }

    api.Shutdown();
}