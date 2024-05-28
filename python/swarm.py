import copy

import numpy as np

from gz.msgs.boolean_pb2 import Boolean
from gz.msgs.frame_pb2 import Frame
from gz.msgs.frame_v_pb2 import Frame_V
from gz.msgs.pose_pb2 import Pose
from gz.msgs.pose_v_pb2 import Pose_V
from gz.msgs.quaternion_pb2 import Quaternion
from gz.msgs.vector3d_pb2 import Vector3d
from gz.transport import Node
from gz.transport import SubscribeOptions

class Swarm:
    '''Swarm interface for the gazebo simulation via gazebo messages

    Attributes
    ----------
    timeout : float
        Timeout for requests in ms
    world_name : str
        The name of the gazebo world
    received_frames : dict
        Dict with drone IDs as keys and a boolean whether pictures
        for a waypoint have been received
    rgb_images : dict of drone ID, numpy array of shape (N, M, 3)
        The received rgb images
    thermal_images : dict of drone ID, numpy array of shape (N, M)
        The received thermal images

    Methods
    -------
    spawn(self, count, model, positions)
        Spawns drones in the simulation
    waypoints(self, ids, positions, orientation)
        Sends the given waypoints to the drones
    '''

    timeout = 1000      # Timeout for request in ms

    _drone_id = 0        # ID counter for drones

    def __init__(self, world_name):
        '''Initializes the communication

        Parameters
        ----------
        world_name : str
            The name of the world as specified in the .sdf file
        '''

        self.world_name = world_name
        self.received_frames = {}
        self.rgb_images = {}
        self.thermal_images = {}
        self.depth_images = {}

        self._frame_node = Node()

        # Construct the topic names for the services and topics
        self.spawn_topic = "/world/" + self.world_name + "/swarm/spawn"
        self.waypoint_topic = "/world/" + self.world_name + "/swarm/waypoint"
        self.frame_topic = "/world/" + self.world_name + "/swarm/frame"

        # Already connect to the frame node
        msg_type_name = Frame_V.DESCRIPTOR.full_name
        sub_options = SubscribeOptions()
        if not self._frame_node.subscribe(self.frame_topic, self._frame_call_back, msg_type_name, sub_options):
            print("Error subscribing to topic [{}]".format(self.frame_topic))
        else:
            print("Subscribed successfully")

    def spawn(self, count, model = 'drone', positions = None):
        '''Spawns drones in the simulation

        Parameters
        ----------
        count : int
            The number of drones to spawn
        model : str
            The name of the model to spawn
        positions : numpy array of shape (count, 3)
            The positions at which to spawn the drones

        Returns
        -------
        ids : numpy array of shape (count)
            The IDs of the spawned drones that act as handles
        '''

        if positions is not None:
            if positions.shape[0] != count:
                raise ValueError("Number of drones to spawn and number of positions don't match up!")

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
            pose_msg.id = self._drone_id
            pose_msg.position.CopyFrom(pos_msg)
            pose_msg.orientation.CopyFrom(quat_msg)

            request.pose.append(pose_msg)

            self.received_frames[self._drone_id] = False
            self.rgb_images[self._drone_id] = None
            self.thermal_images[self._drone_id] = None
            self.depth_images[self._drone_id] = None
            ids.append(self._drone_id)
            self._drone_id += 1

        rep_msg_type_name = Boolean.DESCRIPTOR.full_name
        suc, ret = node.request(self.spawn_topic, request, self.timeout, rep_msg_type_name)

        if suc:
            if not ret:
                raise RuntimeError("Request to spawn drones failed!")
        else:
            raise RuntimeError("Request to spawn drones timed out!")
        
        return np.array(ids)
        
    def waypoints(self, ids, positions, orientation=np.array([0.0, 0.7071068, 0.0, 0.7071068])):
        '''Sends the given waypoints to the drones
        
        Parameters
        ----------
        ids : numpy array of size N
            The ids of the drones that should receive the waypoints
        positions : numpy array of shape (N, 3)
            The actual waypoints
        orientation : numpy array of size 4
            A quaternion specifying the camera orientation
        '''

        if len(ids) != positions.shape[0]:
            raise ValueError("Number of IDs and positions don't match up!")
        
        node = Node()
        request = Pose_V()

        for id, pos in zip(ids, positions, ):

            pos_msg = Vector3d()
            pos_msg.x = -pos[1]     # Add this transformation to fit the LFR coordinate system
            pos_msg.y = -pos[0]     # Add this transformation to fit the LFR coordinate system
            pos_msg.z = pos[2]
           
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

            self.received_frames[id] = False
            self.rgb_images[id] = None
            self.thermal_images[id] = None
            self.depth_images[id] = None

        rep_msg_type_name = Boolean.DESCRIPTOR.full_name
        suc, ret = node.request(self.waypoint_topic, request, self.timeout, rep_msg_type_name)

        if suc:
            if not ret:
                raise RuntimeError("Request to send waypoints failed!")
        else:
            raise RuntimeError("Request to send waypoints timed out!")
        
    def _frame_call_back(self, msg: Frame_V) -> None:

        for frame in msg.frame:

            id = frame.id

            image_width = frame.rgbImage.width
            image_height = frame.rgbImage.height

            img_arr = np.frombuffer(frame.rgbImage.data, dtype=np.uint8)
            img_arr = np.reshape(img_arr, (image_width, image_height, 3))
            img_arr = np.flip(img_arr, 2)

            self.rgb_images[id] = copy.deepcopy(img_arr)
            
            image_width = frame.thermalImage.width
            image_height = frame.thermalImage.height

            img_arr = np.frombuffer(frame.thermalImage.data, dtype=np.uint8, count=image_width*image_height)

            img_arr = np.reshape(img_arr, (image_width, image_height))
            img_arr = np.repeat(img_arr[:, :, np.newaxis], 3, axis=2)

            self.thermal_images[id] = copy.deepcopy(img_arr)
            
            image_width = frame.depthImage.width
            image_height = frame.depthImage.height

            img_arr = np.frombuffer(frame.depthImage.data, dtype=np.uint16, count=image_width*image_height)
            
            img_arr = np.reshape(img_arr, (image_width, image_height))
            img_arr = np.repeat(img_arr[:, :, np.newaxis], 3, axis=2)

            self.depth_images[id] = copy.deepcopy(img_arr)

            self.received_frames[id] = True