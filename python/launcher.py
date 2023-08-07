import subprocess
import sdformat13 as sdf

from typing import Any, Tuple, Iterable

class Launcher:

    launch_config = {
        "iterations": None,
        "running": True,
        "server_only": True,
        "verbosity": 1,
        "world": "shapes.sdf"
    }

    def set_launch_config(self, key: str, value: Any) -> None:

        if key in self.launch_config:
            self.launch_config[key] = value
        else:
            raise ValueError(f"Key {key} is not part of the launch config!")

    def launch(self) -> Tuple[bool, subprocess.CompletedProcess]:

        command = "gz sim"

        command += f" {self.launch_config['world']}"

        if self.launch_config["running"]:
            command += " -r"

        if self.launch_config["server_only"]:
            command += " -s"

        if self.launch_config["verbosity"] is not None:
            command += f" -v {self.launch_config['verbosity']}"

        if self.launch_config["iterations"] is not None:
            command += f" --iterations {self.launch_config['iterations']}"

        print(f"Executing command: {command}")

        completed_process = subprocess.run(command, capture_output=True, shell=True)

        success = completed_process.returncode == 0

        return success, completed_process