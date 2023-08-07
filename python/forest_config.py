import re

from dict2xml import dict2xml

from plugin_config import PluginConfig

class ForestConfig(PluginConfig):

    def __init__(self):
        super().__init__()
        
        self.set_name("Forest")
        self.set_filename("Forest")
        self.config = {
            "model_name": "procedural-forest",
            "generate": True,
            "ground_texture": 0,
            "direct_spawning": True,
            "size": 100,
            "trees": 300
        }

        self._write_content()

    def set_generate(self, generate):
        self.config["generate"] = generate
        self._write_content()

    def set_ground_texture(self, ground_texture):
        self.config["ground_texture"] = ground_texture
        self._write_content()

    def set_direct_spawning(self, direct_spawning):
        self.config["direct_spawning"] = direct_spawning
        self._write_content()

    def set_size(self, size):
        self.config["size"] = size
        self._write_content()

    def set_trees(self, trees):
        self.config["trees"] = trees
        self._write_content()

    def set_seed(self, seed):
        self.config["seed"]  = seed
        self._write_content()

    def add_species(self, name, config):

        tag = f"species_{name}"
        if tag not in self.config:
            self.config[tag] = config
        else:
            raise ValueError(f"The species {name} already exists!")
        
        self._write_content()

    def _write_content(self):

        self.clear_contents()
        content = dict2xml(self.config)
        content = re.sub(
            r'<species_(.*?)>',
            r'<species name="\1">',
            content
        )
        content = re.sub(
            r'</species_(.*?)>',
            r'</species>',
            content
        )

        self.insert_content(content)