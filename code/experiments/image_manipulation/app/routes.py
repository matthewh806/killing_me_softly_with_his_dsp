from app import app
from flask import render_template, url_for
from soxmosh import SoxMosh
import os

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))

@app.route('/')
@app.route('/index')
def index():
    return render_template('index.html', input_path=url_for('static', filename='perfect_blue_face.bmp'))

@app.route("/mosh_image/", methods=['POST'])
def mosh_image():
    sox_mosh = SoxMosh(os.path.join(CURRENT_DIRECTORY, 'static/perfect_blue_face.bmp'))
    sox_mosh.databend_image(os.path.join(CURRENT_DIRECTORY, 'static/perfect_blue_face_out.bmp'),
                        [{"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [60], "decays": [0.5]}}])
                        
    return render_template('index.html', input_path=url_for('static', filename='perfect_blue_face.bmp'), output_path=url_for('static', filename='perfect_blue_face_out.bmp'))