# omen-keyboard-rgb-linux
Simple linux driver for omen laptops with 4 zone keyboard rgb.

## Index

* [About the Project](#about-the-project)
* [Installation](#installation)
* [Useful resources](#useful-resources)

### About the Project
This is a simple and lightweight Linux kernel driver that allows you to control the backlight/rgb of newer omen laptops with 4 zone keyboards.

### Installation

1. Install dependencies  
You will need to install dkms and kernel headers. Please refer to your distro's documentation for more instructions.

2. Clone the repository and enter the directory created
```sh
git clone https://github.com/TitanHZZ/omen-keyboard-rgb-linux.git
cd omen-keyboard-rgb-linux
```

3. Build the project
```sh
sudo make install
```
**Note**: This kernel driver will automatically recompile with every kernel update. No need for manual compilation.

4. **Optional**: Make driver load at boot
```sh
sudo echo "hp-omen-wmi" >> /etc/modules-load.d/hp-omen-wmi.conf
```

### Useful resources
[omen-cli](https://github.com/thebongy/omen-cli) - C# Windows utility  
[hp-omen-linux-module](https://github.com/pelrun/hp-omen-linux-module) - Other Linux kernel driver
