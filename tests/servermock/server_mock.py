from flask import Flask
from flask import request
import json

app = Flask(__name__)


class State:
    __internal_data__ = {
        "AdColony": ["4PFYVQ9L8R.skadnetwork", "YCLNXRL5PM.skadnetwork"],
        "Google-Mobile-Ads-SDK": ["cstr6suwn9.skadnetwork"],
        "ChartboostSDK": ["blskdfjl2e3.skadnetwork"],
        "Applovin": ["ludvb6z3bs.skadnetwork"],
        "Unknown_network": []
    }

    def set(self, d):
        self.__internal_data__ = d

    def get(self):
        return self.__internal_data__


state = State()


# https://network-setup.fyber.com/networks
# {
# "networks": [AdColony, Google-Mobile-Ads-SDK, AppLovinSDK, ... ]
# }

@app.route('/networks', methods=['GET'])
def networks():
    return json.dumps({"networks": list(state.get().keys())})


# https://network-setup.fyber.com/plist&network_list=AdColony,Google-Mobile-Ads-SDK,AppLovinSDK,unknown_network
# {
# "Adcolony": ["4PFYVQ9L8R.skadnetwork", "YCLNXRL5PM.skadnetwork"],
# "Google-Mobile-Ads-SDK": ["cstr6suwn9.skadnetwork"],
# "Applovin": ["ludvb6z3bs.skadnetwork"],
# "Unknown_network": []
# }
@app.route('/plist', methods=['GET'])
def plist():
    network_list = request.args['network_list'].split(",")
    print(f"networks = {network_list}")
    response = {k: state.get().get(k, []) for k in network_list}
    return json.dumps(response)


@app.route('/set_data', methods=['POST'])
def set_data():
    data = request.data
    print(f"request={request}")
    print(f"data={data}")
    state.set(json.loads(data))
    return state.get()


@app.route('/get_data', methods=['GET'])
def get_data():
    return state.get()


@app.route('/shutdown', methods=['GET'])
def shutdown():
    shutdown_server()
    return 'Server shutting down...'


def shutdown_server():
    func = request.environ.get('werkzeug.server.shutdown')
    if func is None:
        raise RuntimeError('Not running with the Werkzeug Server')
    func()
