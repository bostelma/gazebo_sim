# Project Documentation

## Introduction
The project is built using [Gazebo](https://gazebosim.org/home)
, a platform for robotics and autonomous systems simulation.
It allows you to open SDF worlds (Simulation Description Format) and to interact with the simulation through external Python scripts.
This allows the integration of external components and communication with the simulated environment

## System Architecture
The system is composed of several main components:
- **Gazebo**: The simulation environment where the SDF worlds are loaded.
- **Plugins**: Custom extensions that add or modify specific functionality in the simulation
- **Python Scripts**: Allows interaction with Gazebo topics and control over the simulation while it is running

## Apptainer

The general idea is that you work on the project files on the host system while using the environment that contains gazebo via apptainer.

1. Start an named instance of your created image.
    ```
    apptainer instance start ~/gazebo_sim/apptainer/gazebo_sim-full.sif gazebo
    ```

2. Start a shell within the container.
    ```
    apptainer shell instance://gazebo
    ```

3. Stop an instance once you are finished.
    ```
    apptainer instance stop gazebo
    ```

To be able to work on notebooks while using the container environment, the container feature a jupyter server that is running when starting an instance as described above. To get the address and the required token, execute `jupyter notebook list` from within the container. If you want to use an password instead, you need to add it to the build command: `--build-arg JUPYTER_PASSWORD=my_secure_password`. Make sure to use a secure password as it prevents others from executing code on your machine!

## Interacting with Gazebo

You can start the simulation with the following command:

```bash
gz sim ~/gazebo_sim/worlds/example_swarm.sdf -r -s
```
(Note that in this case "example_swarm.sdf" is loaded. To load a different world replace it with "your_world.sdf")

`gz sim`: This command starts the Gazebo simulation

`~/gazebo_sim/worlds/example_swarm.sdf`: The path to the SDF file you want to load

`-r`: This flag instructs Gazebo to start running the simulation immediately. Without it, the simulation starts in a paused state

`-s`: This flag disables the GUI and will run the simulation in headless mode

## Running Python Scripts

While the simulation is running, you can execute Python scripts in another terminal to interact with the simulation.
this allows you to control different aspects of the simulation.
The python scripts are able to communicate with the simulation through requests.

For example: running `python3 example_swarm.py` from ~/gazebo_sim/python to spawn drones and let them generate images from the simulation

### Usage

### Photo Shoot

1. Make sure that everything is available by performing step 7 and 10 of the ubuntu install instructions before starting.

2. Create the data directory for the example script:
    ```
    mkdir -p ~/data/photo_shoot
    ```

3. Generate an example image:
    ```
    cd ~/gazebo_sim/python
    python3 example_photo_shoot.py
    ```

4. The resulting image is now stored at the previously created directory. Have a look at the used python file to generate custom images yourself.

You can also do a photo shoot with: 

`gz sim -s -r --iterations 2 worlds/example_photo_shoot.sdf`


### Swarm

1. Make sure that everything is available by performing step 7 and 10 of the ubuntu install instructions before starting, as well as the additional python path for gz-python.

2. Create the data directory for the example script:
    ```
    mkdir -p ~/data/swarm
    ```

3. Start and run the gazebo simulation first in headless mode:
    ```
    gz sim ~/gazebo_sim/worlds/example_swarm.sdf -r -s
    ```

4. Execute the example python file in another shell, but make sure all paths are set, the workspace is sourced, and the correct python environment is activated!
    ```
    cd ~/gazebo_sim/python
    python3 example_swarm.py 
    ```

5. The resulting images are now stored at the previously created directory. Have a look at the used python file to understand how it is done. The benefit of this method is, that multiple images of the same world can be generated without restarting the whole simulation. As the tree generation takes up most of the startup time, this is a significant time decrease.

## Python:Swarm

The `Swarm` class is responsible for managing a group of drones in the simulation. It provides methods for spawning drones, setting waypoints, and handling frames from the drones' sensors.

### Attributes
- `timeout`: The timeout for requests in milliseconds. Default is 1000 ms.
- `world_name`: The name of the Gazebo world where the swarm operates.
- `received_frames`: A dictionary with drone IDs as keys and booleans indicating whether frames have been received.
- `rgb_images`: A dictionary mapping drone IDs to their respective RGB images.
- `thermal_images`: A dictionary mapping drone IDs to their respective thermal images.

### Methods

- `__init__(self, world_name)`: Initializes the Swarm object. It sets the world name and constructs the topic names for the services and topics in Gazebo. It also subscribes to the frame topic to receive images from the drones.

- `spawn(self, count, model='drone', positions=None)`: Spawns a specified number of drones in the simulation.
  - **Parameters**:
    - `count`: The number of drones to spawn.
    - `model`: The model name of the drone (default is 'drone').
    - `positions`: An optional numpy array specifying the spawn positions for the drones. The array should have shape `(count, 3)`.
  - **Returns**: A numpy array containing the IDs of the spawned drones.
  - **Notes**: This method raises a `RuntimeError` if the spawn request fails or times out.

- `waypoints(self, ids, positions, orientation=np.array([0.0, 0.7071068, 0.0, 0.7071068]))`: Sends waypoints to the specified drones.
  - **Parameters**:
    - `ids`: A numpy array containing the IDs of the drones to which waypoints are sent.
    - `positions`: A numpy array with shape `(N, 3)` representing the waypoints for the drones.
    - `orientation`: A numpy array representing the orientation quaternion for the drones. Default is `[0.0, 0.7071068, 0.0, 0.7071068]`.
  - **Notes**: This method raises a `RuntimeError` if the waypoint request fails or times out.

- `_frame_call_back(self, msg: Frame_V)`: A private method handling incoming frames from the drones' sensors.
  - **Parameters**:
    - `msg`: A `Frame_V` object containing the sensor data.
  - **Description**: This method updates the `received_frames`, `rgb_images`, and `thermal_images` dictionaries based on the incoming frame data.


## Python:Person

The `Person` class is an interface for interacting with "persons" in the simulation. It provides methods to spawn virtual people in the simulation and send them to specified waypoints.

### Attributes

- `timeout`: Timeout for requests in milliseconds. Default is `1000`.
- `world_name`: The name of the world.

### Methods

#### `__init__(self, world_name)`

- Initializes the `Person` object with the name of the world.
- Constructs the topic names for interacting with Gazebo.
- Parameters:
  - `world_name`: The name of the world as specified in the SDF file.

#### `spawn(self, count, model='person', positions=None)`

- Spawns one or more persons in the simulation.
- Parameters:
  - `count`: Number of persons to spawn.
  - `model`: Name of the model to spawn. Default is `'person'`.
  - `positions`: Optional array of shape `(count, 3)` specifying initial positions for the persons.
- Returns:
  - `ids`: An array containing the IDs of the spawned persons.
- Raises:
  - `ValueError`: If the number of persons does not match the number of specified positions.
  - `RuntimeError`: If the request to spawn persons fails or times out.

#### `waypoints(self, ids, positions)`

- Sends waypoints to the specified persons.
- Parameters:
  - `ids`: Array of IDs indicating which persons should receive the waypoints.
  - `positions`: Array of shape `(len(ids), 3)` specifying the waypoints.
- Raises:
  - `ValueError`: If the number of IDs and positions don't match.
  - `RuntimeError`: If the request to send waypoints fails or times out.
- The method calculates the appropriate orientation for each person based on their target direction using quaternion arithmetic.


