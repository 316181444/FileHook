@echo off
REM cd "C:\Documents\AndroidSdk\platform-tools"
REM C:\Documents\AndroidSdk\platform-tools\adb shell "su -c 'chmod 777 /data/data/com.hxms.file/lib'"
echo begin copy
C:\Documents\AndroidSdk\platform-tools\adb push libFileHook.cy.so /data/data/com.hxms.file/lib/
echo copy finish
pause