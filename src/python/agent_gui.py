import tkinter as tk
from tkinter import messagebox
from bleak import BleakClient
import asyncio

# Global variables
window_open = False
connection_status = "Not Connected"
firebeetle_mac = "FBF96330-5137-08DC-A5E5-2B871E85BC5D"

# Connect to FireBeetle
async def connect_to_firebeetle(mac_address, status_label):
    global connection_status
    try:
        async with BleakClient(mac_address) as client:
            if await client.is_connected():
                connection_status = "Connected"
                status_label.config(text=f"Status: {connection_status}", fg="green")
                messagebox.showinfo("Connection", "Successfully connected to FireBeetle!")
            else:
                connection_status = "Connection Failed"
                status_label.config(text=f"Status: {connection_status}", fg="red")
    except Exception as e:
        connection_status = f"Error: {str(e)}"
        status_label.config(text=f"Status: {connection_status}", fg="red")
        messagebox.showerror("Connection Error", f"Failed to connect: {str(e)}")

def connect(status_label):
    asyncio.run(connect_to_firebeetle(firebeetle_mac, status_label))

# Set window state
def set_window_open(state, status_label):
    global window_open
    window_open = state
    if window_open:
        status_label.config(text="Window is Open", fg="blue")
    else:
        status_label.config(text="Window is Closed", fg="blue")
    print(f"Window open: {window_open}")

# Main GUI
def main():
    global firebeetle_mac

    # Create the main window
    root = tk.Tk()
    root.title("FireBeetle BLE Client")
    root.geometry("600x400")

    # Title Label
    title_label = tk.Label(root, text="FireBeetle BLE Client", font=("Helvetica", 16, "bold"))
    title_label.pack(pady=10)

    # MAC Address Entry
    firebeetle_mac_entry = tk.Entry(root, width=50)
    firebeetle_mac_entry.insert(0, firebeetle_mac)
    firebeetle_mac_entry.pack(pady=5)

    # Status Label
    status_label = tk.Label(root, text="Status: Not Connected", fg="red", font=("Helvetica", 12))
    status_label.pack(pady=5)

    # Button Frame
    button_frame = tk.Frame(root)
    button_frame.pack(pady=10)

    # Open Button
    open_button = tk.Button(
        button_frame,
        text="Open Window",
        command=lambda: set_window_open(True, status_label)
    )
    open_button.grid(row=0, column=0, padx=5)

    # Close Button
    close_button = tk.Button(
        button_frame,
        text="Close Window",
        command=lambda: set_window_open(False, status_label)
    )
    close_button.grid(row=0, column=1, padx=5)

    # Connect Button
    connect_button = tk.Button(
        root,
        text="Connect to FireBeetle",
        command=lambda: connect(status_label)
    )
    connect_button.pack(pady=10)

    #Quit Button
    quit_button = tk.Button(
        root,
        text="Quit",
        command=root.quit
    )
    quit_button.pack(pady=10)

    # Run the main loop
    root.mainloop()


def show_message_from_bluetooth():
    

if __name__ == "__main__":
    main()
