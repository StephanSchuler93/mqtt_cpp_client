/*
 * MqttClient.cpp
 * 
 * Implementation of the MqttClient class for connecting, subscribing, publishing,
 * and periodically publishing messages to an MQTT broker.
 * 
 * Author: schuali93
 * Date: 18.01.25
 */

#include "MqttClient.h"
#include <chrono>

MqttClient::MqttClient(const std::string& serverAddress, const std::string& clientId)
    : client(serverAddress, clientId), running(false) {
    client.set_callback(cb);
}

void MqttClient::connect(const std::string& username, const std::string& password) {
    mqtt::connect_options connOpts;
    connOpts.set_user_name(username);
    connOpts.set_password(password);
    client.connect(connOpts)->wait();
}

void MqttClient::subscribe(const std::string& topic, int qos) {
    client.subscribe(topic, qos)->wait();
}

void MqttClient::publish(const std::string& topic, const nlohmann::json& payload, int qos) {
    std::string payloadStr = payload.dump();
    client.publish(topic, payloadStr.c_str(), payloadStr.size(), qos, false)->wait();
}

void MqttClient::disconnect() {
    client.disconnect()->wait();
}

void MqttClient::startPeriodicPublish(const std::string& topic, const nlohmann::json& payload, int qos, int interval) {
    running = true;
    publishThread = std::thread(&MqttClient::periodicPublish, this, topic, payload, qos, interval);
}

void MqttClient::stopPeriodicPublish() {
    running = false;
    if (publishThread.joinable()) {
        publishThread.join();
    }
}

void MqttClient::periodicPublish(const std::string& topic, const nlohmann::json& payload, int qos, int interval) {
    while (running) {
        publish(topic, payload, qos);
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}

void MqttClient::callback::message_arrived(mqtt::const_message_ptr msg) {
    std::cout << "Message arrived: " << msg->get_payload_str() << std::endl;
}