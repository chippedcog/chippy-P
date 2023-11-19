import base64
from io import BytesIO
from PIL import Image
import os
from sanic import Blueprint, Request, empty, file, json

from models.elevenlabs import eleven_labs_text_to_speech, mp3_to_wav
from models.gpt import gpt_completion_image_caption


# BLUEPRINT: aka route prefixing/reference class we attach to the api
blueprint_sketch_narrator_camera = Blueprint("narrator_camera", url_prefix="sketch/sketch_narrator_camera")


dir_path = os.path.dirname(os.path.abspath(__file__))
SHORT_CIRCUIT_REQUESTS = False

# ROUTES
@blueprint_sketch_narrator_camera.route('/caption', methods=['POST'])
async def route_sketch_narrator_camera_caption(request: Request):
    image_path = dir_path + "/image.jpg"
    # short circuit (reduce api reqs)
    if SHORT_CIRCUIT_REQUESTS == True:
        return json({ "success": True, "caption": "Short Circuiting." })

    # REQ
    try:
        # --- request body for file buffer
        image_buf = BytesIO(request.body)
        image_base64 = base64.b64encode(image_buf.read()).decode('utf-8')
        # --- (optional) file saving for ref & cleanup
        if os.path.exists(image_path):
            os.remove(image_path)
        image = Image.open(image_buf)
        image.save(image_path)
        # --- image caption
        caption = gpt_completion_image_caption(image_base64, prompt="What's in this image? Don't talk about quality, and use warm language.", max_tokens=40)
        # --- respond to device
        return json({ "success": True, "caption": caption })
    except Exception as err:
        print(err)
        return empty(status=500)


@blueprint_sketch_narrator_camera.route('/narrate', methods=['POST'])
async def route_sketch_narrator_camera_narrate(request: Request):
    file_path_narration = dir_path + "/image_narrative.wav"
    # short circuit (reduce api reqs after you have a file locally)
    if SHORT_CIRCUIT_REQUESTS == True:
        return await file(file_path_narration, mime_type="audio/wav")

    try:
        # --- check text from req body
        text: str = request.json.get('text')
        if (text == None or len(text) == 0):
            raise "No text provided for narration"
        # --- file saving for ref & cleanup
        if os.path.exists(file_path_narration):
            os.remove(file_path_narration)
        # --- audio narration (saving in this directory for reference)
        narration_audio_mp3_io = eleven_labs_text_to_speech(text, file_path=file_path_narration)
        # --- convert mp3 to wav because compressed audio is a pain on devices
        narration_audio_wav_io = mp3_to_wav(narration_audio_mp3_io)
        # --- (optional) saving for reference/checking
        with open(file_path_narration, 'wb') as f:
            print(f'Writing BytesIO to: {file_path_narration}')
            narration_audio_wav_io.seek(0) # moves cursor to start
            f.write(narration_audio_wav_io.read())
        # --- respond to device (should do bytesio.read() but getting err 'embedded null character in path')
        print(f'Responding w/ audio/wav file image_narrative.wav')
        return await file(file_path_narration, mime_type="audio/wav")
    except Exception as err:
        print(err)
        return empty(status=500)
    