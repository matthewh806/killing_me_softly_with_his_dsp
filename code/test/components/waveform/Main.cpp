#include "../../../ui/CustomLookAndFeel.h"
#include "../../../ui/MainWindow.h"
#include "MainComponent.h"

namespace OUS
{
    //==============================================================================
    class WaveformComponentTestApp : public juce::JUCEApplication
    {
    public:
        //==============================================================================
        WaveformComponentTestApp() {}

        const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
        const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
        bool moreThanOneInstanceAllowed() override { return true; }

        //==============================================================================
        void initialise(const juce::String& commandLine) override
        {
            juce::ignoreUnused(commandLine);
            mainWindow.reset(new MainWindow(getApplicationName(), new WaveformComponentTest(mDefaultDeviceManager), mDefaultDeviceManager));

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
            quit();
        }

        void anotherInstanceStarted(const juce::String& commandLine) override
        {
            juce::ignoreUnused(commandLine);
        }

    private:
        juce::AudioDeviceManager mDefaultDeviceManager;

        std::unique_ptr<MainWindow> mainWindow;
        CustomLookAndFeel customLookAndFeel;
    };

} // namespace OUS

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(OUS::WaveformComponentTestApp)
