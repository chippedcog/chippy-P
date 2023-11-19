from random import randint
from sanic import Blueprint, json

# BLUEPRINT: aka route prefixing/reference class we attach to the api
blueprint_sketch_led_state_action_button = Blueprint("led_state_action_button", url_prefix="device/sketch_led_state_action_button")

# ROUTES
@blueprint_sketch_led_state_action_button.route('/state', methods=['GET'])
def route_sketch_led_state_action_button_state_get(request):
    # LED On/Off State
    on = randint(0, 1)
    # LED Brightness State (converst to low/med/high enum on board)
    brightness = randint(1, 3)
    # Respond
    return json({ 
        "brightness": brightness,
        "on": on
    })
