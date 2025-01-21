from flask import Flask
from get_scores import get_scores

app = Flask(__name__)

@app.route("/")
def share_scores():
    return get_scores()
