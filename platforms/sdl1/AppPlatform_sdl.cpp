#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cerrno>
#include <ctime>

#include "thirdparty/GL/GL.hpp"

#include "stb_image.h"
#include "AppPlatform_sdl.hpp"
#include "common/Utils.hpp"
#include "CustomSoundSystem.hpp"
#include "client/player/input/Controller.hpp"

AppPlatform_sdl::AppPlatform_sdl(std::string storageDir, SDL_Surface* screen)
{
    _init(storageDir, screen);

    /* ICON */
    SDL_Surface* icon = loadSurface("icon.bmp");
    if (icon) {
        SDL_WM_SetIcon(icon, nullptr);
        SDL_FreeSurface(icon);
    }
}

// Macros are cursed
#define _STR(x) #x
#define STR(x) _STR(x)

void AppPlatform_sdl::_init(std::string storageDir, SDL_Surface* screen)
{
    _storageDir = storageDir;
    m_screen = screen;

    _iconTexture = nullptr;
    _icon = nullptr;

    m_bShiftPressed[0] = false;
    m_bShiftPressed[1] = false;

    ensureDirectoryExists(_storageDir.c_str());

    m_pSoundSystem = nullptr;
    m_bIsTouchscreen = false;

    _controller = findGameController();

    clearDiff();
}

void AppPlatform_sdl::initSoundSystem()
{
    if (!m_pSoundSystem) {
        LOG_I("Initializing " STR(SOUND_SYSTEM) "...");
        m_pSoundSystem = new SOUND_SYSTEM();
    } else {
        LOG_E("Trying to initialize SoundSystem more than once!");
    }
}

void AppPlatform_sdl::setIcon(const Texture& icon)
{
    if (!icon.m_pixels) return;

    SAFE_DELETE(_iconTexture);
    if (_icon) SDL_FreeSurface(_icon);

    _iconTexture = new Texture(icon);
    _icon = getSurfaceForTexture(_iconTexture);

    if (_icon) SDL_WM_SetIcon(_icon, nullptr);
}

AppPlatform_sdl::~AppPlatform_sdl()
{
    if (_icon) SDL_FreeSurface(_icon);
    SAFE_DELETE(_iconTexture);
    SAFE_DELETE(m_pSoundSystem);
}

SDL_Joystick* AppPlatform_sdl::findGameController()
{
    if (SDL_NumJoysticks() > 0)
        return SDL_JoystickOpen(0);
    return nullptr;
}

SDL_Surface* AppPlatform_sdl::getSurfaceForTexture(const Texture* const texture)
{
    if (!texture) return nullptr;

    void* pixels = texture->m_pixels;
    int width = texture->m_width;
    int height = texture->m_height;
    int depth = 32;

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        pixels, width, height, depth,
        width * 4,
        0x000000FF, 0x0000FF00, 0x00FF0000,
        0xFF000000
    );

    if (!surface)
        LOG_E("Failed loading SDL_Surface from Texture: %s", SDL_GetError());

    return surface;
}

int AppPlatform_sdl::checkLicense()
{
    return 1; // we own the game!!
}

const char* const AppPlatform_sdl::getWindowTitle() const
{
    return "ReMinecraftPE";
}

int AppPlatform_sdl::getScreenWidth() const
{
    return m_screen ? m_screen->w : 0;
}

int AppPlatform_sdl::getScreenHeight() const
{
    return m_screen ? m_screen->h : 0;
}

void AppPlatform_sdl::setMouseGrabbed(bool b)
{
    SDL_WM_GrabInput(b ? SDL_GRAB_ON : SDL_GRAB_OFF);
    clearDiff();
}

void AppPlatform_sdl::setMouseDiff(int x, int y)
{
    xrel += x;
    yrel += y;
}

void AppPlatform_sdl::getMouseDiff(int& x, int& y)
{
    x = xrel;
    y = yrel;
}

