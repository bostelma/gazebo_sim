## Prerequisites

- Python 3.8
- CMake >= 3.18
- Protoc 3.21.5
- opencv

## Install

1. Setup Python 3.8 environment and install protobuf using `
pip install protobuf==3.20.3`, dict2xml, and opencv.

2. Clone the repository including all submodules:
    ```bash
    git clone --recurse-submodules https://github.com/bostelma/gazebo_sim.git
    ```

3. Follow either the [windows](https://gazebosim.org/docs/garden/install_windows_src) or [ubuntu](https://gazebosim.org/docs/garden/install_ubuntu_src) tool installation. Don't get the sources, as they are part of the repository through submodules. Continue with the installation of the dependencies.

4. Export the previously created python interpreter:
    ```bash
    export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=[your python path]
    ```

5. Build gazebo from within the **workspace** directory, but set **your** python interpreter:
    ```bash
    colcon build --symlink-install --merge-install --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPython3_EXECUTBALE=[path to your python] -DGZ_PYTHON_VERSION=3.8 --packages-ignore gz-python
    ```
6. Make gazebo available for gz-python via '. ~gazebo_sim/workspace/install/setup.bash'

7. Prepare gz-python build from workspace directory and make sure that it finds your choosen python interpreter!
    ```bash
    mkdir -p src/gz-python/build
    cd src/gz-python/build
    cmake ..
    ```

8. In _deps/pybind11_protobuf-src/CMakeLists.txt, replace `SHARED` with `STATIC`

9. Build gz-python using `make`

10. Build each plugin individually:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

## Usage

Mak Gazebo available within a terminal:
```bash
. workspace/install/setup.bash
```

Export the resource path
```bash
export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/gazebo_sim/models
```

Export each plugin library file:
```bash
export GZ_SIM_SYSTEM_PLUGIN_PATH=$GZ_SIM_SYSTEM_PLUGIN_PATH:~/gazebo_sim/plugins/[plugin]/build/lib
```

Add the three python directories `~/gazebo_sim/workspace/src/gz-python/build/python`, `~/gazebo_sim/workspace/install/lib/python`, and `~gazebo_sim/python` to your python path or conda.pth file via `export PYTHONPATH=${PYTHONPATH}:[path]` or `conda develop [path]`, respectively.

## Plugins

### Photo Shoot

Do a photoshoot with: `gz sim -s -r --iterations 2 worlds/example_photo_shoot.sdf`

##  TODOs

- Include gz-python in colcon build
- Add global build for plugins
- Build additional messages separately (https://github.com/gazebosim/gz-sim/blob/fbc3ca84b86cc3c0033ae3979797340b7bf1b361/src/msgs/CMakeLists.txt)

## Issues

- The computer freezes or the process crashes during build due to overfull memory
    - Limit number of cores: `export MAKEFLAGS="-j [n_cores]"`
    - Add executor to colcon: `--executor sequential`

- gz/common/Image.hh does not exist
    - Just delete build, install, and log and do it again

- cmake for sdformat fails due to Module.Development not found
    Add Interpreter as required compnent in sdformat/CMakeLists.txt:148 `find_packate(Python3 ${GZ_PYTHON_VERSION} REQUIRED COMPONENTS Interpreter Development)`

## BUGS
- one tree with two species, 0.5 each produces error
- crash on reset:   [Err] [SceneManager.cc:223] Visual: [Procedural Forest] already exists
                    terminate called after throwing an instance of 'std::bad_optional_access'
- Tree height seems to be smaller in comparison to original implementation




