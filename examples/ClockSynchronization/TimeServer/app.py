import os
import sys
import grpc
import json
import MQTT
import base64
import logging
import argparse

from enum import Enum
from datetime import datetime
from chirpstack_api import api
from datetime import datetime as dt

class States(Enum):
    STATE_START     	            = 0
    STATE_WAIT                      = 1
    PROCESS_REQUEST                 = 2

SERVER = os.getenv("API_CHIRPSTACK_SERVER")
API_TOKEN = os.getenv("API_CHIRPSTACK_API_TOKEN")

Parser = argparse.ArgumentParser()
Parser.add_argument("-lp", "--lora-port", type = int, help = "LoRaWAN port", default = 202)
Parser.add_argument("-mp", "--mqtt-port", type = int, help = "MQTT port", default = 8583)
Parser.add_argument("-gp", "--grpc-port", type = int, help = "GRPC port", default = 8580)
Parser.add_argument("-n", "--number", type = int, help = "Number of clock sync force transmission (<= 7)", default = 7)
Parser.add_argument("-g", "--group", type = int, help = "Multicast group used", default = 0)
Parser.add_argument("-d", "--deveui", type = str, help = "Device EUI", default = "ac1f09fffe071978")
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
    CurrentState = States.STATE_START.value

    LogDirectory = args.log + os.path.os.sep + "Update-Logs"

    if(args.number > 7):
        Parser.error("Maximum number of forced transmissions is 7!")

    if(not(os.path.exists(LogDirectory))):
        os.makedirs(LogDirectory)
    logging.basicConfig(filename = LogDirectory + os.path.sep + "{}.log".format(dt.now().strftime("%Y%d%m")), level = logging.INFO, format = "%(asctime)-15s :: %(name)-20s :: %(message)-15s")
    Logger = logging.getLogger("ClockSync-app")

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
        if(CurrentState == States.STATE_START.value):
            Logger.info("Waiting for clock sync request")

            CurrentState = States.STATE_WAIT.value

        elif(CurrentState == States.STATE_WAIT.value):
            Response = json.loads(MQTT_Client.WaitForMessage())
            if(Response["deviceInfo"]["devEui"] == args.deveui):  
                Data = base64.b64decode(Response["data"])
                Logger.info("   Data from device: {}".format(Data))

                if(Data[0] == 0x01) & (Response["fPort"] == 202):
                    Logger.info("Receive clock sync request")
                    CurrentState = States.PROCESS_REQUEST.value
                else:
                    Logger.info("Receive generic data")
                    CurrentState = States.STATE_START.value

        elif(CurrentState == States.PROCESS_REQUEST.value):
            Logger.info("Process clock sync request")

            AnsRequired = False
            if(Data[5] & (0x01 << 0x04)):
                AnsRequired = True
                Logger.info("   Answer required!")

            TokenReq = Data[5] & 0x0F
            Logger.info("   Token request: {}".format(TokenReq))

            # Start of GPS epoch
            Start = datetime(1980, 1, 6, 0, 0, 0)
            Delta = (datetime.now() - Start)
            Seconds = round(Delta.total_seconds()) & ((0x01 << 32) - 1)
            Logger.info("   Seconds since GPS epoch: {}".format(Seconds))

            # Transmit the time correction
            if(AnsRequired):
                Data = list()
                TokenAns = TokenReq
                Param = TokenAns

                #   Command     AppTimeAns
                #   Byte 0-3:   TimeCorrection
                #   Byte 4:     Param
                #       Bits 0-3:   TokenAns
                #       Bits 4-7:   RFU
                Data.append(1)
                Data.append((Seconds >> 0) & 0xFF)
                Data.append((Seconds >> 8) & 0xFF)
                Data.append((Seconds >> 16) & 0xFF)
                Data.append((Seconds >> 24) & 0xFF)
                Data.append(Param)

                if(args.multi):
                    MulticastSend(Data, args.lora_port)
                else:
                    UnicastSend(Data, args.deveui, args.lora_port)

            CurrentState = States.STATE_WAIT.value

        else:
            Logger.error("Unknown state {}".format(CurrentState))
            exit()