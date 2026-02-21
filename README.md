# shorkdir

A lightweight Linux terminal-based file browser. It is designed to provide simple directory browsing and navigation, and provided `file` is installed, it can also identify and describe a selected file. It is primarily written for use with SHORK family operating systems like [SHORK 486](https://github.com/SharktasticA/SHORK-486), designed to be minimal and not taxing on 486-era hardware, and is statically linked. But it should work on modern Linux distributions just fine.



## Todo

shorkdir is a work-in-progress, and I acknowledge some present shortcomings I intend to address:

* Exiting shorkdir does not land the user in the directory it was last viewing (if different to the start directory).
* shorkdir cannot do anything meaningful with files. At least for text files, it should identify any common editors installed on the system and provide an option to open the file with one.
* shorkdir can produce flickering on some hardware/terminal emulations, usually on older/slower hardware. shorkdir should be modified to only redraw what is needed instead of always redrawing everything.



## Building

### Requirements

You just need a C compiler (tested with GCC and i486-linux-musl-cross)

### Compilation

Simply run `make`

### Installation

Run `make install` to install to `/usr/bin` (you may need `sudo` if not installing as root). If you want to install it elsewhere, you can override the install location prefix like `make PREFIX=/usr/local install`.



## Running

Simply run `shorkdir` to use. Press "?" to get a list of key binds and directory entry type guide.



## Screenshots

<table style="table-layout: fixed; width: 100%;">
  <tr>
    <td style="width: 50%; text-align: center;"><img src="screenshots/86box_root_shork-486.png" style="width: 100%;" /></td>
    <td style="width: 50%; text-align: center;"><img src="screenshots/86box_usr_bin_shork-486.png" style="width: 100%;" /></td>
  </tr>
  <tr>
    <td style="width: 50%;">Browsing root directory</td>
    <td style="width: 50%;">Browsing /usr/bin</td>
  </tr>
</table>

<table style="table-layout: fixed; width: 100%;">
  <tr>
    <td style="width: 50%; text-align: center;"><img src="screenshots/86box_help_shork-486.png" style="width: 100%;" /></td>
    <td style="width: 50%; text-align: center;"><img src="screenshots/86box_inspect_ascii_shork-486.png" style="width: 100%;" /></td>
  </tr>
  <tr>
    <td style="width: 50%;">Help screen</td>
    <td style="width: 50%;">Inspect screen (text file example)</td>
  </tr>
</table>


<table style="table-layout: fixed; width: 100%;">
  <tr>
    <td style="width: 50%; text-align: center;"><img src="screenshots/86box_inspect_executable_shork-486.png" style="width: 100%;" /></td>
    <td style="width: 50%; text-align: center;"><img src="screenshots/86box_inspect_symlink_shork-486.png" style="width: 100%;" /></td>
  </tr>
  <tr>
    <td style="width: 50%;">Inspect screen (executable example)</td>
    <td style="width: 50%;">Inspect screen (symbolic link example)</td>
  </tr>
</table>
