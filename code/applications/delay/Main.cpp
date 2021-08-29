#include "MainComponent.h"
#include "../../ui/CustomLookAndFeel.h"

//==============================================================================
class GuiAppApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    GuiAppApplication() {}
    
    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);
        mainWindow.reset (new MainWindow (getApplicationName(), mDefaultDeviceManager));
        
        mainWindow->setLookAndFeel(&customLookAndFeel);

        mCommandManager.registerAllCommandsForTarget(mainWindow.get());
        mMenuModel.setApplicationCommandManagerToWatch(&mCommandManager);
#if JUCE_MAC && (!defined(JUCE_IOS))
        juce::MenuBarModel::setMacMainMenu(&mMenuModel);
#endif
    }

    void shutdown() override
    {
        juce::MenuBarModel::setMacMainMenu(nullptr);
        mainWindow->setLookAndFeel(nullptr);
        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);
    }
    
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
            return {"Settings"};
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

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow
    : public juce::DocumentWindow
    , public juce::ApplicationCommandTarget
    {
    public:
        explicit MainWindow (juce::String name, juce::AudioDeviceManager& deviceManager)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
            , mAudioDeviceComponent(deviceManager, 0, 256, 0, 256, false, false, false, true)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(deviceManager), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            mAudioDeviceComponent.setSize(300, 300);
            setVisible (true);
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
        };

        void getAllCommands(juce::Array<juce::CommandID>& commands) override
        {
            const juce::CommandID ids[] =
            {
                CommandIds::AudioDeviceSettings
            };
            
            commands.addArray(ids, numElementsInArray(ids));
        }
        
        void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override
        {
            switch (commandID)
            {
                case CommandIds::AudioDeviceSettings:
                {
                    result.setInfo(juce::translate("Audio Device Settings"), juce::translate("Configure audio device settings"), "Settings", 0);
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
        juce::AudioDeviceSelectorComponent mAudioDeviceComponent;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };
    

private:
    juce::AudioDeviceManager mDefaultDeviceManager;
    
    std::unique_ptr<MainWindow> mainWindow;
    juce::ApplicationCommandManager mCommandManager;
    MainMenuModel mMenuModel {mCommandManager};
    CustomLookAndFeel customLookAndFeel;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (GuiAppApplication)
