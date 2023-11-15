from sanic import response, Sanic
from sanic.response import json
from sanic_cors import CORS

import env

# INIT
api_app = Sanic('api')

# MIDDLEWARE
# --- cors
CORS(api_app)
# --- TODO: db driver + session context

# ROUTES
# --- health
@api_app.route('/health', methods=['GET'])
def app_route_health(request):
    print('/health')
    return json({ 'status': 'success' })

# RUN
def start_api():
    api_app.run(
        dev=False,
        host=env.env_get_api_host(),
        port=env.env_get_api_port(),
        auto_reload=False, # auto-reload only for dev. done via watchdog pkg in docker-compose file
        workers=1) # whether 1 or 2 workers, sits at 3.8GB memory usage
 