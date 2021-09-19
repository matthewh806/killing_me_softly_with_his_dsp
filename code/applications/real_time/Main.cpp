#include "../../ui/MainWindow.h"
#include "../../ui/CustomLookAndFeel.h"
#include "MainComponent.h"

//==============================================================================
class GuiAppApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    GuiAppApplication() {}

    // We inject these as compile definitions from the CMakeLists.txt
    // If you've enabled the juce header with `juce_generate_juce_header(<thisTarget>)`
    // you could `#include <JuceHeader.h>` and use `ProjectInfo::projectName` etc. instead.
    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
        juce::ignoreUnused (commandLine);

        mainWindow.reset (new MainWindow (getApplicationName(), new MainComponent(mDefaultDeviceManager), mDefaultDeviceManager));
        
        mainWindow->setLookAndFeel(&customLookAndFeel);
    }

    void shutdown() override
    {
#if JUCE_MAC && (!defined(JUCE_IOS))
        juce::MenuBarModel::setMacMainMenu(nullptr);
#endif
        mainWindow->setLookAndFeel(nullptr);
        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused (commandLine);
    }

private:
    juce::AudioDeviceManager mDefaultDeviceManager;
    
    std::unique_ptr<MainWindow> mainWindow;
    CustomLookAndFeel customLookAndFeel;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (GuiAppApplication)
