import time
import random

import numpy as np

from person import Person

if __name__ == "__main__":

    # Create the person object by passing the name
    # of the world from the .sdf world file. Make
    # sure that the world is currently running:
    # gz sim ~/gazebo_sim/worlds/example_swarm.sdf -r
    person = Person("example_swarm")
    
    # Create a set of waypoints to visit
    waypoint_index = 0
    waypoints = np.array([
        [ 3.0, -3.0, 0.0],
        [ 3.0,  3.0, 0.0],
        [-3.0,  3.0, 0.0],
        [5.0, -5.0, 0.0]
    ])
    ids = person.spawn(4)

    # Set the time per waypoint in seconds
    delay = 1.0    

    # Send the waypoints repeatedly
    try:

        while True:

            waypoints = np.roll(waypoints, 1, axis=0)

            # Apply a random rotation around the z axis
            angle = random.random() * 2 * np.pi
            # Actually send the waypoint
            #person.waypoints(waypoints[waypoint_index]
            person.waypoints(ids, waypoints)
            
            
            time.sleep(delay)

    except KeyboardInterrupt:
        pass
