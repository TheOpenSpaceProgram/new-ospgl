# OSPGL
This repo contains all code for the OSPGL program, despite being called 'new-ospgl', as the original ospgl is now legacy code.

![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/TheOpenSpaceProgram/new-ospgl/c-cpp.yml?branch=master)
![GitHub issues](https://img.shields.io/github/issues/TheOpenSpaceProgram/new-ospgl)


## Collaborating
The project has a pretty big scope, so collaboration of all kinds is needed, from planning to coding and artistic work. 
You can check what's needed on the Issues and Projects. Feel free to create new entries with ideas!

---

# Building
We use CMake for building, so the procedure should be similar on all systems.

OpenSSL is a dependency (for ospm), on most linux distributions it's probably available on the package manager (it will probably come preinstalled),
on windows it needs to be manually downloaded (Here's a good binary compilation: https://slproweb.com/download/Win64OpenSSL-1_1_1f.exe)

``binutils-dev`` is needed for libbfd. You can disable this requirement by disabling
stacktraces at compile time.

You may also need `libx11-dev`, `xorg-dev` and `libgl-mesa-dev` on linux, but these are usually installed on most systems. 

Run these commands to obtain the source:

```
git clone --recurse-submodules https://github.com/TheOpenSpaceProgram/new-ospgl.git
cd new-ospgl
```

## Linux
```
cmake .
cmake .
cmake --build . 
```
Running CMake twice is needed only the first time you build, otherwise `assimp` will build all importers and make the build time really long
You may want to speed up the build using `cmake --build . -- -j [Number of CPUs]`

**Note**: If you run cmake in a subdirectory (for example, `build`), then you may want to copy the `compile_commands.json` file to the root directory for code completion in VIM and other tools. 
If your executable is not in the root directory, move it there as the game requires the `res` folder and the `udata` folder. You can always change these running `./osp` with command line arguments (check `OSP.cpp`) if you want to keep the executables on a different folder.

**Note 2**: If you are running a very modern version of GCC (>= gcc-11), you may experience build issues on our dependencies. You can fix this issue by downgrading to gcc-10, or by waiting until the dependencies fix the errors. 

## Windows

Open the CMakeLists.txt using Visual Studio. You may need to copy the `launch.vs.json` to the `.vs` folder so
the debugger launches the executable on the correct working directory.

Set target to OSPGL.exe and build.

You may need to point CMake to the freetype libraries if they are not detected by default, or disable freetype and use 
stb_freetype, but it has lower text quality.

## MacOS

(Not done yet, I guess the linux procedure will work fine, too)

# Running

During development, you will need some packages which are "directly" linked to the game code.
The final game will only need the `core` package, but for now you must run these two commands to 
get the game to properly load:

(`ospm` must be run in the same folder as the `/res` folder)

```
./ospm fetch https://github.com/TheOpenSpaceProgram/new-ospgl/releases/download/ospm-test/debug_system.zip
./ospm fetch https://github.com/TheOpenSpaceProgram/new-ospgl/releases/download/ospm-test/test_parts.zip
```

If there are weird runtime errors, they may be related to outdated assets. First try to update the packages
running the same commands as before and overwriting. If nothing works, ping `@Tatjam` on the discord
and tell him to upload the latest files. 

# Packaging

`ospm` is used for managing packages, but as of now it's only capable of downloading packages from an URL using the command `fetch`. 

# Code Style
We don't have a very strict coding-style, but these rules must be followed:
- Brackets on the next line
- Single-line blocks, when appropiate, are fine: `if(a) { return; }`
- Avoid "bracketless" blocks, it's very easy to introduce a hard to find bug with them
- Snake case naming for functions and variables (`my_function`, `current_velocity`, `parse_json`)
    - Feel free to use contractions for common stuff, such as `pos` for `position`, `nrm` for `normal`...
- Camel case naming for classes and file names (`MyClass`, `JSONFile`)
- Use `nullptr` instead of `NULL`
- There is no need to mark member variables with anything (`m_name`, ...), unless it's absolutely neccesary
- Using `auto` is recommended for long types such as iterators, but should not be abused as some editors may not be able to resolve them, making code confusing.
