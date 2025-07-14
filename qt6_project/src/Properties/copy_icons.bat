@echo off
REM Copy essential icons from legacy project to Qt6 project

set LEGACY_PATH=C:\Users\karol\Documents\Github_OLD\Item Editor Final\Legacy_App\csharp\Source\Resources
set QT6_PATH=C:\Users\karol\Documents\Github_OLD\Item Editor Final\qt6_project\src\Properties\icons

echo Copying essential icons...

copy "%LEGACY_PATH%\FormIcon.png" "%QT6_PATH%\application.png"
copy "%LEGACY_PATH%\NewIcon.png" "%QT6_PATH%\new.png"
copy "%LEGACY_PATH%\OpenIcon.png" "%QT6_PATH%\open.png"
copy "%LEGACY_PATH%\SaveIcon.png" "%QT6_PATH%\save.png"
copy "%LEGACY_PATH%\SaveAsIcon.png" "%QT6_PATH%\saveas.png"
copy "%LEGACY_PATH%\FindIcon.png" "%QT6_PATH%\find.png"
copy "%LEGACY_PATH%\help.png" "%QT6_PATH%\help.png"
copy "%LEGACY_PATH%\about_background.png" "%QT6_PATH%\about.png"

REM Create simple placeholder icons for missing ones
echo Creating placeholder icons...
copy "%LEGACY_PATH%\FormIcon.png" "%QT6_PATH%\exit.png"
copy "%LEGACY_PATH%\FormIcon.png" "%QT6_PATH%\status_ready.png"
copy "%LEGACY_PATH%\FormIcon.png" "%QT6_PATH%\status_loading.png"
copy "%LEGACY_PATH%\FormIcon.png" "%QT6_PATH%\status_error.png"

echo Icon copying complete!