import time
import cv2
import numpy as np
from swarm import Swarm

try:
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d import Axes3D  # Import für 3D-Plot
except ImportError:
    import os
    os.system('pip install matplotlib')
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d import Axes3D  # Import für 3D-Plot


class Square:
    def __init__(self, x, y, side_length):
        self.x = x  
        self.y = y  
        self.side_length = side_length  

    def contains_point(self, px, py):
        return self.x <= px <= self.x + self.side_length and self.y <= py <= self.y + self.side_length

def inspect_depth_image(depth_image, pixel_coords, drone_id):
    for coord in pixel_coords:
        pixel_value = depth_image[coord]

    min_value = np.min(depth_image)
    max_value = np.max(depth_image)

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.hist(depth_image.flatten(), bins=100, color='gray')
    ax.set_title(f'Drohne {drone_id} - Histogramm der Tiefenwerte')
    plt.show()

    plt.figure(figsize=(8, 8))
    plt.imshow(depth_image, cmap='gray')
    plt.colorbar(label='Tiefe')
    plt.title(f'Drohne {drone_id} - Tiefenbild')

    for coord in pixel_coords:
        plt.plot(coord[1], coord[0], 'ro')

    plt.axis('off')
    plt.show()

def scatterplot(img, ax):
    img_height, img_width = img.shape

    z = 3000 - img
    z = z / 100
    x = np.arange(img_width)
    y = np.arange(img_height)
    x, y = np.meshgrid(x, y)

    x = x.flatten()
    y = y.flatten()
    z = z.flatten()

    # Filter out points with z <= 0
    mask = z > 0
    x = x[mask]
    y = y[mask]
    z = z[mask]

    scatter = ax.scatter(x, y, z, c=z, cmap='viridis')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z Depthvalue')

    return scatter

def add_zero_plane(ax, img):
    img_height, img_width = img.shape

    x = np.arange(img_width)
    y = np.arange(img_height)
    x, y = np.meshgrid(x, y)
    z = np.zeros_like(x)

    ax.plot_surface(x, y, z, color='darkgreen', alpha=0.5)

def shortestDistance(position, samplings):
    position = np.array(position)
    samplings = np.array(samplings)
    
    distances = np.linalg.norm(samplings - position, axis=1)
    
    min_index = np.argmin(distances)
    
    return min_index

def plot_nearest_image(point, sample_positions, imgs):
    # Find the index of the nearest sampling position
    nearest_index = shortestDistance(point, sample_positions)

    # Create a figure for the scatterplot of the nearest depth image
    fig_nearest, ax_nearest = plt.subplots(subplot_kw={'projection': '3d'})
    scatterplot(imgs[nearest_index], ax_nearest)
    add_zero_plane(ax_nearest, imgs[nearest_index])
    ax_nearest.set_title(f'Nearest Sampling Position Scatterplot (Point {point})')
    plt.show()

camera_params = {
    'fov': 50, 
    'image_size': (512, 512),  
    'clip': {
        'near': 1,  
        'far': 250  
    }
}

def calculate_world_coordinates(drone_pos, image_radius, img_x, img_y):
    pos_x = drone_pos[0] + (img_x-256)/512 * image_radius * 2
    pos_y = drone_pos[1] + (img_y-256)/512 * image_radius * 2
    return (pos_x, pos_y)


