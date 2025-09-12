# OMAP4 Cortex-M3 (aka Ducati) bringup

I've had Pandaboard for ages, but have always ran Linux/*BSD/RISCOS/baremetal
stuff on its main Cortex-A9 cores. As we well know OMAP4 has got 2x Cortex-A9,
2x Cortex-M3, and a TI DSP. Usually you interact with the Ducati via Linux's
remoteproc / rpmsg, but it's time to ditch Linux and do it in baremetal
(with support from u-boot). Stay tuned, next time we will try to run anything
on TI DSP (mostly run delay slots lol).

## WTF is this?
Baremetal example how to enable clocks and power and do mappings for the
Cortex-M3 so it's able to run a program that will print a symbol "!" on UART3
(the one with connector on the side).


## Example
```
=> loadx
## Ready for binary (xmodem) download to 0x82000000 at 115200 bps...
CSending m3.bin, 2 blocks: Give your local XMODEM receive command now.
Bytes Sent:    384   BPS:244

Transfer complete
## Total Size      = 0x00000178 = 376 Bytes
=> go 0x82000000
## Starting application at 0x82000000 ...
!
```
&nbsp;&nbsp;☝ yes, this one

## TI
Please think twice before buying anything from TI (or from anyone, really).
These guys will abandon it and delete all their repositories before you know it.
Everything that's not in mainline Linux is basically long gone.
No chance of running Android with GPU and cool TI stuff anymore.
You're buying electronic waste, you just don't know it yet. Rather go get some
board from your favorite e-waste pile, I am sure there's a nice one around you.
Cheers.
