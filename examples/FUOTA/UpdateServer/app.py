import os
import sys
import grpc
import json
import MQTT
import base64
import logging
import argparse
import Fragmentation

from enum import Enum
from chirpstack_api import api
from datetime import datetime as dt

class States(Enum):
    STATE_SYNC_CLOCK                = 0
    STATE_PREPARE_FRAGMENTS         = 1
    STATE_CHECK_VERSION             = 2
    STATE_FRAG_SESSION_SETUP        = 3
    STATE_FRAG_SESSION_SETUP_ANS    = 4
    STATE_FRAG_SESSION_TRANSFER     = 5
    STATE_FRAG_SESSION_DELETE       = 6
    STATE_FRAG_SESSION_DELETE_ANS   = 7
    STATE_FRAG_SESSION_DONE         = 8

SERVER = os.getenv("API_CHIRPSTACK_SERVER")
API_TOKEN = os.getenv("API_CHIRPSTACK_API_TOKEN")

if(not(SERVER and API_TOKEN)):
    sys.stderr.write("API_CHIRPSTACK_SERVER and API_CHIRPSTACK_API_TOKEN must be set.")
    sys.exit(1)

Parser = argparse.ArgumentParser()
Parser.add_argument("-lp", "--lora-port", type = int, help = "LoRaWAN port", default = 201)
Parser.add_argument("-mp", "--mqtt-port", type = int, help = "MQTT port", default = 8583)
Parser.add_argument("-gp", "--grpc-port", type = int, help = "GRPC port", default = 8580)
Parser.add_argument("-g", "--group", type = int, help = "Multicast group used", default = 0)
Parser.add_argument("-d", "--deveui", type = str, help = "Device EUI", default = "ac1f09fffe071978")
Parser.add_argument("-i", "--input", type = str, help = "Input file", default = "files/Input.bin")
Parser.add_argument("-s", "--session", type = int, help = "Fragmentation session ID", default = 0)
Parser.add_argument("-le", "--length", type = int, help = "Fragment size in bytes", default = 20)
Parser.add_argument("-r", "--redundancy", type = int, help = "Fragment redundancy", default = 5)
Parser.add_argument("-mc", "--multi", help = "Set to use multicast instead of a unicast (default)", action = "store_true")
Parser.add_argument("-t", "--terminal", help = "Redirect the output of the logger to the terminal", action = "store_true")
Parser.add_argument("-l", "--log", help = "Output path for the logging informations", type = str, default = ".")
args = Parser.parse_args()

Auth_Token = [("authorization", "Bearer {}".format(API_TOKEN))]

def MulticastSend(Data, Port):
    Logger.debug("Multicast on port {} with data {}".format(Port, Data))

    client = api.MulticastGroupServiceStub(channel)
    req = api.EnqueueMulticastGroupQueueItemRequest()
    req.queue_item.multicast_group_id = MulticastGroupID
    req.queue_item.f_cnt = 1
    req.queue_item.f_port = Port
    req.queue_item.data = bytes(Data)

    return client.Enqueue(req, metadata = Auth_Token)

def UnicastSend(Data, DevEUI, Port):
    Logger.debug("Unicast send to device '{}' on port {} with data {}".format(DevEUI, Port, Data))

    client = api.DeviceServiceStub(channel)
    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.dev_eui = DevEUI
    req.queue_item.f_port = Port
    req.queue_item.data = bytes(Data)

    return client.Enqueue(req, metadata = Auth_Token)