void AppPlatform_sdl::clearDiff()
{
    xrel = 0;
    yrel = 0;
}

bool AppPlatform_sdl::shiftPressed()
{
    return m_bShiftPressed[0] || m_bShiftPressed[1];
}

void AppPlatform_sdl::setShiftPressed(bool b, bool isLeft)
{
    m_bShiftPressed[isLeft ? 0 : 1] = b;
}

int AppPlatform_sdl::getUserInputStatus()
{
    return -1;
}

MouseButtonType AppPlatform_sdl::GetMouseButtonType(Uint8 button)
{
    switch (button) {
        case SDL_BUTTON_LEFT: return BUTTON_LEFT;
        case SDL_BUTTON_RIGHT: return BUTTON_RIGHT;
        case SDL_BUTTON_MIDDLE: return BUTTON_MIDDLE;
        default: return BUTTON_NONE;
    }
}

bool AppPlatform_sdl::GetMouseButtonState(const SDL_Event& event)
{
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN: return true;
        case SDL_MOUSEBUTTONUP: return false;
        default: return false;
    }
}

Keyboard::KeyState AppPlatform_sdl::GetKeyState(uint8_t state)
{
    return (state == SDL_RELEASED) ? Keyboard::UP : Keyboard::DOWN;
}

void AppPlatform_sdl::showKeyboard(int, int, int, int)
{
}

void AppPlatform_sdl::hideKeyboard()
{
}

bool AppPlatform_sdl::isTouchscreen() const
{
    return false;
}

bool AppPlatform_sdl::hasGamepad() const
{
    return _controller != nullptr;
}

void AppPlatform_sdl::gameControllerAdded(int index)
{
    if (!_controller)
        _controller = SDL_JoystickOpen(index);
}

void AppPlatform_sdl::gameControllerRemoved(int index)
{
    if (_controller && SDL_JoystickIndex(_controller) == index) {
        SDL_JoystickClose(_controller);
        _controller = findGameController();
    }
}

void AppPlatform_sdl::handleKeyEvent(const SDL_Event& event)
{
    int key = event.key.keysym.sym;
    uint8_t state = event.key.state;

    switch (key) {
        case SDLK_F2:
            if (state == SDL_PRESSED)
                saveScreenshot("", -1, -1);
            return;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            setShiftPressed(state == SDL_PRESSED, key == SDLK_LSHIFT);
            break;
    }

    Keyboard::feed(GetKeyState(state), key);
}

void AppPlatform_sdl::handleButtonEvent(const SDL_Event& event)
{
    if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP)
        Keyboard::feed(GetKeyState(event.jbutton.state), event.jbutton.button);
}

void AppPlatform_sdl::handleControllerAxisEvent(const SDL_Event& event)
{
    if (event.type != SDL_JOYAXISMOTION) return;

    float val = event.jaxis.value / 32767.0f;

    switch (event.jaxis.axis) {
        case 0: Controller::feedStickX(1, true, val); break;
        case 1: Controller::feedStickY(1, true, val); break;
        case 2: Controller::feedStickX(2, true, val); break;
        case 3: Controller::feedStickY(2, true, val); break;
    }
}

AssetFile AppPlatform_sdl::readAssetFile(const std::string& str, bool quiet) const
{
    std::string path = getAssetPath(str);
    SDL_RWops* io = SDL_RWFromFile(path.c_str(), "rb");
    if (!io) {
        if (!quiet) LOG_W("Couldn't find asset file: %s", path.c_str());
        return AssetFile();
    }

    // SDL 1.2-compatible file size calculation
    Sint64 size = SDL_RWseek(io, 0, SEEK_END);
    SDL_RWseek(io, 0, SEEK_SET);
    if (size < 0) {
        if (!quiet) LOG_E("Error determining the size of the asset file!");
        SDL_RWclose(io);
        return AssetFile();
    }

    unsigned char* buf = new unsigned char[size];
    SDL_RWread(io, buf, size, 1);
    SDL_RWclose(io);

    return AssetFile(size, buf);
}

