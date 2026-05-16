from flask import Flask, render_template, jsonify
import serial
import threading
import time

app = Flask(__name__)

# ================= SERIAL =================
ser = serial.Serial('/dev/cu.usbserial-0001', 115200, timeout=1)

# ================= STUDENT =================
student = {
    "name": "Vishal",
    "class": "CSE-A"
}

# ================= LIVE DATA =================
data = {
    "status": "OUT",
    "human": "YES",
    "time_in": "--",
    "time_out": "--",
    "duration": 0
}

# ================= HISTORY =================
history = []

# ================= STATE =================
start_time = None
prev_state = None

# ===== AUTO TIMEOUT =====
last_serial_time = time.time()

TIMEOUT_SECONDS = 8


# ================= SERIAL THREAD =================
def read_serial():

    global start_time
    global prev_state
    global last_serial_time

    while True:

        try:

            line = ser.readline().decode(
                errors='ignore'
            ).strip()

            if not line:
                continue

            print("ESP:", line)

            # ===== UPDATE LAST SIGNAL TIME =====
            last_serial_time = time.time()

            # ===== STATE =====
            if "State:IN" in line:

                current_state = "IN"

            elif "State:OUT" in line:

                current_state = "OUT"

            else:
                continue

            # ===== STATE CHANGE =====
            if current_state != prev_state:

                prev_state = current_state

                # ===== ENTRY =====
                if current_state == "IN":

                    data["status"] = "IN"

                    data["time_in"] = \
                        time.strftime("%H:%M:%S")

                    start_time = time.time()

                    print(">>> ENTRY")

                # ===== EXIT =====
                elif current_state == "OUT":

                    data["status"] = "OUT"

                    data["time_out"] = \
                        time.strftime("%H:%M:%S")

                    if start_time:

                        duration = int(
                            time.time() - start_time
                        )

                        data["duration"] = duration

                        # ===== SAVE HISTORY =====
                        history.append({

                            "time_in":
                                data["time_in"],

                            "time_out":
                                data["time_out"],

                            "duration":
                                duration
                        })

                    print(">>> EXIT")

        except Exception as e:

            print("ERROR:", e)


# ================= ROUTES =================
@app.route('/')
def index():

    return render_template("index.html")


@app.route('/data')
def get_data():

    global prev_state

    # ===== AUTO TIMEOUT =====
    if (
        data["status"] == "IN"
        and time.time() - last_serial_time > TIMEOUT_SECONDS
    ):

        data["status"] = "OUT"

        data["time_out"] = time.strftime("%H:%M:%S")

        prev_state = "OUT"

        print(">>> AUTO TIMEOUT EXIT")

    # ===== LIVE DURATION =====
    if data["status"] == "IN" and start_time:

        data["duration"] = int(
            time.time() - start_time
        )

    return jsonify({**student, **data})


@app.route('/history')
def get_history():

    return jsonify(history)


# ================= MAIN =================
if __name__ == "__main__":

    threading.Thread(
        target=read_serial,
        daemon=True
    ).start()

    app.run(
        debug=False,
        use_reloader=False
    )
