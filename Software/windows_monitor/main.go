package main

import (
	"fmt"
	"log"
	"os"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"

	"windows_monitor/service"
	"windows_monitor/util"
)

var f mqtt.MessageHandler = func(client mqtt.Client, msg mqtt.Message) {
	fmt.Printf("TOPIC: %s\n", msg.Topic())
	fmt.Printf("MSG: %s\n", msg.Payload())
}

func main() {
	mqtt.DEBUG = log.New(os.Stdout, "", 0)
	mqtt.ERROR = log.New(os.Stdout, "", 0)
	opts := mqtt.NewClientOptions().AddBroker("tcp://192.168.13.52:11883").SetClientID("emqx_test_client")

	opts.SetKeepAlive(60 * time.Second)
	// 设置消息回调处理函数
	opts.SetDefaultPublishHandler(f)
	opts.SetPingTimeout(1 * time.Second)

	c := mqtt.NewClient(opts)
	if token := c.Connect(); token.Wait() && token.Error() != nil {
		panic(token.Error())
	}

	// 订阅主题
	if token := c.Subscribe("iotlink/workgroup/ormissia-win/windows-monitor/stats/cpu/usage", 0, nil); token.Wait() && token.Error() != nil {
		fmt.Println(token.Error())
		os.Exit(1)
	}

	for i := 0; i < 200; i++ {
		cpu := util.FloatToIntString(service.GetCpuPercent())
		fmt.Printf("cpu usage: %s\n", cpu)
		// 发布消息
		token := c.Publish("iotlink/workgroup/ormissia-win/windows-monitor/stats/cpu/usage", 1, false, cpu)
		token.Wait()
		time.Sleep(time.Second * 3)
	}

	time.Sleep(6 * time.Second)

	// 取消订阅
	if token := c.Unsubscribe("iotlink/workgroup/ormissia-win/windows-monitor/stats/cpu/usage"); token.Wait() && token.Error() != nil {
		fmt.Println(token.Error())
		os.Exit(1)
	}

	// 断开连接
	c.Disconnect(250)
	time.Sleep(1 * time.Second)
}
