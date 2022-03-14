# GHDL_GTKWAVE_HELPER
This is a simple interface for GHDL (VHDL compiler) and GTKWAVE (Graphic viewer). Some stuff was made based on what the teacher asked for.

### Tutorial:
This is the home screen
![image](https://user-images.githubusercontent.com/17103500/158256149-bcae32eb-9add-4916-82cb-1a3dd0c869bd.png)
The first thing you'll do is `install`
![image](https://user-images.githubusercontent.com/17103500/158256207-8823a821-d2f5-43bd-93f1-e70fbf7add3d.png)
After installing, this is the last screen
![image](https://user-images.githubusercontent.com/17103500/158256243-7cc83b7c-cfe1-4077-a955-c06380d0a178.png)
Now you just need to call `ghdl`, `gtkw` or `help`. Help always works.
The `ghdl` ou `gtkw` is replaced with the appdata path used by the app before to "install" them.
You can also use an immersive mode, aka call multiple times the one selected with parameters.
```
ghdl --version
ghdl -a porta_e.vhd
ghdl -a porta_e_tb.vhd
ghdl -r porta_e_tb --wave=simulacao.ghw 
```
is equivalent of
```
ghdl
--version
-a porta_e.vhd
-a porta_e_tb.vhd
-r porta_e_tb --wave=simulacao.ghw 
~
```
The `~` exit the immersive mode.

## How to clean up what this app downloaded:
All files are stored at `%appdata%/GHDLGTKW_by_Lunaris`. It was not well thought, but it works, so let's keep it like that.
