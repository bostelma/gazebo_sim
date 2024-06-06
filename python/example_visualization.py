import time
import cv2
import numpy as np
from swarm import Swarm

"""
try:
    import matplotlib.pyplot as plt
except ImportError:
    import os
    os.system('pip install matplotlib')
    import matplotlib.pyplot as plt

def inspect_depth_image(depth_image, pixel_coords, drone_id):
    for coord in pixel_coords:
        pixel_value = depth_image[coord]
        print(f"Drohne {drone_id} - Werte des Pixels bei {coord}: {pixel_value}")

    min_value = np.min(depth_image)
    max_value = np.max(depth_image)
    print(f"Drohne {drone_id} - Minimale Tiefe: {min_value}, Maximale Tiefe: {max_value}")

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.hist(depth_image[:, :, 0].flatten(), bins=100, color='gray')
    ax.set_title(f'Drohne {drone_id} - Histogramm der Tiefenwerte')
    plt.show()

    plt.figure(figsize=(8, 8))
    plt.imshow(depth_image[:, :, 0], cmap='gray')
    plt.colorbar(label='Tiefe')
    plt.title(f'Drohne {drone_id} - Tiefenbild')

    for coord in pixel_coords:
        plt.plot(coord[1], coord[0], 'ro')

    plt.axis('off')
    plt.show()
    """

if __name__ == "__main__":
    swarm = Swarm("example_swarm")
    ids = swarm.spawn(4)
    waypoints = np.array([
        [-2.0, -2.0, 30.0],
        [2.0, -2.0, 30.0],
        [-2.0, 2.0, 30.0],
        [2.0, 2.0, 30.0],
    ])

    swarm.waypoints(ids, waypoints)

    time_delta = 0.01
    time_passed = 0.0
    timeout = 5.0

    vectorized_images = []

    while time_passed < timeout:
        if swarm.received_frames[ids[-1]]:
            for id in ids:
                depth_image = swarm.depth_images[id]
                vectorized_images.append(depth_image.flatten())
            break
        time.sleep(time_delta)
        time_passed += time_delta

    if time_passed >= timeout:
        print("Timeout")
    visibility_matrix = np.array(vectorized_images).T
    print(visibility_matrix)
