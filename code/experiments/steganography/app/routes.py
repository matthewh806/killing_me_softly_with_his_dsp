from app import app
from flask import redirect, render_template, request, session, url_for
from app.forms import TransmitterForm
from werkzeug.utils import secure_filename
import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
from frequency_domain import Transmitter

@app.route('/')
@app.route('/index')
def index():
    transmitter_form = TransmitterForm()

    if 'base_filename' in session:
        base_audio_url = url_for('static', filename='uploads/' + session['base_filename'])
    else:
        base_audio_url = None

    if 'secret_filename' in session:
        secret_audio_url = url_for('static', filename='uploads/' + session['secret_filename'])
    else:
        secret_audio_url = None

    if 'steganographed_filename' in session:
        steganographed_audio_url = url_for('static', filename='output/' + session['steganographed_filename'])
    else:
        steganographed_audio_url = None

    print(steganographed_audio_url)
    print(base_audio_url)
    return render_template('index.html', 
                            base_audio_url = base_audio_url, 
                            secret_audio_url = secret_audio_url, 
                            steganographed_audio_url = steganographed_audio_url, 
                            transmitter_form = transmitter_form)

@app.route('/hide_audio', methods=['POST'])
def hide_audio():
    transmitter_form = TransmitterForm()

    current_file_path = os.path.dirname(os.path.realpath(__file__))
    input_audio_path = os.path.join(current_file_path, "static/uploads/")
    output_audio_path = os.path.join(current_file_path, "static/output/hidden_audio.wav")

    #if transmitter_form.validate():
    base_path = transmitter_form.base_audio.data
    base_filename = secure_filename(base_path.filename)
    base_filepath = os.path.join(input_audio_path, base_filename)
    base_path.save(base_filepath)
    session['base_filename'] = base_filename

    secret_path = transmitter_form.secret_audio.data
    secret_filename = secure_filename(secret_path.filename)
    secret_file_path = os.path.join(input_audio_path, secret_filename)
    secret_path.save(secret_file_path)
    session['secret_filename'] = secret_filename

    transmitter = Transmitter(base_filepath, secret_file_path)
    transmitter.perform()
    transmitter.write(output_audio_path)
    session['steganographed_filename'] = 'hidden_audio.wav'

    return redirect(url_for('index'))