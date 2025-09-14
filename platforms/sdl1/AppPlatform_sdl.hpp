#pragma once

#include <string>

#include "thirdparty/SDL/SDL.h"
#include "client/app/AppPlatform.hpp"
#include "client/player/input/Mouse.hpp"
#include "client/player/input/Keyboard.hpp"

class AppPlatform_sdl final : public AppPlatform
{
public:
    void _init(std::string storageDir, SDL_Surface* screen);
    AppPlatform_sdl(std::string storageDir, SDL_Surface* screen);
    ~AppPlatform_sdl() override;

    void initSoundSystem() override;

    int checkLicense() override;
    const char* const getWindowTitle() const;
    int getScreenWidth() const override;
    int getScreenHeight() const override;

    SDL_Surface* loadSurface(const std::string& path);
    bool doesSurfaceExist(const std::string& path) const;

    int getUserInputStatus() override;
    SoundSystem* const getSoundSystem() const override { return m_pSoundSystem; }

    void setMouseGrabbed(bool b) override;
    void setMouseDiff(int x, int y);
    void getMouseDiff(int& x, int& y) override;
    void clearDiff() override;

    bool shiftPressed() override;
    void setShiftPressed(bool b, bool isLeft);

    static MouseButtonType GetMouseButtonType(Uint8 button);
    static bool GetMouseButtonState(const SDL_Event& event);
    static Keyboard::KeyState GetKeyState(uint8_t state);

    void showKeyboard(int x, int y, int w, int h) override;
    void hideKeyboard() override;

    bool isTouchscreen() const override;

    // Game controller
    bool hasGamepad() const override;
    SDL_Joystick* getPrimaryGameController() const { return _controller; }
    void setPrimaryGameController(SDL_Joystick* controller) { _controller = controller; }
    void gameControllerAdded(int index);
    void gameControllerRemoved(int index);

    void handleKeyEvent(const SDL_Event& event);
    void handleButtonEvent(const SDL_Event& event);
    void handleControllerAxisEvent(const SDL_Event& event);

    // Read Sounds
    AssetFile readAssetFile(const std::string&, bool) const override;

    void saveScreenshot(const std::string& fileName, int width, int height) override;

    Texture loadTexture(const std::string& path, bool b = false) override;
    bool doesTextureExist(const std::string& path) const override;

    bool hasFileSystemAccess() override;

    void recenterMouse() override;

private:
    SDL_Surface* m_screen;  /* Surface */

    SDL_Joystick* _controller;

    const Texture* _iconTexture;
    SDL_Surface* _icon;

    bool m_bShiftPressed[2];

    int xrel;
    int yrel;

    SoundSystem* m_pSoundSystem;

    bool m_bIsTouchscreen;

    SDL_Joystick* findGameController();

    static SDL_Surface* getSurfaceForTexture(const Texture* const texture);

    std::string _storageDir;

    void ensureDirectoryExists(const char* path);

    void setIcon(const Texture& icon);
};
