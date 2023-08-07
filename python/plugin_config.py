import sdformat13 as sdf

from dict2xml import dict2xml

class PluginConfig(sdf.Plugin):

    config = {}

    def __init__(self):
        super().__init__()

    def set_config(self, config):

        self.config.update(config)

        self._write_content()

    def _write_content(self):

        self.clear_contents()
        content = dict2xml(self.config)
        self.insert_content(content)

