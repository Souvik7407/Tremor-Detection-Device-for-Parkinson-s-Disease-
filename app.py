from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

latest_data = {"magnitude": 0, "frequency": 0}

@app.route("/data", methods=["POST"])
def receive_data():
    global latest_data
    latest_data = request.json
    return jsonify({"status": "success", "received": latest_data})

@app.route("/latest", methods=["GET"])
def latest():
    return jsonify(latest_data)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)