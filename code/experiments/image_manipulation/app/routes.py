from app import app
from app.forms import MoshForm, UploadForm, EchoForm
from flask import render_template, url_for, redirect, request, session, current_app
from werkzeug.utils import secure_filename
from soxmosh import SoxMosh
import os
import json
import wtforms_json

from rq.job import Job

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
DEFAULT_FILE = os.path.join(CURRENT_DIRECTORY, "static/perfect_blue_face.bmp")

ALLOWED_EXTENSIONS = {'bmp', 'png'}

DEFAULT_EFFECTS = '[{"echos": {"gain_in": 0.8, "gain_out": 0.5, "delays": [60], "decays": [0.5]}}]'

wtforms_json.init()

@app.route('/')
@app.route('/index')
def index():

    if 'upload_filename' in session:
        input_url = url_for('static', filename='uploads/' + session['upload_filename'])
        
    else:
        input_url = url_for('static', filename='perfect_blue_face.bmp')

    output_url = ""
    if 'output_filename' in session:
        output_url = url_for('static', filename='output/' + session['output_filename'])

    upload_form = UploadForm()
    echo_form = EchoForm(effect_name='echos')
    mosh_form = MoshForm()

    if 'effects_json' in session and not session['effects_json'] == '':
        mosh_form.effects.data = session['effects_json']
    else:
        mosh_form.effects.data = DEFAULT_EFFECTS
        session['effects_json'] = mosh_form.effects.data

    return render_template('index.html', input_path=input_url, output_path=output_url, upload_form=upload_form, echo_form=echo_form, mosh_form=mosh_form)


@app.route("/mosh_image", methods=['GET','POST'])
def mosh_image():
    mosh_form = MoshForm()

    if 'upload_filename' in session:
        input_path = os.path.join(CURRENT_DIRECTORY, 'static/uploads/' + session['upload_filename'])
    else:
        input_path = os.path.join(CURRENT_DIRECTORY, 'static/perfect_blue_face.bmp')
          
    if request.method == 'POST':
        effects_json = json.loads(request.form['effects'])

        if mosh_form.rendergif.data:
            pre, _ = os.path.splitext(session['upload_filename'])
            output_filename = pre + ".gif"
            session['output_filename'] = output_filename
            output_path = os.path.join(CURRENT_DIRECTORY, 'static/output/' + output_filename)
            job = current_app.task_queue.enqueue('app.tasks.generate_gif_task', input_path, output_path, effects_json)

            return redirect(url_for('result', id=job.id))
        else:
            session['output_filename'] = session['upload_filename']
            output_path = os.path.join(CURRENT_DIRECTORY, 'static/output/' + session['output_filename'])
            job = current_app.task_queue.enqueue('app.tasks.generate_image_task', input_path, output_path, effects_json)

            return redirect(url_for('result', id=job.id))

    return redirect(url_for('index'))       


@app.route("/upload", methods=['POST'])
def upload():
    form = UploadForm()

    if form.validate_on_submit():
        f = form.image.data
        filename = secure_filename(f.filename)
        f.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
        session['upload_filename'] = filename
    
    return redirect(url_for('index'))


@app.route("/add_effect", methods=['POST'])
def add_effect():
    effect_dict = request.form.to_dict()
    effect_name = effect_dict.pop('effect_name', None)

    form = None
    if effect_name == 'echos':
        form = EchoForm()

    effect_dict = {}
    ignore_fields = ["effect_name", "submit", "csrf_token"]
    for fieldname, value in form.data.items():
        if not fieldname in ignore_fields:
            if isinstance(value, str):
                # This is a hacky way of dealing with inputs where a list could be passed
                # On the form side we use a TextAreaField and expect the input to be comma separated
                if "," in value:
                    # convert csv to list - try to guess type?
                    effect_str_values = value.split(",")
                    effect_values = [int(float(x)) if int(float(x)) == float(x) else float(x) for x in effect_str_values]
                    value = effect_values
                else:
                    # directly convert string to type 
                    value = [int(float(value)) if int(float(value)) == float(value) else float(value)]

            effect_dict[fieldname] = value

    effect_json = {}
    effect_json[effect_name] = effect_dict

    effects_json_str = session['effects_json'] if 'effects_json' in session else []
    effects_json_array = json.loads(effects_json_str)
    effects_json_array.append(effect_json)
    
    session['effects_json'] = json.dumps(effects_json_array)
    return redirect(url_for('index'))


@app.route('/result/<string:id>')
def result(id):
    job = Job.fetch(id, connection=current_app.redis)
    status = job.get_status()

    if status in ['queued', 'started', 'deferred', 'failed']:
        return render_template('processing.html', refresh=True)
    elif status == 'finished':
        print("finished the job!")
        return redirect(url_for('index'))
    

