#pragma once

#include "JuceHeader.h"
#include "BreakbeatMaker.h"

namespace OUS
{
    class MainMenuModel
    : public juce::MenuBarModel
    {
    public:
        MainMenuModel(juce::ApplicationCommandManager& commandManager)
        : mCommandManager(commandManager)
        {
        }

        ~MainMenuModel() override = default;

        juce::StringArray getMenuBarNames() override
        {
            return {"File", "Settings"};
        }

        juce::PopupMenu getMenuForIndex(int index, juce::String const& categoryName) override
        {
            juce::ignoreUnused(index);
            juce::PopupMenu menu;

            auto const commandIds = mCommandManager.getCommandsInCategory(categoryName);
            for(auto const commandId : commandIds)
            {
                menu.addCommandItem(&mCommandManager, commandId);
            }

            return menu;
        }

        void menuItemSelected(int itemId, int index) override
        {
            juce::ignoreUnused(itemId, index);
        }

    private:
        juce::ApplicationCommandManager& mCommandManager;
    };

    class BreakbeatMainWindow
    : public juce::DocumentWindow
    , public juce::ApplicationCommandTarget
    , private juce::ChangeListener
    {
    public:
        explicit BreakbeatMainWindow(juce::String name, BreakbeatContentComponent* breakbeatComponent, juce::AudioDeviceManager& deviceManager)
        : DocumentWindow(name,
                         juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons)
        , mAudioDeviceComponent(deviceManager, 0, 256, 0, 256, false, false, false, true)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(breakbeatComponent, true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            mCommandManager.registerAllCommandsForTarget(this);
            mMenuModel.setApplicationCommandManagerToWatch(&mCommandManager);
#if JUCE_MAC && (!defined(JUCE_IOS))
            juce::MenuBarModel::setMacMainMenu(&mMenuModel);
#endif

            auto audioSettingsXml = loadDeviceSettings();
            mAudioDeviceComponent.deviceManager.addChangeListener(this);

            mBreakbeatComponent = breakbeatComponent;
            breakbeatComponent->setAudioChannels(2, 2, audioSettingsXml.get());

            mAudioDeviceComponent.setSize(300, 300);
            setVisible(true);
        }

        ~BreakbeatMainWindow() override
        {
            mAudioDeviceComponent.deviceManager.removeChangeListener(this);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        juce::ApplicationCommandTarget* getNextCommandTarget() override
        {
            return nullptr;
        }

        enum CommandIds
        {
            AudioDeviceSettings = 0x2002000,
            ExportSlices = 0x2003000
        };

        void getAllCommands(juce::Array<juce::CommandID>& commands) override
        {
            const juce::CommandID ids[] =
                {
                    CommandIds::AudioDeviceSettings,
                    CommandIds::ExportSlices
                };

            commands.addArray(ids, numElementsInArray(ids));
        }

        void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override
        {
            switch(commandID)
            {
                case CommandIds::AudioDeviceSettings:
                {
                    result.setInfo(juce::translate("Audio Device Settings"), juce::translate("Configure audio device settings"), "Settings", 0);
                    break;
                }
                case CommandIds::ExportSlices:
                {
                    result.setInfo(juce::translate("Export Slices"), juce::translate("Export Current Audio Slices"), "File", 0);
                    break;
                }
            }
        }

        bool perform(juce::ApplicationCommandTarget::InvocationInfo const& info) override
        {
            switch(info.commandID)
            {
                case CommandIds::AudioDeviceSettings:
                {
                    showDeviceSettings();
                    return true;
                }
                case CommandIds::ExportSlices:
                {
                    mBreakbeatComponent->exportAudioSlices();
                    return true;
                }
            }

            return false;
        }

        void showDeviceSettings()
        {
            juce::DialogWindow::LaunchOptions o;
            o.dialogTitle = juce::translate("Audio Device Settings");

            o.content.setNonOwned(&mAudioDeviceComponent);
            o.componentToCentreAround = nullptr;
            o.dialogBackgroundColour = juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
            o.escapeKeyTriggersCloseButton = true;
            o.useNativeTitleBar = false;
            o.resizable = false;
            o.useBottomRightCornerResizer = false;

            o.launchAsync();
        }

    private:
        void changeListenerCallback(juce::ChangeBroadcaster* source) override
        {
            if(source == &mAudioDeviceComponent.deviceManager)
            {
                saveDeviceSettings();
            }
        }

        std::unique_ptr<XmlElement> loadDeviceSettings()
        {
            auto appDataDir = juce::File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("softly_dsp");
            if(!appDataDir.exists())
            {
                return nullptr;
            }

            auto xmlFile = appDataDir.getChildFile("device_settings.xml");
            if(!xmlFile.exists())
            {
                return nullptr;
            }

            return juce::parseXML(xmlFile);
            ;
        }

        void saveDeviceSettings()
        {
            auto const deviceState = mAudioDeviceComponent.deviceManager.createStateXml();
            if(deviceState == nullptr)
            {
                return;
            }

            auto appDataDir = juce::File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("softly_dsp");
            if(!appDataDir.exists())
            {
                appDataDir.createDirectory();
            }

            if(!appDataDir.isDirectory())
            {
                // error just return;
                return;
            }

            auto xmlFile = appDataDir.getChildFile("device_settings.xml");
            xmlFile.create();
            deviceState->writeTo(xmlFile);
        }

        juce::AudioDeviceSelectorComponent mAudioDeviceComponent;

        juce::ApplicationCommandManager mCommandManager;
        MainMenuModel mMenuModel{mCommandManager};
        
        BreakbeatContentComponent* mBreakbeatComponent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BreakbeatMainWindow)
    };
} // namespace OUS
