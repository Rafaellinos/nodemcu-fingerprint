from flask import Flask, jsonify, request
from datetime import datetime, date
import json

app = Flask(__name__)

ids = [
    {
        "id": 1,
        "age": 32,
        "name": "Rafael Veloso",
        "last_login": date(2022, 3, 1),
        "last_logoff": date(2022, 3, 1),
        "logged": False,
    },
]


@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"


@app.route("/checkExists", methods=['GET'])
def confirm_id_exists():
    id_user = int(request.args.get("id_user", -1))
    res = list(filter(lambda u: u['id'] == id_user, ids))
    if res:
        print(f"id{id_user} founded")
        return "found"
    print(f"id {id_user} not found")
    return "error"


@app.route("/createId", methods=['POST'])
def create_id():
    data = {}
    try:
        data = json.loads(request.data)
        print(data)
    except Exception as e:
        print(f"Error while trying to process createid: {e}")
    print(f"id received: {data.get('id')}")
    if not data.get("id"):
        return "error"
    ids.append({
        "id": data.get("id"),
        "age": -1,
        "name": "Unknown",
        "last_login": None,
        "last_logoff": None,
        "logged": False,
    })
    return "ok"


@app.route("/listIds", methods=['GET'])
def list_ids():
    return jsonify(ids)


@app.route("/log", methods=['POST'])
def log_in_off():
    data = {}
    try:
        data = json.loads(request.data)
        print(data)
    except Exception as e:
        print(f"Error while trying to process login/logoff: {e}")
    id_ = data.get("id", -1)
    res = list(filter(lambda u: u['id'] == id_, ids))
    res = res[0]
    if not res:
        return "error"
    res['logged'] = not bool(res.get("logged"))
    return res['logged'] and "on" or "off"


app.run("0.0.0.0", port=5000, debug=False)
