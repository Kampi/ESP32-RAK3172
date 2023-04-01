import os
import json
import base64
import requests
import argparse

SERVER = os.getenv("API_CHIRPSTACK_SERVER")
API_TOKEN = os.getenv("API_CHIRPSTACK_API_TOKEN")
USERNAME = os.getenv("API_CHIRPSTACK_USERNAME")
PASSWORD = os.getenv("API_CHIRPSTACK_PASSWORD")

Headers = {
    "Content-Type" : "application/json",
    "Accept" : "application/json"
}

parser = argparse.ArgumentParser()
parser.add_argument("-g", "--group", type = str)
args = parser.parse_args()

if(__name__ == "__main__"):
    Data = {
        "email" : USERNAME,
        "password" : PASSWORD
    }

    # Login into ChirpStack
    Response = requests.post(SERVER + "/api/internal/login", headers = Headers, data = json.dumps(Data))
    if(not(Response.status_code == 200)):
        print("Invalid response! Exit application...")
        exit()

    Headers["Authorization"] = "Bearer " + json.loads(Response.content).get("jwt")

    Response = requests.get(SERVER + "/api/multicast-groups?limit=10", headers = Headers)
    if(not(Response.status_code == 200)):
        print("Can not get ID from ChirpStack!")
        exit()

    GroupID = 0
    for Item in Response.json()["result"]:
        if(Item["name"] == args.group):
            GroupID = Item["id"]

    if(GroupID == 0):
        print("Can not find multicast group!")
        exit()

    #with open("Firmware.hex", "r") as File:
    #    while(Line := File.readline().rstrip()):
    #        print(Line)

    Data = dict()
    Item = {
        "data" : base64.b64encode("A".encode("ascii")).decode("ascii"),
        "multicast_group_id" : GroupID,
        "fCnt": 1,
        "fPort": 1,
    }
    Data["multicastQueueItem"] = Item

    Response = requests.post(SERVER + "/api/multicast-groups/" + GroupID + "/queue", headers = Headers, data = json.dumps(Data))

    Response = requests.get(SERVER + "/api/multicast-groups/" + GroupID + "/queue", headers = Headers)
    print(Response.json())

    print(Response.status_code)