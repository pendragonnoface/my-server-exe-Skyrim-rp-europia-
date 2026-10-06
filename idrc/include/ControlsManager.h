#pragma once

#include <unordered_map>

namespace IDRC {
    enum IDRCKey {
        kInvalid = 0,
        kForward = 1,
        kBack = 2,
        kSprint = 3,
        kStrafeLeft = 4,
        kStrafeRight = 5,
        kDisplayHealth = 6,
        kUp = 7,
        kDown = 8,
        kRun = 9,
        kSneak = 10,
        kJump = 11,
        kToggleAlwaysRun = 12,
        kToggleAutoCombat = 13,
        kActivate = 14,
        kToggleLockReticle = 15,
        kPrimaryTargetMode = 16,
        kToggleCameraLock = 17
    };

    struct DXScanCode{
        uint32_t key;

        explicit DXScanCode(uint32_t a_key) : key(a_key) {
            if (a_key > InputMap::kMaxMacros) { // not a valid DXScanCode
                key = InputMap::kMaxMacros;
            }
        }

        // Default constructor
        DXScanCode() : key(InputMap::kMaxMacros) {}

        // Allow explicit conversion to uint32_t
        explicit operator uint32_t() const { return key; }

        DXScanCode& operator=(uint32_t a_newKey) {
            key = a_newKey;
            return *this;
        }
    
        bool operator==(const DXScanCode& a_other) const {
            return key == a_other.key;
        }
        bool operator==(uint32_t a_otherKey) const {
            return key == a_otherKey;
        }
    };

    struct GamepadButton {
        uint32_t key;

        explicit GamepadButton(uint32_t a_key) : key(a_key) {
            if (InputMap::GamepadMaskToKeycode(a_key) == InputMap::kMaxMacros) {
                key = 255; // value returned by InputMap::GamepadKeycodeToMask() for an invalid gamepad keycode
            }
        }

        // Default constructor
        GamepadButton() : key(255) {} 

        // Allow explicit conversion to uint32_t
        explicit operator uint32_t() const { return key; }

        GamepadButton& operator=(uint32_t a_newKey) {
            key = a_newKey;
            return *this;
        }
    
        bool operator==(const GamepadButton& a_other) const {
            return key == a_other.key;
        }

        bool operator==(uint32_t a_otherKey) const {
            return key == a_otherKey;
        }
    };

    
    class ControlsManager : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        static ControlsManager& GetSingleton() {
            static ControlsManager instance;
            return instance;
        }
        ControlsManager(const ControlsManager&) = delete;
        ControlsManager& operator=(const ControlsManager&) = delete;

        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override;

        void InitializeData();

        bool RegisterForControls(bool a_reRegisterOnLoad = false, bool a_registerFromGoTDragonCompanions = false);

        bool UnregisterForControls();
        
        bool GetIsKeyPressed(IDRCKey a_key) const;

        bool IsThumbstickPressed() const;

        float GetThumbstickAngle() const;

        void SetKeyMapping(const std::string& a_key, const DXScanCode& a_mappedScanCode);

        bool GetControlBlocked() const;

        void SetControlBlocked(bool a_block);

        void SetInitialAutoCombatMode(bool a_auto);

        bool GetInitialAutoCombatMode() const;

    private:
        ControlsManager() = default;
        ~ControlsManager() = default;

        IDRCKey GetMappedIDRCKey(const RE::ButtonEvent* a_buttonEvent);

        bool IsGamepadButtonPressed(const GamepadButton& a_gamepadButton) const;

        DXScanCode GetMappedDXScanCode(IDRCKey a_key) const;

        DXScanCode GetMappedDXScanCode(const RE::ButtonEvent* a_buttonEvent) const;

        void TouchGamepadButton(const RE::ButtonEvent* a_buttonEvent);

        void TouchMouseButton(const RE::ButtonEvent* a_buttonEvent);

        void SetUseSneakKey(bool a_useSneakKey);

        void PrintAllDeviceMappings(RE::BSInputDevice* a_device, uint32_t a_maxKeyCode);

        bool m_controlBlocked = false;
        bool m_isRegistered = false;
        bool m_initialAutoCombatMode = false;
        bool m_useSneakKey = true;

        std::unordered_map<IDRCKey, DXScanCode> m_keyMap;
    }; // class ControlsManager
} // namespace IDRC
