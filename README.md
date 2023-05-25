# devine

Devine is an alternative to the Windows device manager that retains the look and feel of the original but has several addition features. 

**Additional Features.** 

  Additional information in the tree view. When working with multiple device instances where the default tool provides a single name, 
  specific instances can more easily be identified.

  Define the columns to be shown in the tree view. 

  A filter to focus on the device instances you are interested in. 

  State transitions highlight added / removed device instances. New instances show green for a few seconds, removed instances show red 
  before being removed from the view. 
  
  Multi-select; disable, enable or uninstall multiple device instances in one operation. 


**Limitations.** 

  Some of the views supported by the Windows tool are not supported.
  Devine only shows information from the local computer.

**Licencing.** 

  The source code is supplied as freeware. Do what you want at your own risk for your own use, either for personal use or within your own company
  but not for resale. 

  A prebuilt version is supplied under the terms of the GNU LGPL as this utilises Qt which has the same Licence.

**Building.** 

  You will need Qt 6. We use Qt's qmake tool to create a Visual Studio project which is then compiled with Visual Studio. 

  From a command window with Qt and Visual Studio installed

    qmake devine.pro
    devenv devine.vcxproj 

  It should also be possible to open devine.pro with Qt Creator and build directly from there but we haven't tried that. 


**Prebuilt package.** 

  The ./package folder holds a prebuilt package devine.zip. 
  Simply extract all files from this and execute devine.exe. 

  If you don't already have Visual Studio runtime libraries installed on your computer run the contained vc_redist.x64.exe to install these. 

**UAC requirements.** 

  To use the update features of devine you will need to run devine.exe as an administrator. 
  In the pre-built version the manifest is set to enforce this, but we have seen inconsistent behaviour on Windows 11. 
  If you see issues we suggest creating a shortcut to access Devine with administrator rights enforced. 

** Filter options. **

  Combine multiple search terms by using a pipe symbol "|" as the OR operator and spaces as the AND operator. 

  Examples 

    aaa|bbb        show all entries containing either the term aaa OR bbe 

    ccc ddd        show all entries containing both the terms ccc AND ddd 

    eee fff|ggg    show all entries containing the terms eee and fff in a single entry or ggg

                   in other words AND (space) is evaluated first, using () to illustrate order of evaluation 

                        (eee fff)|ggg 
