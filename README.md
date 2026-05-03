# MASMORRA

A classic terminal *Dungeon Crawler* encapsulated in a monolithic C file.

## 🚀 How to Play

You can jump right into the dungeon by downloading the pre-compiled binary for your system, or compile it from source if you prefer!

### 📥 Download & Play (Recommended)

1. Go to the [Releases page](https://github.com/paixonite/MASMORRA/releases) of this repository.
2. Download the correct file for your operating system:
   - **Windows:** Download `masmorra_windows.exe` and simply double-click it (or run it via cmd/PowerShell).
   - **Linux:** Download the `masmorra_linux` binary. Open your terminal, grant execution permissions, and run it:
     ```bash
     chmod +x masmorra
     ./masmorra
     ```
     
### 🛠️ Compiling from Source

Since the project has zero external dependencies and is structured in a single file, compilation is instant. You only need an installed C compiler (such as GCC).

#### Linux / macOS
```bash
# Clone the repository
git clone [https://github.com/paixonite/MASMORRA.git](https://github.com/paixonite/MASMORRA.git)
cd MASMORRA

# Compile
gcc MASMORRA.c -o masmorra -Wall

# Play
./masmorra
```

#### Windows
If you are using MinGW, the command in the terminal (or PowerShell) is very similar:

```cmd
gcc MASMORRA.c -o masmorra.exe
masmorra.exe
```

## 👤 Author

- **Vítor Paixão** - [https://github.com/paixonite](https://github.com/paixonite)
