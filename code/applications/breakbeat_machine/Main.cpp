/*
  ==============================================================================

    This file was auto-generated and contains the startup code for a PIP.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "BreakbeatMaker.h"
#include "../../ui/MainWindow.h"
#include "../../ui/CustomLookAndFeel.h"

class Application : public juce::JUCEApplication
{
public:
    //==============================================================================
    Application() {}

    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION_STRING; }

    void initialise (const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
        
        mBreakbeatContentComponent = new BreakbeatContentComponent(mDefaultDeviceManager, mRecentFiles);
        
        loadProperties();
        mMainWindow.reset (new MainWindow (getApplicationName(), mBreakbeatContentComponent, mDefaultDeviceManager));
        mMainWindow->setLookAndFeel(&mCustomLookAndFeel);
        
        juce::MessageManager::callAsync([this]()
        {
            if(mRecentFiles.getNumFiles() && mRecentFiles.getFile(0).existsAsFile())
            {
                auto path = mRecentFiles.getFile(0).getFullPathName();
                mBreakbeatContentComponent->newFileOpened(path);
            }
        });
    }
    void shutdown() override
    {
        saveProperties();
        #if JUCE_MAC && (!defined(JUCE_IOS))
                juce::MenuBarModel::setMacMainMenu(nullptr);
        #endif
        
        mMainWindow->setLookAndFeel(nullptr);
        mMainWindow = nullptr;
    }
    
    void saveProperties()
    {
        auto xml = std::make_unique<juce::XmlElement>("BreakbeatMaker");
        if(xml == nullptr)
        {
            return;
        }
        
        xml->setAttribute("recentFiles", mRecentFiles.toString());
        if(std::unique_ptr<juce::XmlElement> markerXml = mBreakbeatContentComponent->toXml())
        {
            xml->addChildElement(markerXml.release());
        }
        xml->writeTo(mPropertyFile);
    }
    
    void loadProperties()
    {
        mPropertyFile =
            juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("Application Support").getChildFile("toous").getChildFile("BreakbeatMaker.xml");
        
        if(!mPropertyFile.existsAsFile())
        {
            mPropertyFile.create();
        }
        
        auto xml = juce::parseXML(mPropertyFile);
        if(xml != nullptr && xml->hasTagName("BreakbeatMaker"))
        {
            mRecentFiles.restoreFromString(xml->getStringAttribute("recentFiles"));
            
            auto* sliceXml = xml->getChildByName("SliceManager::Slices");
            if(sliceXml != nullptr)
            {
                mBreakbeatContentComponent->fromXml(*sliceXml);
            }
        }
        
        mRecentFiles.removeNonExistentFiles();
    }

    std::unique_ptr<MainWindow> mMainWindow;
    
    juce::RecentlyOpenedFilesList mRecentFiles;
    juce::File mPropertyFile;
    
    CustomLookAndFeel mCustomLookAndFeel;
    juce::AudioDeviceManager mDefaultDeviceManager;
    
    BreakbeatContentComponent* mBreakbeatContentComponent;
};

//==============================================================================
START_JUCE_APPLICATION (Application)
