from sanic import Blueprint, json

# BLUEPRINT: aka route prefixing/reference class we attach to the api
blueprint_sketch_ping = Blueprint("blueprint_sketch_ping", url_prefix="blueprint_sketch_ping")

# ROUTES
# --- simple ping!
@blueprint_sketch_ping.route('/ping', methods=['GET'])
def app_route_ping(request):
    return json({ 'status': 'success' })