if __name__ == "__main__":
    swarm = Swarm("example_swarm")
    ids = swarm.spawn(4)
    sample_positions = np.array([
        [0, 0, 30.0],
        [5,0,30],
        [0,5,30],
        [5,5,30],
    ])
    groundPoint = [0,0,0]
    
    swarm.waypoints(ids, sample_positions)

    time_delta = 0.01
    time_passed = 0.0
    timeout = 5.0

    vectorized_images = []
    visibility_values = []
    world_coordinates_list = []
    image_radius = []
    fov_radians = np.deg2rad(camera_params['fov'])
    img_width = camera_params['image_size'][0]
    img_height = camera_params['image_size'][1]
    
    skyImage = np.zeros((img_width, img_height), int)

    #[ID, Min/Max]
    MinX = [0,float('inf')]
    MaxX = [0,float('-inf')]
    MinY = [0,float('inf')]
    MaxY = [0,float('-inf')]
    

    while time_passed < timeout:
        if swarm.received_frames[ids[-1]]:
            for id in ids:
                depth_image = swarm.depth_images[id]
                x = sample_positions[id][0]
                y = sample_positions[id][1]
                #Determine the Minimum X,Y and Maximum X,Y of the sampling positions
                if (x < MinX[1]):
                    MinX = [id, x]
                if (x > MaxX[1]):
                    MaxX = [id, x]
                if (y < MinY[1]):
                    MinY = [id, y]
                if (y > MaxY[1]):
                    MaxY = [id, y]
                
                vectorized_images.append(depth_image.flatten())
                image_radius.append(sample_positions[id][2] * np.tan(fov_radians/2))
                print(sample_positions[id][2] * np.tan(fov_radians/2), "\n")
                visibility_values.append(sample_positions[id][2] * 100)
            break
        time.sleep(time_delta)
        time_passed += time_delta

    if time_passed >= timeout:
        print("Timeout")
    
    
    sky_square = Square(groundPoint[0] - image_radius[0], groundPoint[1] - image_radius[0], 2*image_radius[0])
    
    MinX[1] = calculate_world_coordinates(sample_positions[MinX[0]], image_radius[MinX[0]], 0,0)[0]
    MaxX[1] = calculate_world_coordinates(sample_positions[MaxX[0]], image_radius[MaxX[0]], img_width,img_height)[0]
    MinY[1] = calculate_world_coordinates(sample_positions[MinY[0]], image_radius[MinY[0]], 0,0)[1]
    MaxY[1] = calculate_world_coordinates(sample_positions[MaxY[0]], image_radius[MaxY[0]], img_width, img_height)[1]

    #Calculate how much the max and min images intersects and determine the needed Array-Width
    p1 = MinX[1]
    p2 = calculate_world_coordinates(sample_positions[MaxX[0]], image_radius[MaxX[0]], 0,0)[0]
    p3 = calculate_world_coordinates(sample_positions[MinX[0]], image_radius[MinX[0]], img_width,img_height)[0]
    t = (p3-p2)/(p3-p1)
    arr_width = int(np.ceil(img_width + img_width - t * img_height))
    
    #Calculate how much the max and min images intersects and determine the needed Array-Height
    p1 = MinY[1]
    p2 = calculate_world_coordinates(sample_positions[MaxY[0]], image_radius[MaxY[0]], 0,0)[1]
    p3 = calculate_world_coordinates(sample_positions[MinY[0]], image_radius[MinY[0]], img_width,img_height)[1]
    t = (p3-p2)/(p3-p1)
    arr_height = int(np.ceil(img_height + img_height - t * img_height)) 

    visibility_array = np.zeros((int(arr_width), (int(arr_height))), int)
    visibility_matrix = np.array(vectorized_images)

    for id in ids:
        for i in range(img_width):
            for j in range(img_height):
                if swarm.depth_images[id][i][j] == visibility_values[id]:
                    world_x, world_y = calculate_world_coordinates(sample_positions[id], image_radius[id], j, i)
                    world_x_idx = int(((world_x - MinX[1]) / (MaxX[1] - MinX[1])) * arr_width)
                    world_y_idx = int(((world_y - MinY[1]) / (MaxY[1] - MinY[1])) * arr_height)
                    if 0 <= world_x_idx < arr_width and 0 <= world_y_idx < arr_height:
                        visibility_array[world_x_idx][world_y_idx] += 1

    
    plot_nearest_image([4,4,30], sample_positions, swarm.depth_images)

    num_drones = len(ids)
    num_cols = 2
    num_rows = (num_drones + num_cols - 1) // num_cols + 1  # Add one more row for the visibility array

    fig, axes = plt.subplots(num_rows, num_cols, figsize=(20, 10))

    # Plot the visibility array in the first row
    ax_vis = axes[0, 0]
    im_vis = ax_vis.imshow(visibility_array, cmap='hot', interpolation='nearest')
    ax_vis.set_title('Sichtbarkeitsarray')
    ax_vis.set_xlabel('X')
    ax_vis.set_ylabel('Y')
    fig.colorbar(im_vis, ax=ax_vis, label='Anzahl der Sichtbarkeiten')

    # Hide the second subplot in the first row if it exists
    if num_cols > 1:
        axes[0, 1].axis('off')

    # Plot scatterplots with the additional zero plane
    for i, id in enumerate(ids):
        row = (i + num_cols) // num_cols
        col = (i + num_cols) % num_cols
        ax = fig.add_subplot(num_rows, num_cols, row * num_cols + col + 1, projection='3d')
        scatterplot(swarm.depth_images[id], ax)
        add_zero_plane(ax, swarm.depth_images[id])
        ax.set_title(f'Drohne {id} - 3D Scatterplot with Zero Plane')
        fig.colorbar(ax.collections[0], ax=ax, label='Depthvalue')

    plt.tight_layout()
    plt.show()
