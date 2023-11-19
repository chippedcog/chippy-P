from sanic import Blueprint, json

# BLUEPRINT: aka route prefixing/reference class we attach to the api
blueprint_sketch_ping = Blueprint("sketch_ping", url_prefix="device/sketch_ping")

# ROUTES
# --- simple ping!
@blueprint_sketch_ping.route('/ping', methods=['GET'])
def app_route_ping(request):
    return json({ 'status': 'success' })
