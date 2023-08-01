import copy

import numpy as np

from gz.msgs.pose_pb2 import Pose
from gz.msgs.pose_v_pb2 import Pose_V
from gz.msgs.stringmsg_v_pb2 import StringMsg_V
from gz.msgs.vector3d_pb2 import Vector3d
from gz.msgs.quaternion_pb2 import Quaternion
from gz.msgs.boolean_pb2 import Boolean
from gz.msgs.frame_pb2 import Frame

from gz.transport import Node
from gz.transport import SubscribeOptions

class Swarm:

    timeout = 1000
    frameNode = Node()
    received_frames = None

    def __init__(self, world_name, n_drones):
        self.world_name = world_name
        self.spawn_service = "/world/" + self.world_name + "/swarm/spawn"
        self.n_drones = n_drones
        self.received_frames = [None] * n_drones
        self.rgb_images = [None] * n_drones
        self.thermal_images = [None] * n_drones


    def cb(self, msg: Frame) -> None:

        id = msg.id

        image_width = msg.rgbImage.width
        image_height = msg.rgbImage.height

        img_arr = np.frombuffer(msg.rgbImage.data, dtype=np.uint8)
        img_arr = np.reshape(img_arr, (image_width, image_height, 3))
        img_arr = np.flip(img_arr, 2)

        self.rgb_images[id] = copy.deepcopy(img_arr)

        image_width = msg.thermalImage.width
        image_height = msg.thermalImage.height

        img_arr = np.frombuffer(msg.thermalImage.data, dtype=np.uint8)
        img_arr = np.reshape(img_arr, (image_width, image_height))
        img_arr = np.repeat(img_arr[:, :, np.newaxis], 3, axis=2)

        self.thermal_images[id] = copy.deepcopy(img_arr)

        self.received_frames[id] = True

    def spawn(self):
        node = Node()

        req = Pose_V()
        for id in range(self.n_drones):
            pos_msg = Vector3d()
            pos_msg.x = 0
            pos_msg.y = 0
            pos_msg.z = 0

            quat_msg = Quaternion()
            quat_msg.x = 0.0
            quat_msg.y = 0.0
            quat_msg.z = 0.0
            quat_msg.w = 1.0

            pose_msg = Pose()
            pose_msg.name = "drone"
            pose_msg.id = id
            pose_msg.position.CopyFrom(pos_msg)
            pose_msg.orientation.CopyFrom(quat_msg)
            req.pose.append(pose_msg)

        rep_msg_type_name = Boolean.DESCRIPTOR.full_name

        suc, ret = node.request(self.spawn_service, req, self.timeout, rep_msg_type_name)

        frameTopic = "/world/" + self.world_name + "/swarm/frame"
        msg_type_name = Frame.DESCRIPTOR.full_name
        sub_options = SubscribeOptions()
        if not self.frameNode.subscribe(frameTopic, self.cb, msg_type_name, sub_options):
            print("Error subscribing to topic [{}]".format(frameTopic))

    def waypoint(self, id, x, y, z):
       
        node = Node()
        
        req = Pose_V()
        pos_msg = Vector3d()
        pos_msg.x = -y
        pos_msg.y = -x
        pos_msg.z = z

        quat_msg = Quaternion()
        quat_msg.x = 0.0
        quat_msg.y = 0.0
        quat_msg.z = 0.0
        quat_msg.w = 1.0

        pose_msg = Pose()
        pose_msg.name = "drone"
        pose_msg.id = id
        pose_msg.position.CopyFrom(pos_msg)
        pose_msg.orientation.CopyFrom(quat_msg)
        req.pose.append(pose_msg)

        rep_msg_type_name = Boolean.DESCRIPTOR.full_name

        suc, ret = node.request("/world/" + self.world_name + "/swarm/waypoint", req, 1000, rep_msg_type_name)

        self.received_frames[id] = False