import sdformat13 as sdf
import gz.math7 as gzm

from photo_shoot_config import PhotoShootConfig
from person_config import PersonConfig
from forest_config import ForestConfig
from world_config import WorldConfig
from launcher import Launcher


if __name__ == "__main__":

    world_file_in = "../worlds/example_photo_shoot.sdf"
    world_file_out = "../../photo_shoot.sdf"
    output_directory = "../../data/photo_shoot"

    # Start off by loading an existing world config
    # so that we don't have to create everything!
    world_config = WorldConfig()
    world_config.load(world_file_in)
    
    # Configure the sun as the light source
    # https://github.com/gazebosim/sdformat/blob/sdf13/python/src/sdf/pyLight.cc
    # shows the available functions.
    light = world_config.get_light("sun")
    light.set_direction(gzm.Vector3d(-0.5, 0.1, -0.9))
    
    # Configure the scene, which holds for example the ambient light
    # https://github.com/gazebosim/sdformat/blob/sdf13/python/src/sdf/pyLight.cc
    # shows the available functions
    scene = world_config.get_scene()
    scene.set_ambient(gzm.Color(0.5, 0.5, 0.5, 1.0))

    # Configure the person plugin. See the respective python file 'person_config'
    # for the available options.
    person_config = PersonConfig()
    person_config.set_model_pose("sitting")                 # Must match an .dae mesh file
                                                            # in the respective model!
    person_config.set_pose(gzm.Pose3d(0, 0, 0, 0, 0, 0))
    world_config.add_plugin(person_config)

    # Configure the photo shoot plugin. See the respective python file 
    # 'photo_shoot_config.py' for the available options.
    photo_shoot_config = PhotoShootConfig()
    photo_shoot_config.set_directory(output_directory)
    photo_shoot_config.add_poses([
        gzm.Pose3d( 10, 10, 35, 0, 1.57, 0),    # The angle looks straight down
        gzm.Pose3d(-10, 10, 35, 0, 1.57, 0),
        gzm.Pose3d( 10,-10, 35, 0, 1.57, 0),
        gzm.Pose3d(-10,-10, 35, 0, 1.57, 0),
    ])
    world_config.add_plugin(photo_shoot_config)

    # Configure the procedural forest plugin. See the respective python file
    # for the available options.
    forest_config = ForestConfig()
    forest_config.set_generate(True)
    forest_config.set_ground_texture(0)         # Have a look at the respective model for options
    forest_config.set_direct_spawning(True)     # Does not work when using the gazebo gui, but 3x faster
    forest_config.set_size(100)                 # Width and height of the forest
    forest_config.set_trees(500)                # Number of treees
    forest_config.set_seed(100)                 # Change the seed for multiple runs!
    forest_config.add_species("Birch", {
        "percentage": 1.0,                      # Percentage that this species makes up of all trees
        "homogeneity": 0.95,
        "trunk_texture": 0,                     # Have a look at the respective model for options
        "twigs_texture": 0,                     # Have a look at the respective model for options
        "tree_properties": {
            "clump_max": 0.45,
            "clump_min": 0.4,
            "length_falloff_factor": 0.65,
            "length_falloff_power": 0.75,
            "branch_factor": 2.45,
            "radius_falloff_rate": 0.7,
            "climb_rate": 0.55,
            "taper_rate": 0.8,
            "twist_rate": 8.0,
            "segments": 6,
            "levels": 6,
            "sweep_amount": 0.0,
            "initial_branch_length": 0.7,
            "trunk_length": 1.0,
            "drop_amount": 0.0,
            "grow_amount": 0.4,
            "v_multiplier": 0.2,
            "twig_scale": 0.2
        }
    })
    world_config.add_plugin(forest_config)

    # Save the modified config
    world_config.save(world_file_out)

    # Launch the simulation
    launcher = Launcher()
    launcher.set_launch_config("server_only", True)
    launcher.set_launch_config("running", True)
    launcher.set_launch_config("iterations", 2)
    launcher.set_launch_config("world", world_file_out)
    print(launcher.launch())