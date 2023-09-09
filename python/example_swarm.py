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

    # Set the first waypoints for all our drones
    swarm.waypoints(ids, np.array([
        [-2.0,-2.0, 30.0],
        [ 2.0,-2.0, 30.0],
        [-2.0, 2.0, 30.0],
        [ 2.0, 2.0, 30.0],
    ]))

    # Wait until the data has arrived
    time_delta = 0.01           # Delta time per sleep command in seconds
    time_passed = 0.0           # Time counter to keep track of the time in seconds
    timeout = 1.0               # Timeout in case something goes wrong

    while time_passed < timeout:

        # All frames for a waypoint called send
        # together, so it is enough to check one.
        if swarm.received_frames[ids[0]]:

            for id in ids:
                rgb_image = swarm.rgb_images[id]
                thermal_image = swarm.thermal_images[id]

                # In this example we only store them in a data directory
                # OpenCV expects BGR but it is RGB, so switch the channels
                cv2.imwrite(f"../../data/swarm/rgb_image_drone_{id}.png", cv2.cvtColor(rgb_image, cv2.COLOR_BGR2RGB))    
                cv2.imwrite(f"../../data/swarm/thermal_image_drone_{id}.png", thermal_image)

            break
        
        time.sleep(time_delta)
        time_passed += time_delta