if(__name__ == "__main__"):
    CurrentState = States.STATE_PREPARE_FRAGMENTS.value

    LogDirectory = os.path.join(args.log, "Update-Logs")

    if(not(os.path.exists(LogDirectory))):
        os.makedirs(LogDirectory)
    logging.basicConfig(filename = LogDirectory + os.path.sep + "{}.log".format(dt.now().strftime("%Y%d%m")), level = logging.INFO, format = "%(asctime)-15s :: %(name)-20s :: %(message)-15s")
    Logger = logging.getLogger("UpdateServer-app")

    if(args.terminal):
        logging.getLogger().addHandler(logging.StreamHandler(sys.stdout))

    MQTT_Client = MQTT.ChirpStackMQTT(SERVER, args.mqtt_port)

    channel = grpc.insecure_channel(SERVER + ":{}".format(args.grpc_port))

    client = api.UserServiceStub(channel)
    req = api.ListUsersRequest()
    req.limit = 10
    resp = client.List(req, metadata = Auth_Token)
    UserID = resp.result[1].id

    client = api.TenantServiceStub(channel)
    req = api.ListTenantsRequest()
    req.limit = 10
    resp = client.List(req, metadata = Auth_Token)
    TenantID = resp.result[0].id

    client = api.ApplicationServiceStub(channel)
    req = api.ListApplicationsRequest()
    req.limit = 10
    req.tenant_id = TenantID
    resp = client.List(req, metadata = Auth_Token)
    ApplicationID = resp.result[0].id

    client = api.DeviceServiceStub(channel)
    req = api.ListDevicesRequest()
    req.limit = 10
    req.application_id = ApplicationID
    resp = client.List(req, metadata = Auth_Token)

    client = api.MulticastGroupServiceStub(channel)
    req = api.ListMulticastGroupsRequest()
    req.limit = 10
    req.application_id = ApplicationID
    resp = client.List(req, metadata = Auth_Token)
    MulticastGroupID = resp.result[0].id

    client = api.DeviceServiceStub(channel)
    req = api.FlushDeviceQueueRequest()
    req.dev_eui = args.deveui
    resp = client.FlushQueue(req, metadata = Auth_Token)

    MQTT_Client.SubscribeApplication(ApplicationID)

    while(True):
        # Sync the clock of the server and the end device
        if(CurrentState == States.STATE_SYNC_CLOCK.value):
            Logger.info("Clock synchronization")

            CurrentState = States.STATE_PREPARE_FRAGMENTS.value

        # Prepare the file fragments
        elif(CurrentState == States.STATE_PREPARE_FRAGMENTS.value):
            Logger.info("Prepare fragments")

            FragmentationProv = Fragmentation.Fragmentation(args.input, args.length, args.redundancy)
            FragmentationProv.WriteToFile(os.path.splitext(args.input)[0] + "_coded" + os.path.splitext(args.input)[1])
            FragmentList = FragmentationProv.GetFragments()

            CurrentState = States.STATE_CHECK_VERSION.value

        # Check the package version from the end-device
        elif(CurrentState == States.STATE_CHECK_VERSION.value):
            
            CurrentState = States.STATE_FRAG_SESSION_SETUP.value

        # Define a fragmentation session and send it to the end device
        elif(CurrentState == States.STATE_FRAG_SESSION_SETUP.value):
            Data = list()

            FragSession = ((args.session & 0x03) << 2) | (args.group & 0x03)
            NbFrag = len(FragmentList) & 0xFFFF

            Logger.info("Setup fragmentation session")
            Logger.info("  FragSession: {}".format(FragSession))
            Logger.info("  NbFrag: {}".format(NbFrag))
            Logger.info("  FragSize: {}".format(args.length))

            #   Command     FragSessionSetupReq
            #   Byte 0:     FragSession
            #       Bits 0-3:   McGroupBitMask
            #       Bits 4-5:   FragIndex
            #       Bits 6-7:   RFU
            #   Byte 1-2:   NbFrag
            #   Byte 3:     FragSize
            #   Byte 4:     Control
            #   Byte 5:     Padding
            #   byte 6-9:   Descriptor
            Data.append(2)
            Data.append(FragSession)
            Data.append(NbFrag & 0xFF)
            Data.append((NbFrag >> 8) & 0xFF)
            Data.append(args.length)
            Data.append(0)
            Data.append(0)
            Data.append(0)
            Data.append(0)
            Data.append(0)
            Data.append(0)
            UnicastSend(Data, args.deveui, args.lora_port)

            CurrentState = States.STATE_FRAG_SESSION_SETUP_ANS.value

        # Wait for the setup confirmation of the end device
        elif(CurrentState == States.STATE_FRAG_SESSION_SETUP_ANS.value):
            Logger.info("Wait for fragmentation setup confirmation")

            Response = json.loads(MQTT_Client.WaitForMessage())

            # Check if the right device has answered
            if(Response["deviceInfo"]["devEui"] == args.deveui):  
                Data = base64.b64decode(Response["data"])
                Logger.info("   Data from device: {}".format(Data))

                if(Data[0] == 0x02):
                    if(Data[1] & (0x01 << 0x00)):
                        Logger.error("      Encoding not supported!")
                        exit()
                    elif(Data[1] & (0x01 << 0x01)):
                        Logger.error("      Not enough memory on the device!")
                        exit()
                    elif(Data[1] & (0x01 << 0x02)):
                        Logger.error("      FragSession index not supported!")
                        exit()
                    elif(Data[1] & (0x01 << 0x03)):
                        Logger.error("      Wrong descriptor!")
                        exit()

                    Logger.error("      FragIndex: {}".format(Data[1] >> 6 & 0x03))

                    CurrentState = States.STATE_FRAG_SESSION_TRANSFER.value
                else:
                    Logger.error("Invalid response! Data: {}".format(Data[0]))
            else:
                CurrentState = States.STATE_FRAG_SESSION_SETUP_ANS.value

        # Transfer the fragmented file
        elif(CurrentState == States.STATE_FRAG_SESSION_TRANSFER.value):
            Logger.info("Transfer data")

            for n, Fragment in enumerate(FragmentList, start = 1):
                Logger.info("   Fragment no. {} / {}".format(n, len(FragmentList)))
                Logger.info("   Fragment {}".format(Fragment))

                # We have to add the command in front of the data
                Fragment = [int(Fragment[i:i + 2], 16) for i in range(0, len(Fragment), 2)]
                Index = ((args.session & 0x03) << 14) | (n & 0x3FFF)
                Fragment.insert(0, Index & 0xFF)
                Fragment.insert(0, (Index >> 8) & 0xFF)
                Fragment.insert(0, 8)

                UnicastSend(Fragment, args.deveui, args.lora_port)

            CurrentState = States.STATE_FRAG_SESSION_DELETE.value

        # Delete a fragmentation session
        elif(CurrentState == States.STATE_FRAG_SESSION_DELETE.value):
            Data = list()

            Logger.info("Delete fragmentation session")

            #   Command     FragSessionDeleteReq
            #   Byte 0:     FragSession
            #       Bits 2-7:   RFU
            #       Bits 0-1:   FragIndex
            Data.append(3)
            FragSession = (args.session & 0x03)
            Data.append(FragSession)

            UnicastSend(Data, args.deveui, args.lora_port)

            CurrentState = States.STATE_FRAG_SESSION_DELETE_ANS.value

        # Wait for the delete confirmation of the end device
        elif(CurrentState == States.STATE_FRAG_SESSION_DELETE_ANS.value):
            Logger.info("Wait for fragmentation delete confirmation")

            Response = json.loads(MQTT_Client.WaitForMessage())

            # Check if the right device has answered
            if(Response["deviceInfo"]["devEui"] == args.deveui):  
                Data = base64.b64decode(Response["data"])
                Logger.info("Data from device: {}".format(Data))

                if(Data[0] == 0x03):
                    if(Data[1] & (0x01 << 0x02)):
                        Logger.error("      Session does not exist!")

                    Logger.error("      FragIndex: {}".format(Data[1] >> 6 & 0x03))
                else:
                    Logger.error("Invalid response! Data: {}".format(Data[0]))

                CurrentState = States.STATE_FRAG_SESSION_DONE.value
            else:
                CurrentState = States.STATE_FRAG_SESSION_DELETE_ANS.value

        else:
            Logger.error("Unknown state {}".format(CurrentState))
            exit()