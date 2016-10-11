# 3DS FS Tester

A program for testing 3DS FS service. It can be also used as a archive explorer and for managing save data.

### Build

This program is designed for a CIA build, so that it can manage a private save data and extsave for test, and so that it has priviledge to access save data of system and other programs. Building as 3dsx is also possible, but I haven't tried yet.

To build this program, you need: `DevKitPro` with the latest git version `ctrulib` (to build the ELF), and `makerom` from `Project_CTR` (to build the CIA). Set `DEVKITARM` and `MAKEROM` in the environment. Then run `make`.

### Usage

Upper screen: general message and prompt.

Lower screen: logs of service function call.

When exploring the archive (by pressing A at the main menu), one can press START and input a command (via software keyboard) to call FS functions. Current available commands are:
   - `c p<path> a<attributes> s<size>` calls `CreateFile`;
   - `cd p<path> a<attributes>` calls `CreateDirectory`;
   - `d p<path>` calls `DeleteFile`;
   - `dd p<path>` calls `DeleteDirectory`;
   - `dr p<path>` calls `DeleteDirectoryRecursively`;
   - `o p<path> a<attributes> f<flags>` calls `OpenFile` (and `CloseFile` if necessary);
   - `od p<path>` calls `OpenDirectory` (and `CloseDirectory` if necessary).

Options in these commands are optional. `<path>` is a string without space. `<attributes>` and `<size>` are a raw u32 value, `<flags>` is a character sequence composed by `r`(read), `w`(write) and/or `c`(create).

Here are some example:

```
cd p/a_folder
c p/a_folder/a_file s0x100
o p/a_folder/a_file a0 frw
dr p/a_folder
```
