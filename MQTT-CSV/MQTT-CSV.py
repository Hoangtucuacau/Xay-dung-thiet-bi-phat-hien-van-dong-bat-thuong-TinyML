import time
import ssl
import paho.mqtt.client as paho
import csv
import keyboard

mqtt_host = "127.0.0.1"
mqtt_port = 1883
mqtt_username = ""
mqtt_password = ""

mqtt_client = paho.Client()
mqtt_client.connect(mqtt_host, mqtt_port)

mqtt_topic = "xyz"

csv_file_prefix = "sitdown-test"
csv_file_suffix = ".csv"
csv_file_index = 51
csv_filename = f"{csv_file_prefix}-{csv_file_index}{csv_file_suffix}"


def on_message(client, userdata, message):
    if running:
        mqtt_payload = str(message.payload.decode("utf-8"))
        print(mqtt_payload)
        data = mqtt_payload.strip('{}').split(",")
        cleaned_data = [entry.split(":")[1] for entry in data]

        with open(csv_filename, mode='a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(cleaned_data)


mqtt_client.on_message = on_message
mqtt_client.subscribe(mqtt_topic)


def toggle_running():
    global running, csv_filename, csv_file_index
    running = not running
    if running:
        csv_file_index += 1
        csv_filename = f"{csv_file_prefix}-{csv_file_index}{csv_file_suffix}"
        print("Program is now running. Data will be saved to", csv_filename)
    else:
        print("Program is now paused. Data saved to", csv_filename)


keyboard.add_hotkey('space', toggle_running)

running = True
print("Press space to toggle program state (running/paused).")

try:
    mqtt_client.loop_start()
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    mqtt_client.disconnect()
    print("Program stopped.")
