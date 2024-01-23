import re
import sys

import sdformat13 as sdf

from person_config import PersonConfig
from photo_shoot_config import PhotoShootConfig
from forest_config import ForestConfig

class WorldConfig:

    name: str
    root: sdf.Root
    world: sdf.World

    def __init__(self):

        self.name = None
        self.root = sdf.Root()

    def load(self, path: str) -> bool:
        
        try:
            self.root.load(path)
        except sdf.SDFErrorsException as e:
            print(e, file=sys.stderr)
            return False
        
        if self.root.world_count() == 0:
            print("There is no world in the given file!")
            return False
        elif self.root.world_count() > 1:
            print("More than one world found, select the first one!")

        self.world = self.root.world_by_index(0)
        self.name = self.world.name()

        self.plugins = self.world.plugins()

    def save(self, path: str) -> None:

        # Convert the sdf object to a string
        str = self.root.to_string()

        # Bug fix: The <visualize> tag for light sources is not saved
        #          correctly, so we have to add it manually.

        light_source_names = []
        for light_index in range(self.world.light_count()):
            light = self.world.light_by_index(light_index)
            # Save the name of the light source if it is not visualized
            if not light.visualize():
                light_source_names.append(light.name())
                
        # Add the <visualize> tag for all light sources that are not visualized
        out_str = ""
        lines = str.splitlines()
        for i, line in enumerate(lines):
            changed = False
            for light_source_name in light_source_names:
                if f'<light name=\'{light_source_name}\'' in line:
                    # Orig line
                    out_str += line + "\n"
                    # Spaces
                    spaces = re.match(r"\s*", lines[i+1])
                    if spaces is not None:
                        out_str += spaces.group()
                    # Visualize tag
                    out_str += '<visualize>false</visualize>\n'
                    changed = True
            if not changed:
                out_str += line + "\n"
        str = out_str

        # Finally write the result
        with open(path, "w") as file:
            file.write(str)
        

    def add_atmosphere(self, atmosphere):

        self.world.set_atmosphere(atmosphere)

    def get_atmoshphere(self):
        
        return self.world.atmosphere()

    def add_light(self, light):

        self.world.add_light(light)
        
    def get_light(self, name):

        for light_index in range(self.world.light_count()):
            light = self.world.light_by_index(light_index)
            if light.name() == name:
                return light
            
        return None
    
    def add_plugin(self, plugin):

        new_plugins = []
        for curent_plugin in self.world.plugins():
            if curent_plugin.name() != plugin.name():
                new_plugins.append(curent_plugin)

        # Append the new plugin at the end, because the order is important!
        new_plugins.append(plugin)

        self.world.clear_plugins()
        for new_plugin in new_plugins:
            self.world.add_plugin(new_plugin)

    def get_plugin(self, name):

        for plugin in self.world.plugins():
            if plugin.name() == name:
                return plugin
    
    def add_scene(self, scene):

        self.world.add_scene(scene)
    
    def get_scene(self):

        return self.world.scene()