import base64
from io import BytesIO
from PIL import Image
import os
from sanic import Blueprint, Request, empty, file, json

from models.elevenlabs import eleven_labs_text_to_speech
from models.gpt import gpt_completion_image_caption

dir_path = os.path.dirname(os.path.abspath(__file__))

# BLUEPRINT: aka route prefixing/reference class we attach to the api
blueprint_sketch_narrator_camera = Blueprint("narrator_camera", url_prefix="sketch/sketch_narrator_camera")


# ROUTES
@blueprint_sketch_narrator_camera.route('/caption', methods=['POST'])
async def route_sketch_narrator_camera_caption(request: Request):
    try:
        # --- request body for file buffer
        image_buf = BytesIO(request.body)
        image_base64 = base64.b64encode(image_buf.read()).decode('utf-8')
        # --- file saving for ref & cleanup
        image_path = dir_path + "/image.jpg"
        if os.path.exists(image_path):
            os.remove(image_path)
        image = Image.open(image_buf)
        image.save(image_path)
        # --- image caption
        caption = gpt_completion_image_caption(image_base64, prompt="What is in this picture and/or what is happening? Don't describe lack of quality. Use warm language.", max_tokens=120)
        # --- respond to device
        return json({ "success": True, "caption": caption })
    except Exception as err:
        print(err)
        return empty(status=500)


@blueprint_sketch_narrator_camera.route('/narrate', methods=['POST'])
async def route_sketch_narrator_camera_narrate(request: Request):
    try:
        # --- get text from req
        text: str = request.json.get('text')
        # --- file saving for ref & cleanup
        file_path_narration = dir_path + "/image_narrative.mp3"
        if os.path.exists(file_path_narration):
            os.remove(file_path_narration)
        # --- audio narration (saving in this directory for reference)
        narration_audio_buf_reader = eleven_labs_text_to_speech(text, file_path=file_path_narration)
        # --- respond to device (bufferreader -> bytes w/ .read())
        return await file(narration_audio_buf_reader.read(), mime_type="audio/mpeg")
    except Exception as err:
        print(err)
        return empty(status=500)
    