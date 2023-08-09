from gz.math7 import Pose3d

from plugin_config import PluginConfig

class PhotoShootConfig(PluginConfig):

    def __init__(self):
        super().__init__()
        
        self.set_name("PhotoShoot")
        self.set_filename("PhotoShoot")
        self.config = {
            "directory": None,
            "direct_thermal_factor":  100,
            "indrect_thermal_factor": 25,
            "lower_thermal_threshold": 200,
            "upper_thermal_threshold":400
        }

        self._write_content()

    def set_directory(self, directory):

        self.config["directory"] = directory

        self._write_content()

    def set_direct_thermal_factor(self, direct_thermal_factor):
        self.config["direct_thermal_factor"] = direct_thermal_factor
        self._write_content()

    def set_indirect_thermal_factor(self, indirect_thermal_factor):
        self.config["indirect_thermal_factor"] = indirect_thermal_factor
        self._write_content()

    def set_lower_thermal_threshold(self, lower_thermal_threshold):
        self.config["lower_thermal_threshold"] = lower_thermal_threshold
        self._write_content()

    def set_upper_thermal_threshold(self, upper_thermal_threshold):
        self.config["upper_thermal_threshold"] = upper_thermal_threshold
        self._write_content()

    def set_prefix(self, prefix):
        self.config["prefix"] = prefix
        self._write_content()

    def add_pose(self, pose):

        if "poses" not in self.config:
            self.config["poses"] = {
                "pose": [pose]
            }
        else:
            self.config["poses"]["pose"].append(pose)
            
        self._write_content()
        
    def add_poses(self, poses):

        if "poses" not in self.config:
            self.config["poses"] = {
                "pose": poses
            }
        else:
            self.config["poses"]["pose"].extend(poses)

        self._write_content()

