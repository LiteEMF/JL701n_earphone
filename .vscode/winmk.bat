SET SCRIPT_PATH=%~dp0%
SET PATH=%SCRIPT_PATH%..\sdk\tools\utils;%PATH%
make -C sdk -f "%1" "%2" -j %NUMBER_OF_PROCESSORS%
