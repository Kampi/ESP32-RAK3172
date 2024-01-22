import logging
import paho.mqtt.client as mqtt

class ChirpStackMQTT:
    def __init__(self, Server, Port):
        self.__Logger = logging.getLogger("ChirpStack-MQTT")

        self.__Message = None
        self.__Client = mqtt.Client()
        self.__Client.on_connect = self.__on_MQTT_Connect
        self.__Client.on_message = self.__on_MQTT_Message
        self.__Client.connect(Server, Port, 60)
        self.__Client.loop_start()

        self.__Status = -1

    def __del__(self):
        self.__Client.loop_stop()

    def __on_MQTT_Connect(self, Client, UserData, Flags, ReturnCode):
        self.__Logger.debug("Connected with result code {}".format(ReturnCode))

        self.__Status = 0

    def __on_MQTT_Message(self, Client, UserData, Message):
        self.__Message = Message.payload.decode("ascii")
        self.__Logger.debug("Receive message with topic {} and payload {}".format(Message.topic, self.__Message))

    def SubscribeApplication(self, AppID):
        Topic = "application/{}/device/+/event/up".format(AppID)

        self.__Logger.debug("Subscribe to topic {}".format(Topic))
        self.__Client.subscribe(Topic)

    def WaitForMessage(self):
        while(self.__Message == None):
            pass

        Dummy = self.__Message
        self.__Message = None

        return Dummy

    def Status(self):
        return self.__Status