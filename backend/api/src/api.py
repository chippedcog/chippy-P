from sanic import Sanic
from sanic.response import json
from sanic_cors import CORS

import env
from middleware.error_handler import APIErrorHandler
from blueprints.converse_action_button.routes import blueprint_converse_action_button
from blueprints.narrator_camera.routes import blueprint_narrator_camera
from blueprints.sketch_ping.routes import blueprint_sketch_ping
from blueprints.sketch_led_state_polling.routes import blueprint_sketch_led_state_polling
from blueprints.sketch_led_state_action_button.routes import blueprint_sketch_led_state_action_button

# INIT
api_app = Sanic('api')


# MIDDLEWARE
# --- cors
CORS(api_app)
# --- error handler
api_app.error_handler = APIErrorHandler()


# ROUTES (blueprint = top level route/prefix to prevent naming colissions)
# --- get started pt.1
api_app.blueprint(blueprint_sketch_ping)
# --- get started pt.2
api_app.blueprint(blueprint_sketch_led_state_action_button)
api_app.blueprint(blueprint_sketch_led_state_polling)
# --- get started pt.3
api_app.blueprint(blueprint_narrator_camera)
api_app.blueprint(blueprint_converse_action_button)
# --- goin wild (TODO)
# ...


# RUN
def start_api():
    api_app.run(
        auto_reload=True, # auto-reload only for dev. done via watchdog pkg in docker-compose file
        dev=False,
        host=env.env_get_api_host(),
        port=env.env_get_api_port(),
        workers=1)
