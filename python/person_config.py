from gz.math7 import Pose3d

from plugin_config import PluginConfig

class PersonConfig(PluginConfig):

    def __init__(self):
        super().__init__()
        
        self.set_name("Person")
        self.set_filename("Person")
        self.config = {
            "model_name": "person",
            "model_pose": "idle",
            "pose": Pose3d(0, 0, 0, 0, 0, 0),
            "scale": 0.02
        }

        self._write_content()

    def set_model_pose(self, model_pose):
        self.config["model_pose"] = model_pose
        self._write_content()

    def set_pose(self, pose):
        self.config["pose"] = pose
        self._write_content()

    def set_scale(self, scale):
        self.config["scale"] = scale
        self._write_content()

    def set_temperature(self, temperature):
        self.config["temperature"] = temperature;
        self._write_content()

