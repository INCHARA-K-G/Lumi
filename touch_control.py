import time

from arduino.app_utils import Bridge


class TouchController:
    def __init__(self):
        self.mic_enabled = False
        print("[Touch] Bridge controller ready")

    def is_on(self):
        try:
            self.mic_enabled = bool(
                Bridge.call("isMicEnabled")
            )

            return self.mic_enabled

        except Exception as error:
            print(f"[Touch] Bridge error: {error}")
            return False

    def wait_until_on(self):
        print("[Touch] Waiting for touch...")

        while not self.is_on():
            time.sleep(0.05)

        print("[Touch] MIC ON")