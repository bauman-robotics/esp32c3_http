Step 1. Install Prerequisites
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#step-1-install-prerequisites
https://microsin.net/programming/arm/esp-idf-ubuntu-install.html
https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md
https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/basic_use.md

requirements.txt 

####gdbgui==0.13.2.0 - закоментировал. 
(была ошибка, думаю, уже установлена другая версия)
<<<<<<< HEAD
=======

//==== Старая версия ===============
git clone -b v4.4.3 --recursive https://github.com/espressif/esp-idf.git

git clone --recursive https://github.com/espressif/esp-idf.git

./install.sh all
./install.sh esp32

chmod +x export.sh

. ./export.sh
>>>>>>> d969f0f (before receive val)
