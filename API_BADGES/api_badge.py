from flask import Flask, jsonify
import mysql.connector

app = Flask(__name__)

db_config = {
    "host": "localhost",
    "user": "root",
    "password": "252106",
    "database": "station_blanche"
}

@app.route("/", methods=["GET"])
def home():
    return jsonify({
        "status": "API ONLINE"
    })

@app.route("/users", methods=["GET"])
def get_users():
    conn = mysql.connector.connect(**db_config)

    cursor = conn.cursor(dictionary=True)

    cursor.execute("SELECT * FROM users")

    users = cursor.fetchall()

    cursor.close()
    conn.close()

    return jsonify(users)

if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5050,
        debug=False
    )
