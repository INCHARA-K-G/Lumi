from arduino.app_utils import Bridge


VALID_STATES = {
    "IDLE",
    "LISTENING",
    "THINKING",
    "SPEAKING",
    "HAPPY",
    "CONFUSED",
    "ERROR",
    "SLEEP",
}


def set_eye_state(state: str) -> bool:
    normalized_state = state.strip().upper()

    if normalized_state not in VALID_STATES:
        print(f"[Eyes] Invalid state: {normalized_state}")
        return False

    try:
        Bridge.call(
            "set_eye_state",
            normalized_state
        )

        print(f"[Eyes] State -> {normalized_state}")
        return True

    except Exception as error:
        print(f"[Eyes] Bridge error: {error}")
        return False