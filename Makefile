install:
	dkms install .

uninstall:
	dkms remove hp-omen-wmi/1.0 --all

all: install
