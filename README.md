# RV32I-software
This project serves as a foundation for you to write programs to be run on the self-made RV32I computer. The project contains some runtime libraries ,which encapsulate some machine-level operations, for you to link to your program. Also, you can run your program on the provided RV32I-emulator to verify it.

## Dependencies

The [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) is needed to compile the source code to RISC-V instructions.

## Usage
Write source code and save it in `apps/`(for example, `apps/tty.c`). Then build it with command`make xxx`(for example, `make tty`).

The instruction and data will be output to`target/xxx_rom.hex` and `target/xxx_ram.hex`, these are the data you should load to the ROM and RAM when you run it on the simulator.

Finally, run the simulator.
```bash
$ ./rv32i_emulator ./target/xxx_rom.hex ./target/xxx_ram.hex
```
Now you can see how your program will act on the self-made computer.

## About the self-made computer

This CPU supports most RV32I instructions(without priviledged instructions like `ecall`, `csrrsi`, ...).

The address space is divided as follows.

```
0x00000000~0x00000FFF RAM
0x00100000~ character-mode-video-memory
0x00200000 character-mode-scan-baseline
0x00300000~ gui-mode-video-memory
0x00400000 gui-character-mode-control
0x08000000~0x08000FFF Instruction_ROM
0xbad00000 keyboard
```

The screen resolution is 640x480, and each character occupies a 8x16 region, so the whole screen can at most show 80x30 characters. Maybe we could regard the character-mode video memory as a `char[30][80]` array, but to make the decode process more convenient for hardware, the video memory is a `char[30][128]` array whose base-address is `0x00100000`, and only the first 80 bytes are writeable for each row.

In character-mode, when the screen is full, we may want the screen to "slide down" for a new row, this is implemented by changing the scan-baseline. For example, when all rows are full, we go back to write the row 0 and change the scan-baseline to 1.

In gui-mode, due to the restriction of memory resource, we can't save the RGB information for all 640x480 pixels. Instead, we treat a 8x8 square as a logical "pixel",  and now the screen only has 80x60 "pixels". Caution: the RGB data only occupies 12-bit, the distribution is `11~RRRRGGGGBBBB~0`

So, similar to character-mode, we regard the gui-mode video memory as a `short[60][128]` array whose base-address is `0x00300000`.

If you want to change from character mode to gui-mode, write 1 to `0x00400000`. Write 0 to `0x00400000` to change back.

The keyboard address is `0xbad00000`, you can just read that space to get scan-code from keyboard buffer. If the buffer is empty now, you will get 0 returned.

## TODO

**The whole project is not fully tested, so there may be many BUGS right now.**

1. The emulator now just support the keyboard input of numbers and lowercase English letters(without `Shift`, `Capslock`, ...).
2. The runtime library now only provides some character-mode functions like `putchark(int)` and `getchark()`, but no gui-mode encapsulation function is provided. Maybe you could add some?
3. Develop some new applications to be run on the machine!