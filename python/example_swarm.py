import time
import cv2

import numpy as np

from swarm import Swarm

if __name__ == "__main__":

    # Create the swarm object by passing the name
    # of the world from the .sdf world file.
    swarm = Swarm("example_swarm")

    # Spawn 4 drones and keep the returning ids as handles
    ids = swarm.spawn(4)

    # First waypoints
    waypoints = np.array([
        [-2.0,-2.0, 30.0],
        [ 2.0,-2.0, 30.0],
        [-2.0, 2.0, 30.0],
        [ 2.0, 2.0, 30.0],
    ])

    for d_z in range(0, 30, 5):

        # Set the waypoints for all drones
        current_waypoints = waypoints - np.array([0.0, 0.0, d_z])
        swarm.waypoints(ids, current_waypoints)

        # Wait until the data has arrived
        time_delta = 0.01           # Delta time per sleep command in seconds
        time_passed = 0.0           # Time counter to keep track of the time in seconds
        timeout = 1.0               # Timeout in case something goes wrong

        timeout_occured = False
        
        while time_passed < timeout:

            # All frames for a waypoint called send
            # together, so it is enough to check the
            # last one.
            if swarm.received_frames[ids[-1]]:

                for id in ids:
                    rgb_image = swarm.rgb_images[id]
                    thermal_image = swarm.thermal_images[id]
                    depth_image = swarm.depth_images[id]
                    # In this example we only store them in a data directory
                    # OpenCV expects BGR but it is RGB, so switch the channels
                    cv2.imwrite(f"../../data/swarm/rgb_image_drone_{id}_dz_{d_z}.png", cv2.cvtColor(rgb_image, cv2.COLOR_BGR2RGB))    
                    cv2.imwrite(f"../../data/swarm/thermal_image_drone_{id}_dz_{d_z}.png", thermal_image)
                    cv2.imwrite(f"../../data/swarm/depth_image_drone_{id}_dz_{d_z}.png", depth_image)
                break
            
            time.sleep(time_delta)
            time_passed += time_delta
        if time_passed >= timeout:
                timeout_occured = True
                print(f"Timeout reached for waypoints with dz={d_z}")
        
    if timeout_occured:
        raise TimeoutError("Timeout occured while waiting for waypoint")
