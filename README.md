# Installation

## Apptainer

On any Linux distro that supports Apptainer, but preferably Ubuntu 22.04, it is recommended to use the Apptainer installation option.

1. Clone the simulator without the submodules that contain the gazebo source code.
    ```
    git clone https://github.com/bostelma/gazebo_sim.git
    ```

2. [Install Apptainer](https://apptainer.org/docs/admin/main/installation.html) using the official instructions.

3. Build the Apptainer image from the definition file. There are two versions, one without python bindings, called `ubuntu-gazebo_sim-basic.def` and one with python bindings, called `ubuntu-gazebo_sim-full.def`. The latter uses a python 3.7.9 conda environment, while the first one uses the default python 3.10 interpreter of ubuntu. As the images are read-only, the path to the mutable files on the host system has to be passed to the build command. When using Windows and WSL, use the corresponding windows versions.
    ```
    cd ~/gazebo_sim/apptainer
    apptainer build --build-arg GZ_PATH=~/gazebo_sim gazebo_sim-full.sif ubuntu-gazebo_sim-full.def
    ```

4. Build the plugins using the provided script `build_plugins_basic.sh` or `build_plugins_full.sh`, depending on whether you have the basic or full version:
    ```
    apptainer shell gazebo_sim-full.sif
    cd ../plugins
    sh build_plugins_full.sh
    exit
    ```

## Windows

On Windows, the simulation can can be used in the same way as on Linux/Ubuntu by using WSL 2.
1. Install/Activate [WSL2](https://learn.microsoft.com/de-de/windows/wsl/install) and make sure it is the newest version by updating first via
   ```
   wsl --update
   ```

2. Install a new Ubuntu 22.04 distro using
   ```
   wsl --install Ubuntu-22.04
   ```

3. Start the new installed distro by calling
   ```
   wsl -d Ubuntu-22.04
   ```

4. From here follow either of the insall options, but preferably the apptainer version. On request, a prebuild and exported WSL distro can be provided to you.

## Source Install 

The source install option is currently only tested and supppoorted for Ubuntu 22.04.

### Basic Version

1. Update Ubuntu to make all packages available:
    ```
    sudo apt-get update
    sudo apt-get upgrade
    ```

2. Clone the simulator, which already contains the official Gazebo source code via git submodules, into the home directory:
    ```
    cd
    git clone --recurse-submodules https://github.com/bostelma/gazebo_sim.git
    ```

3. Start with the official [Gazebo Garden Installation on Ubuntu](https://gazebosim.org/docs/garden/install_ubuntu_src) to install the tools and dependencies. Getting the sources is not required and stop before the build process.


4. Install additional dependencies:

    ```
    sudo apt install libopencv-dev python3-opencv
	pip install dict2xml
    ```

5. Install an updated version of protoc:
    ```
    sudo apt autoremove protobuf-compiler libprotobuf-dev libprotobuf-lite23
	mkdir proto_ws && cd proto_ws
	git clone https://github.com/google/protobuf.git
	cd protobuf
	git checkout tags/v21.5
	git submodule init
	git submodule update
	mkdir build && cd build
	cmake .. -DCMAKE_CXX_FLAGS="-fPIC"
	make -j4
	sudo make install
    ```

6. Limit the number of cores for building to avoid crashes due to not enough memory:
    ```
    export MAKEFLAGS="-j4"
    ```

7. Build Gazebo itself using colcon:
    ```
    cd ~/gazebo_sim/workspace
    colcon build --symlink-install --merge-install --cmake-args -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo --packages-ignore gz-python --executor sequential
    ```

8. Source the Gazebo workspace to make it available in the current prompt:
    ```
    . ~/gazebo_sim/workspace/install/setup.bash
    ```

9. Build the basic plugins: Forest, Person, and Photo Shoot individually as seen below, or by executing the script `build_plugins_basic.sh` in the plugins directory.
    ```
    cd ~/gazebo_sim/plugins/forest  # change to 'person' and 'photo_shoot' respectively
	mkdir build
	cd build
	cmake .. -DCMAKE_INSTALL_PREFIX=~/gazebo_sim/plugins/install
	make
    make install
    ```

10. Setup all paths to make everything available in the current prompt:
    ```
	export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/gazebo_sim/models
	export GZ_SIM_SYSTEM_PLUGIN_PATH=$GZ_SIM_SYSTEM_PLUGIN_PATH:~/gazebo_sim/plugins/install/lib
	export PYTHONPATH=${PYTHONPATH}:~/gazebo_sim/python
	export PYTHONPATH=${PYTHONPATH}:~/gazebo_sim/workspace/install/lib/python
    ```

### Full Version

This component adds python binding to msgs and transport and is required to use the swarm functionality.

1. Update Ubuntu to make all packages available:
    ```
    sudo apt-get update
    sudo apt-get upgrade
    ```

2. Setup a python 3.7 environment, for example using conda and install protobuf==3.20.3, opencv=4.5.5.64, libstdcxx-ng, and conda-build
    ```
    cd
    wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
    bash Miniconda3-latest-Linux-x86_64.sh
    ```
    Restart the shell by closing and reopening it.
    ```
    conda create -n gz-ws python=3.7.9
    conda activate gz-ws
    conda install protobuf=3.20.3
    conda install conda-build libstdcxx-ng
    pip install opencv-python==4.5.5.64
    ```

3. Follow steps 2 through 5 of the ubuntu install instructions and install the correct libogre version if you are using windows.

4. Export your selected python interpreter (`which python` shows path):
    ```
    export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=[your python path]
    ```

5. Modify the sdformat13 CMakeLists.txt to fix a bug: Replace `find_package(Python3 ${GZ_PYTHON_VERSION} REQUIRED COMPONENTS Development)` with `find_package(Python3 ${GZ_PYTHON_VERSION} REQUIRED COMPONENTS Interpreter Development)` in `~/gazebo_sim/workspace/src/sdformat/CMakeLists.txt`, line 148.
   

6. Add your python path and version to the build command:
    ```
    cd ~/gazebo_sim/workspace
    export MAKEFLAGS="-j4"
    colcon build --symlink-install --merge-install --cmake-args -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPython3_EXECUTABLE=[path to your python] -DGZ_PYTHON_VERSION=3.7 --packages-ignore gz-python --executor sequential
    ```

7. Source the Gazebo workspace to make it available in the current prompt:
    ```
    . ~/gazebo_sim/workspace/install/setup.bash
    ```

8. Prepare gz-python build from workspace directory and make sure that it finds your chosen python interpreter!
    ```
    mkdir -p ~/gazebo_sim/workspace/src/gz-python/build
    cd src/gz-python/build
    cmake ..
    ```

9. In _deps/pybind11_protobuf-src/CMakeLists.txt, replace `SHARED` with `STATIC`

10. Build gz-python using `make`

11. Build the plugins as in step 9 of the ubuntu install instructions, but this time build the swarm plugin as well by calling `build_plugins_full.sh`.

12. Setup all paths to make everything available in the current prompt. If you don't use conda, replace the conda develop command with a respective export command like in step 10 of the ubuntu install instructions.
    ```
	export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:~/gazebo_sim/models
	export GZ_SIM_SYSTEM_PLUGIN_PATH=$GZ_SIM_SYSTEM_PLUGIN_PATH:~/gazebo_sim/plugins/install/lib
	conda develop ~/gazebo_sim/python
    conda develop ~/gazebo_sim/workspace/install/lib/python
    conda develop ~/gazebo_sim/workspace/src/gz-python/build/python
    ```

## Plugins

### Photo Shoot

Do a photo shoot with: `gz sim -s -r --iterations 2 worlds/example_photo_shoot.sdf`

## Issues

- The computer freezes or the process crashes during build due to overfull memory
    - Limit number of cores: `export MAKEFLAGS="-j [n_cores]"`
    - Add executor to colcon: `--executor sequential`

- gz/common/Image.hh does not exist
    - Just delete build, install, and log and do it again

- cmake for sdformat fails due to Module.Development not found
    Add Interpreter as required component in sdformat/CMakeLists.txt:148 `find_package(Python3 ${GZ_PYTHON_VERSION} REQUIRED COMPONENTS Interpreter Development)`

- The light source itself gets rendered in the form of a square:

    ![Image of rendered light source](resources/rendered_light_source.png)

    This behavior can be disabled by adding the tag `<visualize>false</visualize>` within the light tag in the respective world file, or by calling `light.set_visualize(False)` in the respective Python file.

- Memory issues when installing Gazebo on Windows and WSL using Apptainer:
    - Increase the allowed ram by adding a file `.wslconfig` in the home directory of your windows user with the following content:
        ```
        [wsl2]
        memory=14GB
        ```

## BUGS
- one tree with two species, 0.5 each produces error
- crash on reset:   [Err] [SceneManager.cc:223] Visual: [Procedural Forest] already exists
                    terminate called after throwing an instance of 'std::bad_optional_access'




