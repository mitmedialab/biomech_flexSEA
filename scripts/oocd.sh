
echo
echo [running openocd...]
openocd -s ~/Desktop/FlexSEA/embedded-arm/openocd-bin/share/openocd/scripts/ -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg
echo