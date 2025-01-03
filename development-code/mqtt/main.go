package main

import (
	"fmt"
	mqtt "github.com/eclipse/paho.mqtt.golang"
)

func main() {
	// MQTT Client opties
	opts := mqtt.NewClientOptions()
	opts.AddBroker("tcp://broker.hivemq.com:1883")
	opts.SetClientID("go-mqtt-wildcard-subscriber")
	
	// Callback voor berichten
	opts.SetDefaultPublishHandler(func(client mqtt.Client, msg mqtt.Message) {
		fmt.Printf("Ontvangen bericht: %s van onderwerp: %s\n", msg.Payload(), msg.Topic())
	})

	// Maak een nieuwe client
	client := mqtt.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		panic(token.Error())
	}
	fmt.Println("Verbonden met MQTT-broker!")

	// Abonneren met een wildcard
	topic := "sensor/+/temperature"
	if token := client.Subscribe(topic, 1, nil); token.Wait() && token.Error() != nil {
		fmt.Println(token.Error())
	}
	fmt.Printf("Geabonneerd op onderwerp: %s\n", topic)

	// Applicatie actief houden
	select {}
}

