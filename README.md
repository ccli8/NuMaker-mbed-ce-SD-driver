# Block device driver for SD card supported by Nuvoton platforms

This driver implements [BlockDevice class](https://github.com/ARMmbed/mbed-os/blob/master/features/filesystem/bd/BlockDevice.h)
introduced with mbed OS 5.4 for SD card running in SD bus mode on Nuvoton platforms which support it.

ARM mbed team also releases an official [SD driver](https://github.com/armmbed/sd-driver), but it supports SPI bus mode 
rather than SD bus mode.

## Use with FAT file system
The [SD file system example](https://developer.mbed.org/teams/Nuvoton/code/NuMaker-mbed-SD-FileSystem-example/) is cloned from
the [FAT file system example](https://github.com/armmbed/mbed-os-example-fat-filesystem) released by ARM mbed team and
is modified to use this SD block device to back the FAT file system.

## Support Nuvoton platforms
- [NuMaker-PFM-NUC472](https://developer.mbed.org/platforms/Nuvoton-NUC472/)
- NuMaker-PFM-M487

