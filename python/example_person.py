import time

import numpy as np

from person import Person

if __name__ == "__main__":

    # Create the person object by passing the name
    # of the world from the .sdf world file. Make
    # sure that the world is currently running:
    # gz sim ~/gazebo_sim/worlds/example_swarm.sdf -r
    person = Person("example_person")
    
    # Create a set of waypoints to visit
    waypoints = np.array([
        [ 3.0, -3.0, 0.0],
        [ 3.0,  3.0, 0.0],
        [-3.0,  3.0, 0.0],
        [-3.0, -3.0, 0.0]
    ])
    waypoint_indices = np.arange(len(waypoints))

    # Spawn the people at the initial waypoints
    ids = person.spawn(4, positions=waypoints)

    # Set the time per waypoint in seconds
    delay = 0.1   

    # Send the waypoints repeatedly
    try:

        while True:

            # Increase the waypoint indices
            waypoint_indices = (waypoint_indices + 0.1) % len(waypoints)

            # Create the numpy arrays for the waypoints and orientations
            final_waypoints = np.full_like(waypoints, 0.0)
            final_orientations = np.repeat(np.array([[0.0, 0.0, 0.0, 1.0]]), len(waypoints), axis=0)

            # Compute each final waypoint and orientation
            for i, id in enumerate(ids):

                start = waypoints[int(waypoint_indices[i])]
                end = waypoints[(int(waypoint_indices[i]) + 1) % len(waypoints)]
                direction = end - start

                final_waypoints[i] = start + direction * (waypoint_indices[i] % 1)

                # Adjust for different coordinate systems
                angle = np.arctan2(-direction[1], direction[0])
                
                final_orientations[id] = np.array([0.0, 0.0, np.sin(angle/2), np.cos(angle/2)])

            person.waypoints(ids, final_waypoints, final_orientations)
            time.sleep(delay)

    except KeyboardInterrupt:
        pass
