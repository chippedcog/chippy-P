from io import BytesIO
import os
from sanic import Blueprint, Request, empty, json

from models.gpt import whisper_speech_to_text


# BLUEPRINT: aka route prefixing/reference class we attach to the api
blueprint_converse_action_button = Blueprint("converse_action_button", url_prefix="device/converse_action_button")


dir_path = os.path.dirname(os.path.abspath(__file__))
SHORT_CIRCUIT_REQUESTS = False

# ROUTES
@blueprint_converse_action_button.route('/say', methods=['POST'])
async def route_converse_action_button_say(request: Request):
    speech_path = dir_path + "/speech.wav"
    # short circuit (reduce api reqs)
    # if SHORT_CIRCUIT_REQUESTS == True:
    #     return json({ "success": True, "caption": "Short Circuiting." })

    # REQ
    try:
        # --- request body for file buffer
        speech_bytes = BytesIO(request.body)
        # --- (optional) file saving for ref & cleanup
        with open(speech_path, 'wb') as f:
            print(f'Writing BytesIO to: {speech_path}')
            speech_bytes.seek(0) # moves cursor to start
            f.write(speech_bytes.read())
        # --- image caption
        speech_bytes.seek(0) # moves cursor to start
        speech_text = whisper_speech_to_text(speech_bytes.read())
        # --- respond to device
        return json({ "success": True, "caption": speech_text })
    except Exception as err:
        print(err)
        return empty(status=500)
