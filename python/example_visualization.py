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

def parallel_world_coordinates(drone_pos, image_radius, img_y):
    pos_x = [i for i in range(512)]
    pos_x = (pos_x - np.asarray(256)) / 512 * image_radius * 2 + drone_pos[0]
    pos_y = drone_pos[1] + (img_y-256)/512 * image_radius * 2
    pos_y = np.asarray(pos_y)
    return (pos_x, pos_y)

def fill_missing_points(SkyImage):
    SkyImage = np.array(SkyImage)
    
    filled_array = np.copy(SkyImage)
    set_points = np.where(SkyImage != -1)
    set_values = SkyImage[set_points]
    
    for i in range(SkyImage.shape[0]):
        for j in range(SkyImage.shape[1]):
            if SkyImage[i, j] == -1:
                nearest_index = np.argmin(np.abs(set_points[0] - i) + np.abs(set_points[1] - j))
                filled_array[i, j] = set_values[nearest_index]
    
    return filled_array



if __name__ == "__main__":
    start_time = time.time()
    swarm = Swarm("example_swarm")
    drone_count = 4
    ids = swarm.spawn(drone_count)
    sample_positions = np.array([
        [-3, -2, 30.0],
        [-3, -1, 30.0],
        [-3, 1, 30.0],
        [-3, 2, 30.0],
    ])
    
    depthImages = []

    groundPoint = [0.5,0.5,0]
    
    time_delta = 0.1
    time_passed = 0.0
    timeout = 20.0

    sample_iterations = 6
    sample_distance = 1

    vectorized_images = []
    visibility_value = sample_positions[0][2] * 100
    world_coordinates_list = []
    fov_radians = np.deg2rad(camera_params['fov'])
    image_radius = sample_positions[0][2] * np.tan(fov_radians/2)
    img_width = camera_params['image_size'][0]
    img_height = camera_params['image_size'][1]
    
    skyImage = np.full((img_width, img_height), -1)

    #[ID, Min/Max]
    MinX = [0,float('inf')]
    MaxX = [0,float('-inf')]
    MinY = [0,float('inf')]
    MaxY = [0,float('-inf')]
    
    
    for i in range(sample_iterations):
        shifts = np.array([[i * sample_distance, 0, 0]] * drone_count)
        cur_sample_positions = sample_positions + shifts
        
        swarm.waypoints(ids, cur_sample_positions)
        while time_passed < timeout:
            if swarm.received_frames[ids[-1]]:
                for id in ids:
                    depthImages.append(swarm.depth_images[id])
                break
            time.sleep(time_delta)
            time_passed += time_delta
    end_time = time.time()
    elapsed_time = end_time - start_time
    print("Elapsed_time after sampling: ", elapsed_time)        
    if time_passed >= timeout:
        print("Timeout")
    for id in range(len(ids) * sample_iterations):
        depth_image = depthImages[id]
        x = sample_positions[id % drone_count][0] + i * sample_distance * np.floor(id / drone_count)
        y = sample_positions[id % drone_count][1] + i * sample_distance * np.floor(id / drone_count)

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

    calculate_coordinates = lambda m, w, h: calculate_world_coordinates(
        sample_positions[m % drone_count] + np.array([np.floor(m / drone_count) * sample_distance,0,0]),
        image_radius,
        w,
        h
    )

    MinX[1] = calculate_coordinates(MinX[0], 0, 0)[0]
    MaxX[1] = calculate_coordinates(MaxX[0], img_width, img_height)[0]
    MinY[1] = calculate_coordinates(MinY[0], 0, 0)[1]
    MaxY[1] = calculate_coordinates(MaxY[0], img_width, img_height)[1]

    #Calculate how much the max and min images intersects and determine the needed Array-Width
    p1 = MinX[1]
    p2 = calculate_coordinates(MaxX[0], 0, 0)[0]
    p3 = calculate_coordinates(MinX[0], img_width, img_height)[0]

    t = (p3-p2)/(p3-p1)
    arr_width = int(np.ceil(img_width + img_width - t * img_height))
    
    #Calculate how much the max and min images intersects and determine the needed Array-Height
    p1 = MinY[1]
    p2 = calculate_coordinates(MaxY[0], 0, 0)[1]
    p3 = calculate_coordinates(MaxY[0], img_width, img_height)[1]

    t = (p3-p2)/(p3-p1)
    arr_height = int(np.ceil(img_height + img_height - t * img_height)) 

    visibility_array = np.zeros((int(arr_width), (int(arr_height))), int)
    visibility_matrix = np.array(vectorized_images)
    distances = []

    for id in range(len(ids) * sample_iterations):
        for i in range(img_width):
            #Vectorised calculation
            world_x, world_y = parallel_world_coordinates(sample_positions[id%drone_count] + np.array([np.floor(id/drone_count) * sample_distance, 0, 0]),image_radius, i)
            img_rad = np.asarray(2 *image_radius / img_width)
            distances = abs(world_x - np.asarray(groundPoint[0]))
            insideImage = distances < img_rad
            if insideImage.any() and abs(world_y - groundPoint[1]) < img_rad:
                j = np.where(insideImage)[0][0]
                dx = 256 - i
                dy = 256 - j
                skyImage[256 + dx][256 + dy] = abs(0-depthImages[id][i][j])

            visTest = np.where(depthImages[id][i] == visibility_value)[0]
            visTest2 = len(visTest)
            for j in range(visTest2):
                world_x_idx = int(((world_x[visTest[j]] - MinX[1]) / (MaxX[1] - MinX[1])) * arr_width)
                world_y_idx = int(((world_y - MinY[1]) / (MaxY[1] - MinY[1])) * arr_height)
                visibility_array[world_x_idx][world_y_idx] += 1


    #for id in range(len(ids) * sample_iterations):
     #   for i in range(img_width):
      #      for j in range(img_height):
       #         world_x, world_y = calculate_coordinates(id, j, i)
        #        if depthImages[id][i][j] == visibility_value:
         #           world_x_idx = int(((world_x - MinX[1]) / (MaxX[1] - MinX[1])) * arr_width)
          #          world_y_idx = int(((world_y - MinY[1]) / (MaxY[1] - MinY[1])) * arr_height)
           #         if 0 <= world_x_idx < arr_width and 0 <= world_y_idx < arr_height:
            #            visibility_array[world_x_idx][world_y_idx] += 1

    skyImage = fill_missing_points(skyImage)

    end_time = time.time()
    elapsed_time = end_time - start_time
    print("Elapsed_time: ", elapsed_time)

    plt.figure(figsize=(10, 8))
    plt.imshow(skyImage, cmap='bone', interpolation='nearest')
    plt.colorbar(label='Number of Detections')
    plt.title('SkyImage Heatmap')
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.show()
    # Plot only two subplots: visibility array and scatterplot for drone 0
    fig, axes = plt.subplots(1, 2, figsize=(20, 10))

    # Plot the visibility array
    ax_vis = axes[0]
    im_vis = ax_vis.imshow(visibility_array, cmap='hot', interpolation='nearest')
    ax_vis.set_title('Sichtbarkeitsarray')
    ax_vis.set_xlabel('X')
    ax_vis.set_ylabel('Y')
    fig.colorbar(im_vis, ax=ax_vis, label='Anzahl der Sichtbarkeiten')

    # Plot scatterplot with the additional zero plane for drone 0
    ax_scatter = fig.add_subplot(122, projection='3d')
    scatterplot(depthImages[0], ax_scatter)
    add_zero_plane(ax_scatter, swarm.depth_images[0])
    ax_scatter.set_title(f'Drohne 0 - 3D Scatterplot with Zero Plane')
    fig.colorbar(ax_scatter.collections[0], ax=ax_scatter, label='Depthvalue')

    plt.tight_layout()
    plt.show()

    
