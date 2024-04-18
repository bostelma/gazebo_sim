import numpy as np

from gz.msgs.boolean_pb2 import Boolean
from gz.msgs.pose_pb2 import Pose
from gz.msgs.pose_v_pb2 import Pose_V
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

    _person_id = 0      # ID counter for persons
    def __init__(self, world_name):
        '''Initializes the communication

        Parameters
        ----------
        world_name : str
            The name of the world as specified in the .sdf file
        '''

        self.world_name = world_name

        # Construct the topic name for the service
        self.spawn_topic = "/world/" + self.world_name + "/person/spawn"
        self._waypoint_topic = "world/" + self.world_name + "/person/waypoint"

    def spawn(self, count, model = 'person', positions = None):
        '''Spawns persons in the simulation

        Parameters
        ----------
        count : int
            The number of persons to spawn
        model : str
            The name of the model to spawn
        positions   : numpy array of shape (count, 3)
            The positions at which to spawn the persons

        Returns
        -------
        ids : numpy array of shape (count)
            The IDs of the spawned persons    
        ''' 

        if positions is not None:
            if positions.shape[0] != count:
                raise ValueError("Number of persons to spawn and number of positions don't match up!")

        node = Node()
        request = Pose_V()

        ids = []

        for i in range(count):
            
            pos_msg = Vector3d()
            if positions is not None:
                pos_msg.x = -positions[i, 1]     # Add this transformation to fit the LFR coordinate system
                pos_msg.y = -positions[i, 0]     # Add this transformation to fit the LFR coordinate system
                pos_msg.z = positions[i, 2]
            else:
                pos_msg.x = 0
                pos_msg.y = 0
                pos_msg.z = 0
            quat_msg = Quaternion()
            quat_msg.x = 0.0
            quat_msg.y = 0.0
            quat_msg.z = 0.0
            quat_msg.w = 1.0

            pose_msg = Pose()
            pose_msg.name = model
            pose_msg.id = self._person_id
            pose_msg.position.CopyFrom(pos_msg)
            pose_msg.orientation.CopyFrom(quat_msg)

            request.pose.append(pose_msg)
            ids.append(self._person_id)
            self._person_id += 1

        rep_msg_type_name = Boolean.DESCRIPTOR.full_name
        suc, ret = node.request(self.spawn_topic, request, self.timeout, rep_msg_type_name)
            
        if suc:
            if not ret:
                raise RuntimeError("Request to spawn persons failed!")
        else:
            raise RuntimeError("Request to spawn persons timed out!")
        
        return np.array(ids)

    def waypoints(self, ids,  positions):
        '''Sends the given waypoint to the person
        
        Parameters
        ----------
        ids: numpy array of size N
            The ids of the persons that should receive the waypoints
        positions : numpy array of shape (N,3)
            The actual waypoint in x, y, z
        orientation : numpy array of size 4
            A quaternion specifying the person orientation
        '''

        if len(ids) != positions.shape[0]:
            raise ValueError("Number of IDs and positions don't match up!")
        
        node = Node()
        request = Pose_V()

        for id, pos in zip(ids, positions):
            
            pos_msg = Vector3d()
            pos_msg.x = -pos[1]     # Add this transformation to fit the LFR coordinate system
            pos_msg.y = -pos[0]     # Add this transformation to fit the LFR coordinate system
            pos_msg.z = 0.0

            if len(positions) > 1:  # Ensure there is more than one waypoint to calculate direction
                direction = positions[(id - 1) % len(positions)] - pos
                direction /= np.linalg.norm(direction)  # Normalize direction vector
            else:
                direction = np.array([0, 0, 0])  # If only one waypoint, keep direction zero
                
            # Calculate orientation quaternion based on direction
            angle = np.arccos(direction.dot(np.array([1, 0, 0])) / np.linalg.norm(direction))
            
            orientation = np.array([0.0, 0.0, np.sin(angle/2), np.cos(angle/2)])    # Orientation towards next waypoint

            quat_msg = Quaternion()
            quat_msg.x = orientation[0]
            quat_msg.y = orientation[1]
            quat_msg.z = orientation[2]
            quat_msg.w = orientation[3]

            pose_msg = Pose()
            pose_msg.id = id
            pose_msg.position.CopyFrom(pos_msg)
            pose_msg.orientation.CopyFrom(quat_msg)
            
            request.pose.append(pose_msg)
        

        rep_msg_type_name = Boolean.DESCRIPTOR.full_name
        suc, ret = node.request(self._waypoint_topic, request, self.timeout, rep_msg_type_name)

        if suc:
            if not ret:
                raise RuntimeError("Request to send waypoints failed!")
        else:
            raise RuntimeError("Request to send waypoints timed out!")