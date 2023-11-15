from sanic import Sanic
from sanic.response import json
from sanic_cors import CORS

import env
from middleware.error_handler import APIErrorHandler
from blueprints.sketch_ping.routes import blueprint_sketch_ping

# INIT
api_app = Sanic('api')

# MIDDLEWARE
# --- cors
CORS(api_app)
# --- error handler
api_app.error_handler = APIErrorHandler()


# ROUTES
api_app.blueprint(blueprint_sketch_ping)


# RUN
def start_api():
    api_app.run(
        dev=True,
        host=env.env_get_api_host(),
        port=env.env_get_api_port(),
        auto_reload=False, # auto-reload only for dev. done via watchdog pkg in docker-compose file
        workers=1) # whether 1 or 2 workers, sits at 3.8GB memory usage
