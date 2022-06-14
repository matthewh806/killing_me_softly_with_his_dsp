from app import app
from app.forms import MoshForm, UploadForm
from flask import render_template, url_for, redirect, request, session
from werkzeug.utils import secure_filename
from soxmosh import SoxMosh
import os
import json

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
DEFAULT_FILE = os.path.join(CURRENT_DIRECTORY, "static/perfect_blue_face.bmp")

ALLOWED_EXTENSIONS = {'bmp', 'png'}

@app.route('/')
@app.route('/index')
def index():

    if 'upload_filename' in session:
        input_path = url_for('static', filename='uploads/' + session['upload_filename'])
    else:
        input_path = url_for('static', filename='perfect_blue_face.bmp')

    upload_form = UploadForm()
    mosh_form = MoshForm()
    return render_template('index.html', input_path=input_path, upload_form=upload_form, mosh_form=mosh_form)


@app.route("/mosh_image", methods=['GET','POST'])
def mosh_image():
    upload_form = UploadForm()
    mosh_form = MoshForm()

    if 'upload_filename' in session:
        input_path = os.path.join(CURRENT_DIRECTORY, 'static/uploads/' + session['upload_filename'])
        input_url = url_for('static', filename='uploads/' + session['upload_filename'])
        output_path = os.path.join(CURRENT_DIRECTORY, 'static/output/' + session['upload_filename'])
        output_url = url_for('static', filename='output/' + session['upload_filename'])
    else:
        input_path = os.path.join(CURRENT_DIRECTORY, 'static/perfect_blue_face.bmp')
        input_url = url_for('static', filename='perfect_blue_face.bmp')
        output_path = os.path.join(CURRENT_DIRECTORY, 'static/perfect_blue_face_out.bmp')
        output_url = url_for('static', filename='output/perfect_blue_face_out.bmp')
    
    if request.method == 'POST':
        effects_json = json.loads(request.form['effects'])
        sox_mosh = SoxMosh(input_path)

        if mosh_form.rendergif.data:
            pre, _ = os.path.splitext(output_path)
            output_path = pre + ".gif"

            url_pre, _ = os.path.splitext(output_url)
            output_url = url_pre + ".gif"

            sox_mosh.databend_to_gif(output_path, effects_json)
        else:
            sox_mosh.databend_image(output_path, effects_json)

        return render_template('index.html', input_path=input_url, output_path=output_url, upload_form=upload_form, mosh_form=mosh_form)

    return render_template('index.html', input_path=input_url, output_path=output_url, upload_form=upload_form, mosh_form=mosh_form)              


@app.route("/upload", methods=['POST'])
def upload():
    form = UploadForm()

    if form.validate_on_submit():
        f = form.image.data
        filename = secure_filename(f.filename)
        f.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
        session['upload_filename'] = filename
    
    return redirect(url_for('index'))
