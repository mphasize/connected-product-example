/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 *
 * @format
 * @flow
 */

import React, { Component } from "react";
import { Platform, StyleSheet, Text, View } from "react-native";
import { Buffer } from "buffer";
import { BleManager } from "react-native-ble-plx";

const instructions = Platform.select({
  ios: "Press Cmd+R to reload,\n" + "Cmd+D or shake for dev menu",
  android:
    "Double tap R on your keyboard to reload,\n" +
    "Shake or press menu button for dev menu"
});

const PiServiceID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const PiCharacteristicIDButton = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

type Props = {};
export default class App extends Component<Props> {
  constructor(props) {
    super(props);

    this.manager = new BleManager();
    this.device = null;

    this.state = {
      ble: "Unknown",
      connected: false
    };
  }

  componentDidMount() {
    this.manager.onStateChange(state => {
      // returns (const) subscription, which could be used to cancel event subscription
      console.log("BLE state:", state);
      this.setState({ ble: state });

      if (state === "PoweredOn") {
        this.scanAndConnect();
        //subscription.remove();
      }
    }, true);
  }

  render() {
    return (
      <View style={styles.container}>
        <Text style={styles.welcome}>Welcome to React Native!</Text>
        <Text style={styles.instructions}>To get started, edit App.js</Text>
        <Text style={styles.instructions}>{instructions}</Text>
      </View>
    );
  }

  scanAndConnect() {
    console.log("Scanning for MarcusBLE");
    this.setState({ connected: false });
    this.device = null;
    this.manager.startDeviceScan(null, null, (error, device) => {
      if (error) {
        console.log(error);
        // Handle error (scanning will be stopped automatically)
        return;
      }

      // Check if it is a device you are looking for based on advertisement data
      // or other criteria.
      if (device.name === "MarcusBLE") {
        // Stop scanning as it's not necessary if you are scanning for one device.
        this.manager.stopDeviceScan();

        // Proceed with connection.
        device
          .connect()
          .then(device => {
            console.log("Connected to MarcusBLE");
            this.device = device;
            return device.discoverAllServicesAndCharacteristics();
          })
          .then(device => {
            this.setState({ connected: true });
            this.manager.monitorCharacteristicForDevice(
              device.id,
              PiServiceID,
              PiCharacteristicIDButton,
              (error, characteristic) => this.onButton1(error, characteristic)
            );
          })
          .catch(error => {
            // Handle errors
            console.log(error);
          });
      }
    });
  }

  onButton1(error, characteristic) {
    if (error && error.errorCode === 201) {
      return this.scanAndConnect();
    }

    const value = Buffer.from(characteristic.value, "base64").toString("ascii");
    console.log("Button 1 ", value);
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
    backgroundColor: "#F5FCFF"
  },
  welcome: {
    fontSize: 20,
    textAlign: "center",
    margin: 10
  },
  instructions: {
    textAlign: "center",
    color: "#333333",
    marginBottom: 5
  }
});
