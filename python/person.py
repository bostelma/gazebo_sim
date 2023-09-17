import numpy as np

from gz.msgs.boolean_pb2 import Boolean
from gz.msgs.pose_pb2 import Pose
from gz.msgs.quaternion_pb2 import Quaternion
from gz.msgs.vector3d_pb2 import Vector3d
from gz.transport import Node

class Person:
    '''Person interface for the gazebo simulation via gazebo messages

    Attributes
    ----------
    timeout : float
        Timeout for requests in ms
    world_name : str
        The name of the gazebo world

    Methods
    -------
    waypoint(self, position, orientation)
        Sends the given waypoint to the person
    '''

    timeout = 1000      # Timeout for requests in ms

    def __init__(self, world_name):
        '''Initializes the communication

        Parameters
        ----------
        world_name : str
            The name of the world as specified in the .sdf file
        '''

        self.world_name = world_name

        # Construct the topic name for the service
        self._waypoint_topic = "world/" + self.world_name + "/person/waypoint"

    def waypoint(self, position, orientation=np.array([0.0, 0.0, 0.0, 1.0])):
        '''Sends the given waypoint to the person
        
        Parameters
        ----------
        position : numpy array of size 3
            The actual waypoint in x, y, z
        orientation : numpy array of size 4
            A quaternion specifying the person orientation
        '''

        node = Node()
        request = Pose()

        pos_msg = Vector3d()
        pos_msg.x = -position[1]     # Add this transformation to fit the LFR coordinate system
        pos_msg.y = -position[0]     # Add this transformation to fit the LFR coordinate system
        pos_msg.z = 0.0

        quat_msg = Quaternion()
        quat_msg.x = orientation[0]
        quat_msg.y = orientation[1]
        quat_msg.z = orientation[2]
        quat_msg.w = orientation[3]

        request.position.CopyFrom(pos_msg)
        request.orientation.CopyFrom(quat_msg)

        rep_msg_type_name = Boolean.DESCRIPTOR.full_name
        suc, ret = node.request(self._waypoint_topic, request, self.timeout, rep_msg_type_name)

        if suc:
            if not ret:
                raise RuntimeError("Request to send waypoint failed!")
        else:
            raise RuntimeError("Request to send waypoint timed out!")