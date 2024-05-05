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
        [-3.0, -3.0, 0.0]
    ])
    ids = person.spawn(4, positions=waypoints)

    # Set the time per waypoint in seconds
    delay = 1.0    

    # Send the waypoints repeatedly
    try:

        while True:

            waypoints = np.roll(waypoints, 1, axis=0)

            orientations = np.zeros((len(ids), 4))

            for idx in range(len(ids)):
                if len(waypoints) > 1:
                    direction = waypoints[(idx - 1) % len(waypoints)] - waypoints[idx]
                    direction /= np.linalg.norm(direction) 

                    angle = np.arccos(direction.dot([1, 0, 0]))

                    orientations[idx] = np.array([0.0, 0.0, np.sin(angle/2), np.cos(angle/2)])
                else:
                    orientations[idx] = np.array([0.0, 0.0, 0.0, 1.0])

            # Actually send the waypoint
            #person.waypoints(waypoints[waypoint_index]
            person.waypoints(ids, waypoints, orientations)
            
            
            time.sleep(delay)

    except KeyboardInterrupt:
        pass
