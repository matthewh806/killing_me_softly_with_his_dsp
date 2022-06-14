from flask import Flask
import os

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))

app = Flask(__name__)
app.config['SECRET_KEY'] = 'you-will-never-guess'
app.config['UPLOAD_FOLDER'] = os.path.join(CURRENT_DIRECTORY, 'static/uploads')
app.config['MAX_CONTENT_LENGTH'] = 30 * 1000 * 1000 # 30 mb

from app import routes