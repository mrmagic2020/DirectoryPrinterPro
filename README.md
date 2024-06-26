# DirectoryPrinterPro

<div align="center">

<img alt="--use-prev-cmd" src="https://github.com/mrmagic2020/DirectoryPrinterPro/blob/main/assets/--use-prev-cmd.png?raw=true" width="400">

</div>

## Installation

`Download` or `clone` the [repository](https://github.com/mrmagic2020/DirectoryPrinterPro) to your local machine.

### Via CMake

```bash
cd /path/to/DirectoryPrinterPro/build
cmake ..
sudo cmake --build .
sudo cmake --install .
```

This will install the `printdir` executable in the `/usr/local/bin` directory.

### Via script (CMake required)

In your terminal, enter `sh /path/to/DirectoryPrinterPro/install.sh`. This basically runs the commands above.

## Usage

Navigate to the directory you want to print and run the following command:

`printdir [OPTIONS]`

### Options

`-h, --help` - Print help message and exit.

`--to-file` - Print the output to a file under the root directory.

`-d,--depth INT` - Set recursion depth. A negative value means infinite depth.

`-n, --name TEXT` - Set the root directory name. Only affects the output.

`--ignore TEXT ...` - Set the list of files/directories to be ignored. (e.g. `.vscode`, `.git`, `build`, etc.)

`--no-content TEXT ...` - Ignore the contents of specific directories. (Only prints the directory name.)

`--use-prev-cmd` - Use the same command executed previously. Commands without this flag and with the --to-file flag will be stored in the file. This will ignore all other flags and options.
