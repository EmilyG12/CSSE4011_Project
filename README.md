# Artemis Lime  
###### Handheld Bluetooth Fighting Game

## Handheld Trainer:
> Use the m5 core2 as a handheld game console, it advertises details about its user over bluetooth, and can search for other nearby devices to initiate fights with them. There are a variety of creatures you can train to perform different combat moves. (the master lists, and user details saved/loaded from SD card). It uses the device's touch screen and capacitive buttons as a GUI or USB UART to access a debug shell.

## Arena:
> Using the nrf board you can either run a command line variant of the trainer or host an arena, arenas snoop on nearby fights and publish fight data and statistics to the web dashboard. They can also host tournaments that trainers can connect to, and be sorted into a bracket.

## File Structure
```shell
.
├── app # The handheld/arena program.
│    ├── boards
│    └── src
├── libs # Some generic libraries.
│    └── bt # A general bluetooth lib.
├── milestone # Content for the Milestone
└── scripts # Helpful scripts for development
```

## Contributors
Ibtisam Aslam - 46476470  
Emily Good - 47453311  
Sioryn Willett - 43926200  

## Install
```shell
$ west build -b m5stack_core2/esp32/procpu app -p
$ west flash
```

## Dependencies
- [Zephyr/West](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
- `$ pip install asyncio pykwalify pyserial requests numpy pyyaml`