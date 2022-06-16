from app import app
from app.forms import MoshForm, UploadForm
from flask import render_template, url_for, redirect, request, session, current_app
from werkzeug.utils import secure_filename
from soxmosh import SoxMosh
import os
import json

from rq.job import Job

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
DEFAULT_FILE = os.path.join(CURRENT_DIRECTORY, "static/perfect_blue_face.bmp")

ALLOWED_EXTENSIONS = {'bmp', 'png'}

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
    mosh_form = MoshForm()
    return render_template('index.html', input_path=input_url, output_path=output_url, upload_form=upload_form, mosh_form=mosh_form)


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

@app.route('/result/<string:id>')
def result(id):
    job = Job.fetch(id, connection=current_app.redis)
    status = job.get_status()

    if status in ['queued', 'started', 'deferred', 'failed']:
        return render_template('processing.html', refresh=True)
    elif status == 'finished':
        print("finished the job!")
        return redirect(url_for('index'))
    

