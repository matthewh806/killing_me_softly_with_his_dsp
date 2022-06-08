from app import app
from app.forms import MoshForm
from flask import render_template, url_for, redirect, request
from soxmosh import SoxMosh
import os
import json

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))

@app.route('/')
@app.route('/index')
def index():
    form = MoshForm()
    return render_template('index.html', input_path=url_for('static', filename='perfect_blue_face.bmp'), form=form)

@app.route("/mosh_image", methods=['GET','POST'])
def mosh_image():
    form = MoshForm()
    
    if request.method == 'POST':
        effects_json = json.loads(request.form['effects'])
        sox_mosh = SoxMosh(os.path.join(CURRENT_DIRECTORY, 'static/perfect_blue_face.bmp'))
        sox_mosh.databend_image(os.path.join(CURRENT_DIRECTORY, 'static/perfect_blue_face_out.bmp'), effects_json)

        return render_template('index.html', input_path=url_for('static', filename='perfect_blue_face.bmp'), output_path=url_for('static', filename='perfect_blue_face_out.bmp'), form=form)

    return render_template('index.html', input_path=url_for('static', filename='perfect_blue_face.bmp'), output_path=url_for('static', filename='perfect_blue_face_out.bmp'), form=form)              
