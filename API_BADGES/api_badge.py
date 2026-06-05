from flask import Flask, jsonify, request
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

@app.route("/check-badge/<uid_badge>", methods=["GET"])
def check_badge(uid_badge):
    try:
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor(dictionary=True)

        query = """
        SELECT
            badges.uid_badge,
            users.id_user,
            users.nom,
            users.prenom,
            users.pin
        FROM badges
        JOIN users ON badges.id_user = users.id_user
        WHERE badges.uid_badge = %s
        """

        cursor.execute(query, (uid_badge,))
        result = cursor.fetchone()

        cursor.close()
        conn.close()

        if result:
            return jsonify({
                "authorized": True,
                "badge": result["uid_badge"],
                "id_user": result["id_user"],
                "nom": result["nom"],
                "prenom": result["prenom"],
                "pin": result["pin"]
            })

        return jsonify({
            "authorized": False,
            "message": "Badge inconnu"
        }), 404

    except Exception as e:
        return jsonify({
            "authorized": False,
            "error": str(e)
        }), 500

@app.route("/log-acces", methods=["POST"])
def log_acces():
    try:
        data = request.get_json()

        id_user = data.get("id_user")
        methode_auth = data.get("methode_auth")
        resultat = data.get("resultat")
        porte = data.get("porte")

        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()

        query = """
        INSERT INTO acces_log
        (
            id_user,
            methode_auth,
            resultat,
            porte,
            date_acces
        )
        VALUES
        (
            %s,
            %s,
            %s,
            %s,
            NOW()
        )
        """

        cursor.execute(
            query,
            (
                id_user,
                methode_auth,
                resultat,
                porte
            )
        )

        conn.commit()

        cursor.close()
        conn.close()

        return jsonify({
            "success": True,
            "message": "Log ajoute"
        })

    except Exception as e:
        return jsonify({
            "success": False,
            "error": str(e)
        }), 500

if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5050,
        debug=False
    )
