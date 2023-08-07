from gz.math7 import Pose3d

from plugin_config import PluginConfig

class PhotoShootConfig(PluginConfig):

    def __init__(self):
        super().__init__()
        
        self.set_name("PhotoShoot")
        self.set_filename("PhotoShoot")
        self.config = {
            "directory": None
        }

        self._write_content()

    def set_directory(self, directory):

        self.config["directory"] = directory

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

