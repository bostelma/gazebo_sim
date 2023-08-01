## Prerequisites

- Python 3.8
- CMake >= 3.18
- Protoc 3.21.5 

## Install

1. Setup Python 3.8 environment and install protobuf using `
pip install protobuf==3.20.3` and opencv.

2. Clone the repository including all submodules:
    ```bash
    git clone -recurse-submodules https://github.com/bostelma/gazebo_sim.git
    ```

3. Follow either the [windows](https://gazebosim.org/docs/garden/install_windows_src) or [ubuntu](https://gazebosim.org/docs/garden/install_ubuntu_src) tool installation. Don't get the sources, as they are part of the repository through submodules. Continue with the installation of the dependencies.

4. Export the previously created python interpreter:
    ```bash
    export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=[your python path]
    ```

5. Build gazebo from within the workspace directory:
    ```bash
    colcon build --symlink-install --merge-install --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo --packages-ignore gz-python
    ```

6. Prepare gz-python build from workspace directory and make sure that it finds your choosen python interpreter!
    ```bash
    mkdir -p src/gz-python/build
    cd src/gz-python/build
    cmake ..
    ```

7. In _deps/pybind11_protobuf-src/CMakeLists.txt, replace `SHARED` with `STATIC`

8. Build gz-python using `make`

9. Build each plugin individually:
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

Add the two python directories `~/gazebo_sim/workspace/src/gz-python/build/python` and `~gazebo_sim/python` to your python path or conda.pth file via `export PYTHONPATH=${PYTHONPATH}:[path]` or `conda develop [path]`, respectively.

##  TODOs

- Include gz-python in colcon build
- Add global build for plugins

## Issues

- The computer freezes or the process crashes during build due to overfull memory
    - Limit number of cores: `export MAKEFLAGS="-j [n_cores]"`
    - Add executor to colcon: `--executor sequential`

## BUGS
- one tree with two species, 0.5 each produces error
- crash on reset:   [Err] [SceneManager.cc:223] Visual: [Procedural Forest] already exists
                    terminate called after throwing an instance of 'std::bad_optional_access'
- Tree height seems to be smaller in comparison to original implementation