void AppPlatform_sdl::ensureDirectoryExists(const char* path)
{
    struct stat obj;
    if (stat(path, &obj) != 0 || !S_ISDIR(obj.st_mode)) {
#if defined(_WIN32) && !defined(__MINGW32__)
        int ret = XPL_MKDIR(path);
#else
        int ret = XPL_MKDIR(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
        if (ret != 0) {
            LOG_E("Error Creating Directory: %s: %s", path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

/* Save */
void AppPlatform_sdl::saveScreenshot(const std::string& filename, int width, int height)
{
    std::string screenshots = _storageDir + "/screenshots";

    /* Filename with timecode */
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char timeStr[256];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H.%M.%S", timeinfo);

    ensureDirectoryExists(screenshots.c_str());

    std::string path = screenshots + "/";
    std::string file = path + timeStr + ".bmp";
    int num = 1;
    while (XPL_ACCESS(file.c_str(), F_OK) != -1) {
        file = path + SSTR(timeStr << "-" << num << ".bmp");
        num++;
    }

    /* Save */
    if (SDL_SaveBMP(m_screen, file.c_str()) != 0) {
        LOG_E("Screenshot Failed: %s", file.c_str());
    } else {
        LOG_I("Screenshot Saved: %s", file.c_str());
    }
}

/* Load bitmap */
SDL_Surface* AppPlatform_sdl::loadSurface(const std::string& path)
{
    std::string realPath = getAssetPath(path);
    SDL_Surface* surface = SDL_LoadBMP(realPath.c_str());
    if (!surface) {
        LOG_E("Couldn't load BMP: %s", path.c_str());
    }
    return surface;
}

/* Does bitmap exist */
bool AppPlatform_sdl::doesSurfaceExist(const std::string& path) const
{
    std::string realPath = getAssetPath(path);
    std::ifstream f(realPath.c_str());
    return f.good();
}

/* Load bitmap (texture) */
Texture AppPlatform_sdl::loadTexture(const std::string& path, bool bIsRequired)
{
    Texture out;
    out.m_hasAlpha = true;
    out.field_D = 0;

    // Get Full Path
    std::string realPath = getAssetPath(path);

    // Read File
    SDL_RWops *io = SDL_RWFromFile(realPath.c_str(), "rb");
    if (!io)
    {
        LOG_E("Couldn't find file: %s", path.c_str());
        return out;
    }
    Sint64 size = SDL_RWseek(io, 0, RW_SEEK_END);
    SDL_RWseek(io, 0, RW_SEEK_SET);
    unsigned char *file = new unsigned char[size];
    SDL_RWread(io, file, size, 1);
    SDL_RWclose(io);

    // Parse Image
    int width = 0, height = 0, channels = 0;
    stbi_uc *img = stbi_load_from_memory(file, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha);
    delete[] file;
    if (!img)
    {
        // Failed To Parse Image
        LOG_E("The image could not be loaded properly: %s", path.c_str());
        return out;
    }

    // Copy Image
    uint32_t *img2 = new uint32_t[width * height];
    memcpy(img2, img, width * height * sizeof (uint32_t));
    stbi_image_free(img);

    // Create Texture
    out.m_width = width;
    out.m_height = height;
    out.m_pixels = img2;

    // Return
    return out;
}

/* STUBBED */
bool AppPlatform_sdl::doesTextureExist(const std::string&) const
{
    return false;
}

/* STUBBED */
/* Always access FS */
bool AppPlatform_sdl::hasFileSystemAccess()
{
    return true; /* TODO: Match SDL 2.x code (Do I need to fix this, no idea; I just stubbed all the functions at first then started eyeballing shit. )*/
}

void AppPlatform_sdl::recenterMouse()
{
    SDL_WarpMouse(m_screen->w / 2, m_screen->h / 2);
}
